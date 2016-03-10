#include "qgl_render_widget.h"

#include <cmath>
#include <algorithm>
#include <typeinfo>

#include "../bxdf/bsdf.h"
#include "../image/film.h"
#include "../camera/orthographic.h"
#include "../camera/perspective.h"

#include "shader_location.h"

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
        , rotationMat() {
        timer = new QTimer(this);
        timer->start(10);
        connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    }

    QGLRenderWidget::~QGLRenderWidget() {
        delete timer;
        delete shaderProgram;
    }

    void QGLRenderWidget::setScene(const std::vector<spica::Triangle>& tris,
                                   const std::vector<spica::Spectrum>& Kd,
                                   const std::shared_ptr<const Camera>& camera_) {
        this->camera = camera_;

        const Point2i res = camera_->film()->resolution();
        this->resize(res.x(), res.y());
        for (int i = 0; i < tris.size(); i++) {
            vbo.add(tris[i], Kd[i]);
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
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
            QString(kShaderDirectory.c_str()) + "blinn_phong.vs");
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
            QString(kShaderDirectory.c_str()) + "blinn_phong.fs");
        shaderProgram->link();
        
        if (!shaderProgram->isLinked()) {
            std::cerr << "[ERROR] failed to link shader program !!" << std::endl;
            exit(1);
        }
    }

    void QGLRenderWidget::resizeGL(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void QGLRenderWidget::paintGL() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        qglClearColor(Qt::black);

        if (camera->film()->resolution().x() == 0 ||
            camera->film()->resolution().y() == 0) return;

        QMatrix4x4 projMat, viewMat, modelMat, normalMat;

        spica::Matrix4x4 c2s = camera->cameraToScreen().getMat();
        spica::Matrix4x4 w2c = camera->cameraToWorld().getInvMat();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                projMat(i, j) = c2s(i, j);
                viewMat(i, j) = w2c(i, j);
            }
        }
                
        modelMat.setToIdentity();
        modelMat = modelMat * rotationMat;
        modelMat.scale(1.0 - _scrallDelta * 0.1);

        normalMat = (viewMat * modelMat).inverted().transposed();

        shaderProgram->bind();
        shaderProgram->setUniformValue("mMat", modelMat);
        shaderProgram->setUniformValue("vMat", viewMat);
        shaderProgram->setUniformValue("pMat", projMat);
        shaderProgram->setUniformValue("normalMat", normalMat);

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