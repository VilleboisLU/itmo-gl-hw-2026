#include "scene.h"
#include <GL/gl.h>

#include <algorithm>
#include <memory>
#include <vector>

#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMatrix4x4>
#include <QObject>
#include <QOpenGLShaderProgram>
#include <QPointF>
#include <QSize>
#include <QSlider>
#include <QString>
#include <QVector2D>
#include <QWidget>

namespace {
constexpr int kFloatForSliderScale = 10;
constexpr int kPointCount = 2000;

void AddFloatSlider(QWidget* page, QFormLayout* form, const QString& name,
                    float& field, float min, float max,
                    const std::function<void()>& request_update) {
  auto* slider = new QSlider(Qt::Horizontal, page);
  slider->setRange(qRound(min * kFloatForSliderScale),
                   qRound(max * kFloatForSliderScale));
  slider->setValue(qRound(field * kFloatForSliderScale));

  auto* label = new QLabel(QString::number(field, 'f', 1), page);
  label->setMinimumWidth(35);

  QObject::connect(slider, &QSlider::valueChanged, page,
                   [&field, label, request_update](int value) {
                     field = static_cast<float>(value) / kFloatForSliderScale;
                     label->setText(QString::number(field, 'f', 1));
                     request_update();
                   });

  auto* row = new QHBoxLayout;
  row->addWidget(slider);
  row->addWidget(label);
  form->addRow(name, row);
}

void AddIntSlider(QWidget* page, QFormLayout* form, const QString& name,
                  int& field, int min, int max,
                  const std::function<void()>& request_update) {
  auto* slider = new QSlider(Qt::Horizontal, page);
  slider->setRange(min, max);
  slider->setValue(field);

  auto* label = new QLabel(QString::number(field), page);
  label->setMinimumWidth(35);

  QObject::connect(slider, &QSlider::valueChanged, page,
                   [&field, label, request_update](int value) {
                     field = value;
                     label->setText(QString::number(field));
                     request_update();
                   });

  auto* row = new QHBoxLayout;
  row->addWidget(slider);
  row->addWidget(label);
  form->addRow(name, row);
}

std::unique_ptr<QOpenGLShaderProgram> MakeProgram(const QString& vert_path, 
                                                  const QString& frag_path) {
  auto program = std::make_unique<QOpenGLShaderProgram>();
  if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vert_path)) {
    qDebug() << program->log();
  }
  if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, frag_path)) {
    qDebug() << program->log();
  }
  if (!program->link()) {
    qDebug() << program->log();
  }
  return program;
}

}  // namespace

QMatrix4x4 Camera2D::ViewProjection(float aspect) const {
  const float half_width = half_height * aspect;
  QMatrix4x4 m;
  m.ortho(center.x() - half_width, center.x() + half_width,
          center.y() - half_height, center.y() + half_height, -1.0f, 1.0f);
  return m;
}

QVector2D Camera2D::ScreenToWorld(QPointF pixel, QSize viewport) const {
  const float width = static_cast<float>(std::max(viewport.width(), 1));
  const float height = static_cast<float>(std::max(viewport.height(), 1));
  const float ndc_x = 2.0f * static_cast<float>(pixel.x()) / width - 1.0f;
  const float ndc_y = 1.0f - 2.0f * static_cast<float>(pixel.y()) / height;
  return center + QVector2D(ndc_x * half_height * (width / height), ndc_y * half_height);
}

void Camera2D::Pan(QPointF from, QPointF to, QSize viewport) {
  center -= ScreenToWorld(to, viewport) - ScreenToWorld(from, viewport);
}

void Camera2D::ZoomAt(QPointF pixel, float factor, QSize viewport) {
  const QVector2D before = ScreenToWorld(pixel, viewport);
  half_height = std::clamp(half_height / factor, 1e-3f, 1e3f);
  const QVector2D after = ScreenToWorld(pixel, viewport);
  center += before - after;
}

void FunctionScene::init() { 
  initializeOpenGLFunctions();
  program_ = MakeProgram(":/shaders/function.vert", ":/shaders/function.frag");

  std::vector<GLfloat> v(kPointCount);
  for (int i = 0; i < kPointCount; i++) {
    v[i] = static_cast<GLfloat>(i) / (kPointCount - 1);
  }

  vao_.create();
  vao_.bind();

  vbo_.create();
  vbo_.bind();
  vbo_.allocate(v.data(), static_cast<int>(v.size() * sizeof(GLfloat)));

  program_->enableAttributeArray(0);
  program_->setAttributeBuffer(0, GL_FLOAT, 0, 1, sizeof(GLfloat));

  vao_.release();
  vbo_.release();
}

void FunctionScene::render(QSize viewport) {
  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  const float aspect = static_cast<float>(viewport.width()) / 
                       static_cast<float>(std::max(viewport.height(), 1));
  const float half_width = camera_.half_height * aspect;
  const float x_min = camera_.center.x() - half_width;
  const float x_max = camera_.center.x() + half_width;

  program_->bind();
  program_->setUniformValue("viewProj", camera_.ViewProjection(aspect));
  program_->setUniformValue("xMin", x_min);
  program_->setUniformValue("xMax", x_max);
  program_->setUniformValue("a", a_);
  program_->setUniformValue("b", b_);
  program_->setUniformValue("c", c_);
  program_->setUniformValue("d", d_);
  program_->setUniformValue("color", QVector3D(1.0f, 0.0f, 0.0f));

  vao_.bind();
  glDrawArrays(GL_LINE_STRIP, 0, kPointCount);
  vao_.release();
  program_->release();
}

QWidget* FunctionScene::CreateControls(QWidget* parent,
                                  std::function<void()> request_update) {
  auto* page = new QWidget(parent);
  auto* form = new QFormLayout(page);
  
  AddFloatSlider(page, form, "a", a_, -10.0f, 10.0f, request_update);
  AddFloatSlider(page, form, "b", b_, -10.0f, 10.0f, request_update);
  AddFloatSlider(page, form, "c", c_, -10.0f, 10.0f, request_update);
  AddFloatSlider(page, form, "d", d_, -10.0f, 10.0f, request_update);
  return page;
} 

void NewtonScene::init() { 
  initializeOpenGLFunctions();
  program_ = MakeProgram(":/shaders/newton.vert", ":/shaders/newton.frag");
  vao_.create();
  camera_.half_height = 1.5f;
}

void NewtonScene::render(QSize viewport) {
  program_->bind();
  program_->setUniformValue("resolution", 
                            QVector2D(static_cast<float>(viewport.width()),
                                      static_cast<float>(viewport.height())));
  program_->setUniformValue("center", camera_.center);
  program_->setUniformValue("halfHeight", camera_.half_height);
  program_->setUniformValue("n", roots_);
  program_->setUniformValue("maxIter", iterations_);

  vao_.bind();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  vao_.release();
  program_->release();
}

QWidget* NewtonScene::CreateControls(QWidget* parent,
                                  std::function<void()> request_update) {
  auto* page = new QWidget(parent);
  auto* form = new QFormLayout(page);
  
  AddIntSlider(page, form, "Корни",    roots_,      3, 12,  request_update);
  AddIntSlider(page, form, "Итерации", iterations_, 1, 200, request_update);
  return page;
}

