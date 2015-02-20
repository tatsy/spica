#include <iostream>

#include "rainy.h"
using namespace rainy;

int main(int argc, char **argv) {
    Vector3 v(1.0, 2.0, 3.0);
    Vector3 u(1.0, 2.0, 3.0);
    v += u;

    std::cout << v.x() << " " << v.y() << " " << v.z() << std::endl;

    return 0;
}
