#include <QtWidgets/qapplication.h>
#include <QtCore/qcommandlineparser.h>

#include <iostream>

#include "sceneparser.h"
using namespace spica;

static constexpr int DEFAULT_NUM_THREADS = 4;

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("spica runtime");

    // Prepare for parsing command line arguments.
    QCommandLineParser parser;
    parser.setApplicationDescription("The spica renderer runtime.");
    parser.addHelpOption();
    parser.addOptions({
        {{"i", "input"},
            QApplication::translate("main", "Input XML file defining the rendering scene (Required)"),
            QCoreApplication::translate("main", "input")},
        {"nthreads",
            QApplication::translate("main", "# of threads to use for rendering (default = 4)"),
            QApplication::translate("main", "nthreads")},
        {{"o", "output"},
            QApplication::translate("main", "Base of output filename (default = image_)"),
            QApplication::translate("main", "output")},
        {"gui",
            QApplication::translate("main", "Show GUI if this options is set (default = OFF)")}
    });
    parser.process(app);

    // Store option values to variables
    std::string sceneFile = "";
    if (parser.isSet("input")) {
        sceneFile = parser.value("input").toStdString();
    } else {
        std::cout << parser.helpText().toStdString() << std::endl;
        return -1;
    }
    printf("Scene: %s\n", sceneFile.c_str());

    int nThreads = DEFAULT_NUM_THREADS;
    if (parser.isSet("nthreads")) {
        nThreads = parser.value("nthreads").toInt();
    }
    printf("Threads: %d\n", nThreads);

    std::string outfile = "image_";
    if (parser.isSet("output")) {
        outfile = parser.value("output").toStdString();
    }

    bool enableGui = false;
    if (parser.isSet("gui")) {
        enableGui = true;
    }
    printf("GUI: %s\n", enableGui ? "ON" : "OFF");

    // Set parameters
    RenderParams &params = RenderParams::getInstance();

    // Generate rendering process
    SceneParser sceneParser(sceneFile);
    sceneParser.parse();

    // Show GUI if required
    if (enableGui) {
    
    }

    return 0;
}
