#include "gtest/gtest.h"
#include "../include/spica.h"
using namespace spica;

TEST(ImageTest, InstanceTest) {
	Image image;
	EXPECT_EQ(0, image.width());
	EXPECT_EQ(0, image.height());
	ASSERT_DEATH(image(0, 0), "");
	ASSERT_DEATH(Image(-1, -1), "");
	
	image = Image(320, 240);
	EXPECT_EQ(320, image.width());
	EXPECT_EQ(240, image.height());
	for (int y = 0; y < image.height(); y++) {
		for (int x = 0; x < image.width(); x++) {
			EXPECT_EQ(0.0, image(x, y).red());
			EXPECT_EQ(0.0, image(x, y).green());
			EXPECT_EQ(0.0, image(x, y).blue());
		}
	}
}
