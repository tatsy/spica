#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_VIEWER_H_
#define _SPICA_SCENE_VIEWER_H_

#include <memory>

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>

#include "qgl_render_widget.h"
#include "render_param_widget.h"
#include "render_thread.h"

#include "../../include/spica.h"

namespace spica {

class SceneViewer : public QMainWindow {
    Q_OBJECT

public:
    explicit SceneViewer(QWidget *parent = 0);
    ~SceneViewer();

    void setScene(const std::vector<Triangle>& tris,
                  const std::vector<Spectrum>& Kd,
                  const std::shared_ptr<const Camera>& camera);

private:
    void setSignalSlots();
        
private slots:
    void onRenderButtonClicked();
    void onLoadButtonClicked();

protected:
    QWidget* mainContainer;
    QHBoxLayout* mainLayout;
        
    QWidget* rightContainer;
    QWidget* leftContainer;

    QVBoxLayout* rightLayout;
    QVBoxLayout* leftLayout;

    QGLRenderWidget* qglWidget;
    RenderParamWidget* paramWidget;

    std::unique_ptr<Integrator> renderer;
    RenderThread* renderThread;
};

}  // namespace spica

#endif  // _SPICA_SCENE_VIEWER_H_
