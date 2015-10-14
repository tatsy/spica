#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

TEST(PathTest, Extension) {
    const std::string filename = "abc/def.txt";
    EXPECT_EQ(".txt", path::getExtension(filename));
}

TEST(PathTest, Directory) {
    const std::string filename = "abc/def.txt";
    EXPECT_EQ("abc/", path::getDirectory(filename));
}
