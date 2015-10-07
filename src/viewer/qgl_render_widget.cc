#include "qgl_render_widget.h"

#include <cmath>
#include <algorithm>
#include <typeinfo>

#include "../camera/orthographic_camera.h"
#include "../camera/perspective_camera.h"
#include "../camera/dof_camera.h"

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

    void QGLRenderWidget::setScene(const Scene& scene, const Camera& camera_) {
        this->camera = camera_;

        this->resize(camera_.imageW(), camera_.imageH());

        for (int i = 0; i < scene.numTriangles(); i++) {
            vbo.add(scene.getTriangle(i), scene.getBsdf(i).reflectance());
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

        if (camera.imageW() == 0 || camera.imageH() == 0) return;

        const Vector3D eye = camera.center();
        const Vector3D lookTo = eye + camera.direction();
        const Vector3D up = camera.up();

        QMatrix4x4 projMat, viewMat, modelMat, normalMat;
        cameraToProj(&projMat);
        
        viewMat.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                       QVector3D(lookTo.x(), lookTo.y(), lookTo.z()),
                       QVector3D(up.x(), up.y(), up.z()));
        
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

    void QGLRenderWidget::cameraToProj(QMatrix4x4* mat) const {
        ICamera* ptr = camera._ptr;    
        if (typeid(*ptr) == typeid(OrthographicCamera)) {
            OrthographicCamera* cam = reinterpret_cast<OrthographicCamera*>(ptr);
            const Rect& rect = cam->rect();
            QRect qrect(rect.x(), rect.y(), rect.width(), rect.height());
            mat->ortho(qrect);
        } else if (typeid(*ptr) == typeid(PerspectiveCamera)) {
            PerspectiveCamera* cam = reinterpret_cast<PerspectiveCamera*>(ptr);
            const double verticalAngle = 180.0 * cam->fov() / PI;
            mat->perspective(verticalAngle, (float)width() / (float)height(), 1.0f, 1000.0f);
        } else if (typeid(*ptr) == typeid(DoFCamera)) {
            DoFCamera* cam = reinterpret_cast<DoFCamera*>(ptr);
            const double verticalAngle = 360.0 / PI * atan(cam->sensorH() / (2.0 * cam->distSL()));
            mat->perspective(verticalAngle, (float)width() / (float)height(), 1.0f, 1000.0f);
        } else {
            std::cerr << "[ERROR] unknown camera projection type detected: " << (typeid(*ptr).name()) << std::endl;
            std::abort();
        }
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