#include "qgl_render_widget.h"
#include <QtCore/QtMath>

#include <cmath>
#include <algorithm>
#include <typeinfo>

namespace spica {

    QGLRenderWidget::QGLRenderWidget(QWidget *parent)
        : QGLWidget(parent)
		, shaderProgram(0)
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

    void QGLRenderWidget::setScene(const Scene& scene_, const Camera& camera_) {
        scene = &scene_;
        camera = &camera_;

        this->resize(camera->imageW(), camera->imageH());
    }

    void QGLRenderWidget::initializeGL() {
        qglClearColor(Qt::black);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);
        glEnable(GL_COLOR_MATERIAL);

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
		const Vector3 eye = camera->center();
        const Vector3 lookTo = eye + camera->direction();
        const Vector3 up = camera->up();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		QMatrix4x4 projMat, viewMat, modelMat;
		projMat.perspective(30.0f, (float)width() / (float)height(), 1.0f, 1000.0f);
		
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

		for (int i = 0; i < scene->numObjects(); i++) {
            const Primitive* ptr = scene->getObjectPtr(i);
            
            if (i == scene->lightID()) {
                const Sphere* lightSphere = reinterpret_cast<const Sphere*>(ptr);
                float lightPos[4];
                lightPos[0] = lightSphere->center().x();
                lightPos[1] = lightSphere->center().y();
                lightPos[2] = lightSphere->center().z();
                lightPos[3] = 1.0f;
                glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
            }
            
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
            glColor3f(ptr->color().red(), ptr->color().green(), ptr->color().blue());

            glPushMatrix();
            if (strcmp(typeid(*ptr).name(), "class spica::Sphere") == 0) {
                const Sphere* sphere = reinterpret_cast<const Sphere*>(ptr);
                glTranslated(sphere->center().x(), sphere->center().y(), sphere->center().z());
                glutSolidSphere(sphere->radius(), 256, 256);
            } else if (strcmp(typeid(*ptr).name(), "class spica::Trimesh") == 0) {
                const Trimesh* trimesh = reinterpret_cast<const Trimesh*>(ptr);
                glBegin(GL_TRIANGLES);
                for (int i = 0; i < trimesh->numFaces(); i++) {
                    Triangle tri = trimesh->getTriangle(i);
                    Vector3 normal = trimesh->getNormal(i);
                    glNormal3d(normal.x(), normal.y(), normal.z());
                    glVertex3d(tri.p0().x(), tri.p0().y(), tri.p0().z());
                    glVertex3d(tri.p1().x(), tri.p1().y(), tri.p1().z());
                    glVertex3d(tri.p2().x(), tri.p2().y(), tri.p2().z());
                }
                glEnd();
            }
            glPopMatrix();
        }
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
		temp.rotate(4.0 * qRadiansToDegrees(angle), objSpaceRotAxis);
		
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
		_scrallDelta += e->delta() / 120.0;
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