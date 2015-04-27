#include <GL/glew.h>
#include <GL/glut.h>

#include "qgl_render_widget.h"

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

        const Sphere* sphere = reinterpret_cast<const Sphere*>(scene.getObjectPtr(0));
        glutSolidSphere(sphere->radius(), 64, 64);
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