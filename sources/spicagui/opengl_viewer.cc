#include "opengl_viewer.h"

#include <cmath>
#include <algorithm>
#include <typeinfo>

#include "../bxdf/bsdf.h"
#include "../image/film.h"
#include "../camera/orthographic.h"
#include "../camera/perspective.h"
#include "../shape/triangle.h"

#include "shader_location.h"

namespace spica {

    OpenGLViewer::OpenGLViewer(QWidget *parent)
        : QOpenGLWidget(parent) {
        timer = std::make_unique<QTimer>(this);
        timer->start(10);
        connect(timer.get(), SIGNAL(timeout()), this, SLOT(animate()));
    }

    OpenGLViewer::~OpenGLViewer() {
    }

    void OpenGLViewer::initializeGL() {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void OpenGLViewer::resizeGL(int w, int h) {
        glViewport(0, 0, width(), height());
    }

    void OpenGLViewer::paintGL() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLViewer::animate() {
        update();
    }

    void OpenGLViewer::wheelEvent(QWheelEvent* e) {
    }

    void  OpenGLViewer::mousePressEvent(QMouseEvent* e) {
    }

    void OpenGLViewer::mouseMoveEvent(QMouseEvent* e) {
    }

    void OpenGLViewer::mouseReleaseEvent(QMouseEvent* e) {
    }

}  // namespace spica