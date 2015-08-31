#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

TEST(YamlTest, LoadTest) {
    YamlParser yaml;
    yaml.load(kDataDirectory + "test_scene.yml");
}