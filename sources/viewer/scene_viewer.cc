#include "scene_viewer.h"

#include <QtCore/qdebug.h>

#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qspinbox.h>
#include <QtWidgets/qfiledialog.h>

#include "report_engine.h"

namespace spica {

class SceneViewer::Ui : public QWidget {
public:
    explicit Ui(QWidget* parent = nullptr)
        : QWidget{ parent } {
        layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        setLayout(layout);

        spinBox = new QSpinBox(this);
        spinBox->setValue(1);
        layout->addWidget(spinBox);
    }

    virtual ~Ui() {
        delete spinBox;
        delete layout;
    }

private:
    QVBoxLayout* layout = nullptr;
    QSpinBox* spinBox = nullptr;
};

SceneViewer::SceneViewer(QWidget *parent)
    : QMainWindow{ parent } {
    setFont(QFont("Meiryo UI"));

    mainWidget = new QWidget();
    mainLayout = new QGridLayout();
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    // Right area
    ui = new Ui(this);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->addWidget(ui, 0, 1);

    // Left area
    stackedWidget = new QStackedWidget(this);

    glViewer = new OpenGLViewer();
    stackedWidget->addWidget(glViewer);

    imageViewer = new ImageViewer(this);
    stackedWidget->addWidget(imageViewer);

    stackedWidget->setCurrentIndex(1);

    mainLayout->setColumnStretch(0, 4);
    mainLayout->addWidget(stackedWidget, 0, 0);

    // Menu
    fileMenu = menuBar()->addMenu(tr("&File"));
    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcut(tr("Ctrl+O"));
    fileMenu->addAction(openAct);

    // SIGNAL / SLOT settings
    connect(openAct, SIGNAL(triggered()), this, SLOT(OnOpenActTriggered()));
}

SceneViewer::~SceneViewer() {
    delete glViewer;
    delete imageViewer;
    delete ui;

    delete mainLayout;
    delete mainWidget;
}

void SceneViewer::OnOpenActTriggered() {
    QString filename = QFileDialog::getOpenFileName(this, "Open", QString(kDataDirectory.c_str()), "XML(*.xml)");
    if (filename == "") return;

    auto engine = std::make_shared<ReportEngine>();
    connect(engine.get(), SIGNAL(imageSaved(const QImage&)), this, SLOT(OnImageSaved(const QImage&)));

    auto task = std::make_shared<std::function<void()>>([=] {
        engine->start(filename.toStdString());
    });

    RenderController* controller = new RenderController(task);
    controller->operate();
}

void SceneViewer::OnImageSaved(const QImage& image) {
    imageViewer->showImage(image);
}

}  // namespace spica
