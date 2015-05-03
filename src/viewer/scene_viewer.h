#ifndef _SPICA_SCENE_VIEWER_H_
#define _SPICA_SCENE_VIEWER_H_

#include <qmainwindow.h>
#include <qboxlayout.h>

#include "qgl_render_widget.h"
#include "../renderer/camera.h"
#include "../renderer/scene.h"

namespace spica {

    class SceneViewer : public QMainWindow {
    public:
        explicit SceneViewer(QWidget *parent = 0);
        ~SceneViewer();

        void setScene(const Scene& scene, const Camera& camera);

    protected:
        QGLRenderWidget* qglWidget;
    };

}

#endif  // _SPICA_SCENE_VIEWER_H_
