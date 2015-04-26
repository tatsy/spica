#ifndef _SPICA_QGL_RENDER_WIDGET_H_
#define _SPICA_QGL_RENDER_WIDGET_H_

#include <QtOpenGL/qgl.h>

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

    };

}

#endif  // _SPICA_QGL_RENDER_WIDGET_H_
