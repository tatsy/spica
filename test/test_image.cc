#include "gtest/gtest.h"
#include "../include/spica.h"
using namespace spica;

namespace {
    Random rng = Random();
}

TEST(ImageTest, InstanceTest) {
    const int width = 320;
    const int height = 240;

    Image image;
    EXPECT_EQ(0, image.width());
    EXPECT_EQ(0, image.height());
    ASSERT_DEATH(image(0, 0), "");
    ASSERT_DEATH(Image(-1, -1), "");
    
    image = Image(width, height);
    EXPECT_EQ(320, image.width());
    EXPECT_EQ(240, image.height());
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            EXPECT_EQ(0.0, image(x, y).red());
            EXPECT_EQ(0.0, image(x, y).green());
            EXPECT_EQ(0.0, image(x, y).blue());
        }
    }

    Image rand(160, 120);
    for (int y = 0; y < rand.height(); y++) {
        for (int x = 0; x < rand.width(); x++) {
            rand.pixel(x, y) = Color(rng.nextReal(), rng.nextReal(), rng.nextReal());
        }
    }

    image = rand;
    EXPECT_EQ(rand.width(), image.width());
    EXPECT_EQ(rand.height(), image.height());
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            EXPECT_EQ(rand(x, y).red(), image(x, y).red());
            EXPECT_EQ(rand(x, y).green(), image(x, y).green());
            EXPECT_EQ(rand(x, y).blue(), image(x, y).blue());
        }
    }
}
