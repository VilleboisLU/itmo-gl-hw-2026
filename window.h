#pragma once

#include <memory>
#include <vector>

#include <QComboBox>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QToolButton>
#include <QWidget>

#include "openglwidget.h"
#include "scene.h"

class Window : public OpenGLWidget {
  Q_OBJECT

 public:
  Window();
  ~Window() override;

  void onInit() override;
  void onRender() override;
  void onResize(size_t width, size_t height) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

 private:
  void SetupFpsLabel();
  void SetupOverlay();
  QStackedWidget* CreateScenePanel(QWidget* parent);
  QToolButton* CreateToggleButton(QWidget* overlay, QWidget* panel);
  QComboBox* CreateSceneCombo(QWidget* overlay, QStackedWidget* panel);


  std::vector<std::unique_ptr<Scene>> scenes_;
  int current_ = 0;
  Scene& scene() { return *scenes_[current_]; }
  bool dragging_ = false;
  QPointF last_mouse_pos_;
};
