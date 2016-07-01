#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_VIEWER_H_
#define _SPICA_SCENE_VIEWER_H_

#include <memory>

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qstackedwidget.h>

#include "image_viewer.h"
#include "opengl_viewer.h"
#include "render_worker.h"

#include "../../include/spica.h"

namespace spica {

class SceneViewer : public QMainWindow {
    Q_OBJECT

public:
    explicit SceneViewer(QWidget *parent = 0);
    virtual ~SceneViewer();
        
private slots:
    void OnOpenActTriggered();
    void OnImageSaved(const QImage& image);

private:
    QWidget* mainWidget = nullptr;
    QGridLayout *mainLayout = nullptr;

    QStackedWidget* stackedWidget = nullptr;

    class Ui;
    Ui* ui = nullptr;
    OpenGLViewer *glViewer = nullptr;
    ImageViewer* imageViewer = nullptr;

    QMenu* fileMenu = nullptr;
    QAction* openAct = nullptr;
};

}  // namespace spica

#endif  // _SPICA_SCENE_VIEWER_H_
