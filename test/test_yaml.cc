#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

TEST(YamlTest, LoadTest) {
    YamlParser yaml;
    yaml.load(kDataDirectory + "test_scene.yml");

    YamlElement doc = yaml.documentElement();
    YamlElement ch1 = doc.childByKey("scene");
    YamlElement ch2 = ch1.childByKey("meshes");
    YamlElement ch3 = ch2.childByKey("bunny");
    YamlElement ch4 = ch3.childByKey("path");
    std::cout << "path = " << ch4.asString() << std::endl;
}