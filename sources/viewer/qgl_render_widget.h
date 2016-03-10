#ifndef _SPICA_QGL_RENDER_WIDGET_H_
#define _SPICA_QGL_RENDER_WIDGET_H_

#include <QtOpenGL/qgl.h>
#include <qopenglshaderprogram.h>
#include <qtimer.h>

#include <qevent.h>

#include "vbo.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"

namespace spica {

    class QGLRenderWidget : public QGLWidget {
        Q_OBJECT

    public:
        explicit QGLRenderWidget(QWidget *parent = 0);
        ~QGLRenderWidget();

        void setScene(const std::vector<spica::Triangle>& tris,
                      const std::vector<spica::Spectrum>& Kd,
                      const std::shared_ptr<const Camera>& camera);

    protected:
        void initializeGL();
        void resizeGL(int width, int height);
        void paintGL();

        void wheelEvent(QWheelEvent* e);
        void mousePressEvent(QMouseEvent* e);
        void mouseMoveEvent(QMouseEvent* e);
        void mouseReleaseEvent(QMouseEvent* e);

        void updateMouse();
        QVector3D getArcBallVector(int x, int y);

    protected slots:
        void animate();

    protected:
        bool _useArcBall;
        double _scrallDelta;
        bool _isRotate;
        double _newX;
        double _newY;
        double _oldX;
        double _oldY;

        QMatrix4x4 rotationMat;

    private:
        QTimer* timer;
        QOpenGLShaderProgram* shaderProgram;
        
        VBO vbo;
        std::shared_ptr<const Camera> camera;

    };

}

#endif  // _SPICA_QGL_RENDER_WIDGET_H_
