#include <iostream>
#include <string>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "core/argparse.h"
#include "core/renderparams.h"
using namespace spica;

#include "killtimer.h"
#include "renderworker.h"

static constexpr int DEFAULT_NUM_THREADS = 0;

std::string remove_extension(const std::string &path) {
    size_t p = path.find_last_of(".");
    return path.substr(0, p);
}

int main(int argc, char** argv) {
    ArgumentParser parser;
    try {
        parser.addArgument("-i", "--input", "", true);
        parser.addArgument("-t", "--threads", "-1");
        parser.addArgument("-o", "--output", "");
        if (!parser.parse(argc, argv)) {
            std::cout << parser.helpText() << std::endl;
        }
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    // Store option values to variables
    std::string sceneFile = parser.getString("input");
    int nThreads = parser.getInt("threads");
    std::string outfile = "";
    if (parser.getString("output") != "") {
        outfile = parser.getString("output");
    } else {
        fs::path path(sceneFile.c_str());
        outfile = remove_extension(fs::canonical(fs::absolute(path)).string());
    }

    // Set parameters
    RenderParams &params = RenderParams::getInstance();
    params.add("numUserThreads", nThreads);
    params.add("outputFile", outfile);

//    KillTimer timer(0, 4, 30);
//    timer.start();

    RenderWorker worker(sceneFile);
    worker.start();
}
