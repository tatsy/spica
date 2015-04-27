#include "qgl_render_widget.h"

#include <typeinfo>

namespace spica {

    QGLRenderWidget::QGLRenderWidget(QWidget *parent)
        : QGLWidget(parent)
    {
    }

    QGLRenderWidget::~QGLRenderWidget()
    {
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
    }

    void QGLRenderWidget::resizeGL(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void QGLRenderWidget::paintGL() {
        const Vector3 eye = camera->center();
        const Vector3 lookTo = eye + camera->direction();
        const Vector3 up = camera->up();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(30.0, (double)width() / (double)height(), 1.0, 1000.0);

        gluLookAt(eye.x(), eye.y(), eye.z(), lookTo.x(), lookTo.y(), lookTo.z(), up.x(), up.y(), up.z());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

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
            std::cout << typeid(*ptr).name() << std::endl;
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

        //glFlush();
    }

}  // namespace spica