#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

TEST(YamlTest, LoadTest) {
    YamlParser yaml;
    yaml.load(kDataDirectory + "test_scene.yml");
}

TEST(SceneLoaderTest, LoadTest) {
    /*
    Scene scene;
    Camera camera;
    SceneLoader loader;
    loader.load(kDataDirectory + "test_scene.yml", &scene, &camera);
    */
}