#pragma once

#include <functional>
#include <memory>

#include <QMatrix4x4>
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QVector2D>
#include <QWidget>

struct Camera2D {
  QVector2D center{0.0f, 0.0f};
  float half_height = 1.0f;

  QMatrix4x4 ViewProjection(float aspect) const;
  QVector2D ScreenToWorld(QPointF pixel, QSize viewport) const;
  void Pan(QPointF from, QPointF to, QSize viewport);
  void ZoomAt(QPointF pixel, float factor, QSize viewport);
};

class Scene : protected QOpenGLFunctions {
 public:
  virtual ~Scene() = default;

  virtual QString name() const = 0;
  virtual void init() = 0;
  virtual void render(QSize viewport) = 0;
  virtual QWidget* CreateControls(QWidget* parent,
                                  std::function<void()> request_update) = 0;
  Camera2D& camera() { return camera_; }

 protected:
  Camera2D camera_;
};

class FunctionScene : public Scene {
 public:
  QString name() const override { return "График функции"; }
  void init() override;
  void render(QSize viewport) override;
  QWidget* CreateControls(QWidget* parent,
                          std::function<void()> request_update) override;

 private:
  std::unique_ptr<QOpenGLShaderProgram> program_;
  QOpenGLVertexArrayObject vao_;
  QOpenGLBuffer vbo_{QOpenGLBuffer::VertexBuffer};
  float a_ = 1.0f;
  float b_ = 0.0f;
  float c_ = 0.0f;
  float d_ = 0.0f;
};

class NewtonScene : public Scene {
 public:
  QString name() const override { return "Фрактал \"Бассейны Ньютона\""; }
  void init() override;
  void render(QSize viewport) override;
  QWidget* CreateControls(QWidget* parent,
                          std::function<void()> request_update) override;

 private:
  std::unique_ptr<QOpenGLShaderProgram> program_;
  QOpenGLVertexArrayObject vao_;
  QOpenGLBuffer vbo_{QOpenGLBuffer::VertexBuffer};
  int roots_ = 3;
  int iterations_ = 5;
};
