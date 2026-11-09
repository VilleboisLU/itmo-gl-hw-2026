#include "window.h"

#include <memory>

#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "scene.h"

namespace {

void SelectPage(QStackedWidget* panel, int index) {
  for (int i = 0; i < panel->count(); ++i) {
    auto policy = (i == index) ? QSizePolicy::Preferred : QSizePolicy::Ignored;
    panel->widget(i)->setSizePolicy(QSizePolicy::Preferred, policy);
  }
  panel->setCurrentIndex(index);
}

}  // namespace

Window::Window() {
  scenes_.push_back(std::make_unique<FunctionScene>());
  scenes_.push_back(std::make_unique<NewtonScene>());
  SetupFpsLabel();
  SetupOverlay();
}

Window::~Window() {
  makeCurrent();
  scenes_.clear();
  doneCurrent();
}

void Window::onInit() {
  qDebug() << scene().camera().ViewProjection(1.0f) << scene().camera().ScreenToWorld({0, 0}, {100, 100});
  for (auto& s : scenes_) {
    s->init();
  }
}

void Window::onRender() {
  scene().render(size() * devicePixelRatioF());
  update();
}

void Window::onResize(const size_t width, const size_t height) {
  glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        last_mouse_pos_ = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    OpenGLWidget::mousePressEvent(event);
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        const QPointF pos = event->position();
        scene().camera().Pan(last_mouse_pos_, pos, size());
        last_mouse_pos_ = pos;
        update();
        event->accept();
        return;
    }
    OpenGLWidget::mouseMoveEvent(event);
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && dragging_) {
        dragging_ = false;
        unsetCursor();
        event->accept();
        return;
    }
    OpenGLWidget::mouseReleaseEvent(event);
}

void Window::wheelEvent(QWheelEvent *event)
{
    const float steps = static_cast<float>(event->angleDelta().y()) / 120.0f;
    if (steps == 0.0f) {
        event->ignore();
        return;
    }
    scene().camera().ZoomAt(event->position(), std::pow(1.1f, steps), size());
    update();
    event->accept();
}

void Window::SetupFpsLabel() {
  auto* fps_label = new QLabel("FPS: 0", this);
  fps_label->setStyleSheet("QLabel { color: white; }");
  fps_label->setAttribute(Qt::WA_TransparentForMouseEvents);
  fps_label->adjustSize();
  fps_label->move(0, 0);
  fps_label->raise();

  connect(this, &Window::fpsUpdated, [fps_label](double fps) {
    fps_label->setText(QString("FPS: %1").arg(fps, 0, 'f', 0));
    fps_label->adjustSize();
  });
}

void Window::SetupOverlay() {
  auto* overlay = new QWidget(this);
  overlay->setAttribute(Qt::WA_StyledBackground);
  overlay->setStyleSheet(
      "QWidget { background: rgba(0, 0, 0, 140); color: white; }"
      "QLabel { background: transparent; }");

  auto* panel = CreateScenePanel(overlay);
  auto* toggle = CreateToggleButton(overlay, panel);
  auto* combo = CreateSceneCombo(overlay, panel);

  auto* vbox = new QVBoxLayout(overlay);
  auto* top_bar = new QHBoxLayout;
  top_bar->addWidget(combo);
  top_bar->addWidget(toggle);
  vbox->addLayout(top_bar);
  vbox->addWidget(panel);

  overlay->adjustSize();
  overlay->move(0, 20);
  overlay->raise();
}

QStackedWidget* Window::CreateScenePanel(QWidget* parent) {
  auto* panel = new QStackedWidget(parent);
  for (auto& s : scenes_) {
    panel->addWidget(s->CreateControls(panel, [this]() { update(); }));
  }
  SelectPage(panel, current_);
  return panel;
}

QToolButton* Window::CreateToggleButton(QWidget* overlay, QWidget* panel) {
  auto* toggle = new QToolButton(overlay);
  toggle->setText("Слайдеры с параметрами");
  toggle->setCheckable(true);
  toggle->setChecked(true);
  connect(toggle, &QToolButton::toggled, this, [panel, overlay](bool on) {
    panel->setVisible(on);
    overlay->adjustSize();
  });
  return toggle;
}

QComboBox* Window::CreateSceneCombo(QWidget* overlay, QStackedWidget* panel) {
  auto* combo = new QComboBox(overlay);
  for (const auto& s : scenes_) {
    combo->addItem(s->name());
  }
  connect(combo, &QComboBox::currentIndexChanged, this,
          [this, panel, overlay](int index) {
            current_ = index;
            SelectPage(panel, index);
            overlay->adjustSize();
            update();
          });
  return combo;
}

