#include "scene_viewer.h"

namespace spica {

    SceneViewer::SceneViewer(QWidget *parent)
        : QMainWindow(parent)
        , mainContainer(new QWidget)
        , mainLayout(new QHBoxLayout)

        , rightContainer(new QWidget)
        , leftContainer(new QWidget)

        , rightLayout(new QVBoxLayout)
        , leftLayout(new QVBoxLayout)

        , qglWidget(new QGLRenderWidget)
        , paramWidget(new RenderParamWidget)
    {
        setFont(QFont("Meiryo UI"));

        mainContainer->setLayout(mainLayout);
        mainLayout->addWidget(leftContainer);
        mainLayout->addWidget(rightContainer);

        rightContainer->setLayout(rightLayout);
        QSizePolicy rightSize(QSizePolicy::Preferred, QSizePolicy::Preferred);
        rightSize.setHorizontalStretch(1);
        rightContainer->setSizePolicy(rightSize);

        leftContainer->setLayout(leftLayout);
        QSizePolicy leftSize(QSizePolicy::Preferred, QSizePolicy::Preferred);
        leftSize.setHorizontalStretch(4);
        leftContainer->setSizePolicy(leftSize);

        leftLayout->addWidget(qglWidget);

        rightLayout->addWidget(paramWidget);

        setCentralWidget(mainContainer);
    }

    SceneViewer::~SceneViewer()
    {
        delete qglWidget;
        delete paramWidget;
        
        delete rightLayout;
        delete leftLayout;

        delete rightContainer;
        delete leftContainer;

        delete mainLayout;
        delete mainContainer;
    }

    void SceneViewer::setScene(const Scene& scene, const Camera& camera) {
        qglWidget->setScene(scene, camera);
    }

}  // namespace spica
