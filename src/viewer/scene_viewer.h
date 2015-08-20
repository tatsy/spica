#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_VIEWER_H_
#define _SPICA_SCENE_VIEWER_H_

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>

#include "qgl_render_widget.h"
#include "render_param_widget.h"

#include "../renderer/camera.h"
#include "../renderer/scene.h"

namespace spica {

    class SceneViewer : public QMainWindow {
    public:
        explicit SceneViewer(QWidget *parent = 0);
        ~SceneViewer();

        void setScene(const Scene& scene, const Camera& camera);

    protected:
        QWidget* mainContainer;
        QHBoxLayout* mainLayout;
        
        QWidget* rightContainer;
        QWidget* leftContainer;

        QVBoxLayout* rightLayout;
        QVBoxLayout* leftLayout;

        QGLRenderWidget* qglWidget;
        RenderParamWidget* paramWidget;
    };

}

#endif  // _SPICA_SCENE_VIEWER_H_
