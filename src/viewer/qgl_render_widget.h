#ifndef _SPICA_QGL_RENDER_WIDGET_H_
#define _SPICA_QGL_RENDER_WIDGET_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include <QtOpenGL/qgl.h>
#include <qopenglshaderprogram.h>
#include <qtimer.h>

#include <qevent.h>

#include "vbo.h"
#include "../renderer/scene.h"
#include "../renderer/camera.h"

namespace spica {

    class QGLRenderWidget : public QGLWidget {
        Q_OBJECT

    public:
        explicit QGLRenderWidget(QWidget *parent = 0);
        ~QGLRenderWidget();

        void setScene(const Scene& scene, const Camera& camera);

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
        const Camera* camera;

    };

}

#endif  // _SPICA_QGL_RENDER_WIDGET_H_
