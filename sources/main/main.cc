#include "../../include/spica.h"

#include <cstring>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    spica::Option option;
    std::vector<std::string> filenames;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--nthreads")) {
            option.nThreads = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--outfile")) {
            option.outfile = argv[++i];
        } else if (!strcmp(argv[i], "--integrator")) {
            option.integrator = argv[++i];
        } else {
            filenames.push_back(argv[++i]);
        }
    }

    // Start rendering
    spica::Engine engine;
    engine.init(option);
    for (const auto& f : filenames) {
        engine.start(f);
    }
    engine.cleanup();
}
