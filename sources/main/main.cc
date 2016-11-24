#include "spica.h"

#include <cstring>
#include <string>
#include <vector>

#define ENABLE_FP_EXCEPTION 0
#if ENABLE_FP_EXCEPTION
#include <float.h>
void setupFPExceptions() {
    unsigned int cw, newcw;
    _controlfp_s(&cw, 0, 0);
    newcw = ~(_EM_INVALID | _EM_DENORMAL | _EM_ZERODIVIDE); // | _EM_OVERFLOW | _EM_UNDERFLOW);
    _controlfp_s(&cw, newcw, _MCW_EM);
}
#endif

int main(int argc, char** argv) {

#if ENABLE_FP_EXCEPTION
    setupFPExceptions();
#endif

    spica::Option option;
    std::vector<std::string> filenames;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--nthreads")) {
            option.nThreads = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--nsamples")) { 
            option.nSamples = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--outfile")) {
            option.outfile = argv[++i];
        } else {
            filenames.push_back(argv[i]);
        }
    }

    // Start rendering
    spica::Engine engine;
    engine.init(option);
    for (const auto& f : filenames) {
        std::cout << f << std::endl;
        engine.start(f);
    }
    engine.cleanup();
}
