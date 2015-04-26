#include "qgl_render_widget.h"

#include <gl/glut.h>
#include <gl/glew.h>

namespace spica {

    QGLRenderWidget::QGLRenderWidget(QWidget *parent)
        : QGLWidget(parent)
    {
    }

    QGLRenderWidget::~QGLRenderWidget()
    {
    }

    void QGLRenderWidget::setScene(const Scene& scene, const Camera& camera) {
        printf("%d %d\n", camera.imageW(), camera.imageH());
        this->resize(camera.imageW(), camera.imageH());
    }

    void QGLRenderWidget::initializeGL() {
        qglClearColor(Qt::black);
    }

    void QGLRenderWidget::resizeGL(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void QGLRenderWidget::paintGL() {
    }

}  // namespace spica