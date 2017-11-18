#include "gtest/gtest.h"
#include "spica.h"
#include "test_params.h"
using namespace spica;

#include <string>

#include "filesystem/path.h"
namespace fs = filesystem;

namespace {

auto sampler = std::make_unique<Random>((unsigned int)time(0));
const std::string filepath = TEMP_DIRECTORY + "test_image.bmp";
const std::string hdrpath  = TEMP_DIRECTORY + "test_hdr.hdr";

}  // Anonymous namespace

class ImageTest : public ::testing::Test {
protected:
    const int width;
    const int height;

protected:
    ImageTest() 
        : width(320)
        , height(240) {
    }

    ~ImageTest() {}

    void SetUp() {
        fs::create_directory(fs::path(TEMP_DIRECTORY));
    }

    void randomImage(Image* rand) {
        rand->resize(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                rand->pixel(x, y) = RGBSpectrum(sampler->get1D(),
                                                sampler->get1D(),
                                                sampler->get1D());
            }
        }
    }
};

// --------------------------------------------------
// Image 
// --------------------------------------------------

TEST_F(ImageTest, DefaultInstance) {
    Image image;
    EXPECT_EQ(0, image.width());
    EXPECT_EQ(0, image.height());
    ASSERT_DEATH(image(0, 0), "");
}

TEST_F(ImageTest, InstanceWithSize) {
    Image image(width, height);
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

TEST_F(ImageTest, CopyAndAssign) {
    Image rand;
    randomImage(&rand);

    Image image(rand);
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

TEST_F(ImageTest, Move) {
    Image rand;
    randomImage(&rand);

    Image copy(rand);
    Image image(std::move(rand));
    EXPECT_EQ(width, image.width());
    EXPECT_EQ(height, image.height());
    EXPECT_EQ(0, rand.width());
    EXPECT_EQ(0, rand.height());
    ASSERT_DEATH(rand(0, 0), "");
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            EXPECT_EQ(copy(x, y).red(), image(x, y).red());
            EXPECT_EQ(copy(x, y).green(), image(x, y).green());
            EXPECT_EQ(copy(x, y).blue(), image(x, y).blue());
        }
    }
}

TEST_F(ImageTest, InvalidPathToLoad) {
    Image image;
    try {
        image.load("dammy_path.bmp");
        FAIL();
    } catch (const RuntimeException &e) {}

    try {
        image.load("image.jpg");
        FAIL();
    } catch (const RuntimeException &e) {}
}

TEST_F(ImageTest, Resize) {
    Image image;
    image.resize(width, height);
    EXPECT_EQ(width, image.width());
    EXPECT_EQ(height, image.height());
}

TEST_F(ImageTest, Fill) {
    Image image(width, height);
    image.fill(RGBSpectrum(1.0, 2.0, 3.0));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            EXPECT_EQ(1.0, image(x, y).red());
            EXPECT_EQ(2.0, image(x, y).green());
            EXPECT_EQ(3.0, image(x, y).blue());
        }
    }
}

TEST_F(ImageTest, SaveAndLoad) {
    Image image(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            image.pixel(x, y) = RGBSpectrum(sampler->get1D(),
                                            sampler->get1D(),
                                            sampler->get1D());
        }
    }
    image.save(filepath);

    Image loaded;
    loaded.load(filepath);
    EXPECT_EQ(image.width(), loaded.width());
    EXPECT_EQ(image.height(), loaded.height());
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            EXPECT_NEAR(image(x, y).red(),   loaded(x, y).red(),   0.01) << "Red is different";        
            EXPECT_NEAR(image(x, y).green(), loaded(x, y).green(), 0.01) << "Green is different";        
            EXPECT_NEAR(image(x, y).blue(),  loaded(x, y).blue(),  0.01) << "Blue is different";        
        }
    }

    image.save(hdrpath);
    loaded.load(hdrpath);
    EXPECT_EQ(image.width(), loaded.width());
    EXPECT_EQ(image.height(), loaded.height());
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            EXPECT_NEAR(image(x, y).red(),   loaded(x, y).red(),   0.01) << "Red is different";        
            EXPECT_NEAR(image(x, y).green(), loaded(x, y).green(), 0.01) << "Green is different";        
            EXPECT_NEAR(image(x, y).blue(),  loaded(x, y).blue(),  0.01) << "Blue is different";        
        }
    }
}

TEST_F(ImageTest, Bilateral) {
    Image image;
    image.load(DATA_DIRECTORY + "lamp.png");

    Image result;
    double sigma_s = std::max(image.width(), image.height()) * 0.02;
    EXPECT_NO_FATAL_FAILURE(birateral(image, &result, sigma_s, 0.4));

    result.save(TEMP_DIRECTORY + "birateral.png");
}

TEST_F(ImageTest, ReinhardTmo) {
    Image image;
    image.load(DATA_DIRECTORY + "memorial.hdr");

    ReinhardTmo tmo;
    EXPECT_NO_FATAL_FAILURE(image = tmo.apply(image));
    EXPECT_NO_FATAL_FAILURE(image = GammaTmo(2.2).apply(image));
    image.save(TEMP_DIRECTORY + "reinhard.png");
}

TEST_F(ImageTest, DragoTmo) {
    Image image;
    image.load(DATA_DIRECTORY + "memorial.hdr");

    DragoTmo tmo;
    EXPECT_NO_FATAL_FAILURE(image = tmo.apply(image));
    EXPECT_NO_FATAL_FAILURE(image = GammaTmo(2.2).apply(image));
    image.save(TEMP_DIRECTORY + "drago.png");
}

TEST_F(ImageTest, DurandTmo) {
    Image image;
    image.load(DATA_DIRECTORY + "memorial.hdr");

    DurandTMO tmo;
    EXPECT_NO_FATAL_FAILURE(image = tmo.apply(image));
    EXPECT_NO_FATAL_FAILURE(image = GammaTmo(2.2).apply(image));
    image.save(TEMP_DIRECTORY + "durand.png");
}
