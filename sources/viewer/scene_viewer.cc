#include "scene_viewer.h"

#include <fstream>
#include <map>

#include <QtCore/qdebug.h>

#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qspinbox.h>
#include <QtWidgets/qfiledialog.h>

#include "report_engine.h"

static const std::string configFile = "config.ini";

namespace spica {

class SceneViewer::Config {
public:
    Config(const std::string& confFile)
        : filename_{ confFile } {
        std::ifstream ifs(confFile.c_str(), std::ios::in);
        if (!ifs.is_open()) {
            fprintf(stderr, "Failed to open file: %s\n", confFile.c_str());
            return;
        }

        std::string line;
        while (!ifs.eof()) {
            std::getline(ifs, line);
            if (line.length() == 0) continue;

            int pos = line.find_first_of(' ');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            table_.insert(std::make_pair(key, value));
        }
    }

    ~Config() {
        std::ofstream ofs(filename_.c_str(), std::ios::out);
        for (const auto& p : table_) {
            ofs << p.first << " " << p.second;
        }
        ofs.close();
    }

    std::string get(const std::string& key, const std::string& defaultValue) const {
        const auto it = table_.find(key);
        if (it == table_.cend()) {
            return defaultValue;
        } 
        return it->second;
    }

    void set(const std::string& key, const std::string& value) {
        table_[key] = value;
    }

private:
    std::string filename_;
    std::map<std::string, std::string> table_;
};

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

    // Open config file.
    config_ = std::make_unique<Config>(configFile);
}

SceneViewer::~SceneViewer() {
    delete glViewer;
    delete imageViewer;
    delete ui;

    delete mainLayout;
    delete mainWidget;
}

void SceneViewer::OnOpenActTriggered() {
    std::string lastOpenedDir = config_->get("LAST_OPENED_DIR", kDataDirectory);
    QString filename = QFileDialog::getOpenFileName(this, "Open",
                                                    QString(lastOpenedDir.c_str()),
                                                    "XML(*.xml)");       
    if (filename == "") return;

    QFileInfo fileinfo(filename);
    config_->set("LAST_OPENED_DIR", fileinfo.absoluteDir().absolutePath().toStdString());

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
