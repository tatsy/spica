#include "qgl_render_widget.h"

#include <cmath>
#include <algorithm>
#include <typeinfo>

namespace spica {

    QGLRenderWidget::QGLRenderWidget(QWidget *parent)
        : QGLWidget(parent)
        , shaderProgram(0)
        , vbo()
        , timer(0)
        , _useArcBall(false)
        , _scrallDelta(0.0)
        , _isRotate(false)
        , _newX(0.0)
        , _newY(0.0)
        , _oldX(0.0)
        , _oldY(0.0)
        , rotationMat()
    {
        timer = new QTimer(this);
        timer->start(10);
        connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    }

    QGLRenderWidget::~QGLRenderWidget()
    {
        delete timer;
        delete shaderProgram;
    }

    void QGLRenderWidget::setScene(const Scene& scene, const Camera& camera_) {
        this->camera = &camera_;

        this->resize(camera_.imageW(), camera_.imageH());

        for (int i = 0; i < scene.numObjects(); i++) {
            vbo.add(scene.get(i), scene.getMaterial(i).color);
        }
    }

    void QGLRenderWidget::initializeGL() {
        qglClearColor(Qt::black);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_CULL_FACE);

        // Initialize GLSL
        shaderProgram = new QOpenGLShaderProgram(this);
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../../src/viewer/blinn_phong.vs");
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../../src/viewer/blinn_phong.fs");
        shaderProgram->link();
    }

    void QGLRenderWidget::resizeGL(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void QGLRenderWidget::paintGL() {
        const Vector3 eye = camera->lensCenter();
        const Vector3 lookTo = eye + camera->direction();
        const Vector3 up = camera->up();
        const double verticalAngle = 360.0 / PI * atan(camera->sensorH() / (2.0 * camera->distSL()));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QMatrix4x4 projMat, viewMat, modelMat;
        projMat.perspective(verticalAngle, (float)width() / (float)height(), 1.0f, 1000.0f);
        
        viewMat.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                       QVector3D(lookTo.x(), lookTo.y(), lookTo.z()),
                       QVector3D(up.x(), up.y(), up.z()));
        
        modelMat.setToIdentity();
        modelMat = modelMat * rotationMat;
        modelMat.scale(1.0 - _scrallDelta * 0.1);

        shaderProgram->bind();
        shaderProgram->setUniformValue("mMat", modelMat);
        shaderProgram->setUniformValue("vMat", viewMat);
        shaderProgram->setUniformValue("pMat", projMat);

        shaderProgram->setAttributeArray("vertices", vbo.vertices(), 3);
        shaderProgram->setAttributeArray("normals", vbo.normals(), 3);
        shaderProgram->setAttributeArray("colors", vbo.colors(), 3);

        float lightPos[4] = {0.0, 9.0, 0.0, 1.0};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        shaderProgram->enableAttributeArray("vertices");
        shaderProgram->enableAttributeArray("normals");
        shaderProgram->enableAttributeArray("colors");
        glDrawElements(GL_TRIANGLES, vbo.numIndices(), GL_UNSIGNED_INT, vbo.indices());
        shaderProgram->disableAttributeArray("vertices");
        shaderProgram->disableAttributeArray("normals");
        shaderProgram->disableAttributeArray("colors");
    }

    void QGLRenderWidget::animate() {
        update();
    }

    void QGLRenderWidget::updateMouse() {
        QVector3D u = getArcBallVector(_newX, _newY);
        QVector3D v = getArcBallVector(_oldX, _oldY);

        double angle = acos(std::min(1.0f, QVector3D::dotProduct(u, v)));

        QVector3D rotAxis = QVector3D::crossProduct(v, u);
        QMatrix4x4 eye2ObjSpaceMat = rotationMat.inverted();

        QVector3D objSpaceRotAxis = eye2ObjSpaceMat * rotAxis;

        QMatrix4x4 temp;
        double angleByDegree = 180.0 * angle / PI;
        temp.rotate(4.0 * angleByDegree, objSpaceRotAxis);
        
        rotationMat = temp * rotationMat;
    }

    QVector3D QGLRenderWidget::getArcBallVector(int x, int y) {
        QVector3D pt = QVector3D(2.0 * x / this->width() - 1.0, 2.0 * y / this->height() - 1.0, 0.0);
        pt.setY(pt.y() * -1.0);

        double xySquared = pt.x() * pt.x() + pt.y() * pt.y();

        if (xySquared <= 1.0) {
            pt.setZ(sqrt(1.0 - xySquared));
        } else {
            pt.normalize();
        }

        return pt;
    }

    void QGLRenderWidget::wheelEvent(QWheelEvent* e) {
        _scrallDelta += e->delta() / 1000.0;
    }

    void  QGLRenderWidget::mousePressEvent(QMouseEvent* e) {
        _isRotate = false;
        if (e->button() == Qt::LeftButton) {
            _oldX = e->x();
            _oldY = e->y();

            _newX = e->x();
            _newY = e->y();

            _isRotate = true;
            _useArcBall = true;
        }
    }

    void QGLRenderWidget::mouseMoveEvent(QMouseEvent* e) {
        if (e->buttons() & Qt::LeftButton) {
            if (_isRotate) {
                _newX = e->x();
                _newY = e->y();
                updateMouse();
            }
            _oldX = e->x();
            _oldY = e->y();
        }
    }

    void QGLRenderWidget::mouseReleaseEvent(QMouseEvent* e) {
        if (e->button() == Qt::LeftButton) {
            _useArcBall = false;
        }
    }

}  // namespace spica