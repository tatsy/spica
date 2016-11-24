#include "gtest/gtest.h"

#include <memory>
#include <string>
#include <type_traits>

#include "spica.h"
using namespace spica;

// -----------------------------------------------------------------------------
// Scene Typed Tests
// -----------------------------------------------------------------------------

template <class AccelType>
class SceneTypedTest : public ::testing::Test {
protected:
    SceneTypedTest() {}

    virtual ~SceneTypedTest() {}

    virtual void SetUp() {
        std::vector<std::shared_ptr<Primitive>> prims;
        std::vector<std::shared_ptr<Light>> lights;
        auto s1 = std::make_shared<Sphere>(Point3d(0.0, 0.0, 0.0), 5.0);
        auto s2 = std::make_shared<Sphere>(Point3d(0.0, 10.0, 0.0), 2.0);
        prims.emplace_back(new GeometricPrimitive(s1, nullptr, nullptr));
        prims.emplace_back(new GeometricPrimitive(s2, nullptr, nullptr));
        auto bvh = std::make_shared<AccelType>(prims);
        this->scene = Scene(bvh, lights);
    }

    virtual void TearDown() {
    }

protected:
    Scene scene;

private:
    static_assert(std::is_base_of<Accelerator, AccelType>::value,
                  "Type should derive Accelerator!!");
};

TYPED_TEST_CASE_P(SceneTypedTest);

TYPED_TEST_P(SceneTypedTest, IntersectionTest) {
    SurfaceInteraction isect;
    Ray ray(Point3d(0.0, 0.0, 10.0), Vector3d(0.0, 0.0, -1.0));
    EXPECT_TRUE(this->scene.intersect(ray));
    EXPECT_TRUE(this->scene.intersect(ray, &isect));

    EXPECT_NE(isect.primitive(), nullptr);
    EXPECT_EQ(Point3d(0.0, 0.0, 5.0), isect.pos());

    ray = Ray(Point3d(0.0, 0.0, 10.0), Vector3d(0.0, 1.0, 0.0));
    EXPECT_FALSE(this->scene.intersect(ray));
    EXPECT_FALSE(this->scene.intersect(ray, &isect));
}

REGISTER_TYPED_TEST_CASE_P(SceneTypedTest, IntersectionTest);

using AccelTypes = ::testing::Types<BBVHAccel, QBVHAccel>;
INSTANTIATE_TYPED_TEST_CASE_P(, SceneTypedTest, AccelTypes);


// -----------------------------------------------------------------------------
// Scene Value-parameterized Tests
// -----------------------------------------------------------------------------

class SceneIntersectionTest : public ::testing::TestWithParam<std::string> {
protected:
    SceneIntersectionTest()
        : nTrial{ 100 } {
    }

    virtual ~SceneIntersectionTest() {
    }

    virtual void SetUp() {
        MeshIO meshio;
        auto groups = meshio.load(kDataDirectory + GetParam());
        for (const auto& g : groups) {
            for (const auto& s : g.shapes()) {
                prims.emplace_back(new GeometricPrimitive(s, nullptr, nullptr));            
            }
        }
    }

    virtual void TearDown() {
    }

    int nTrial;
    std::vector<std::shared_ptr<Primitive>> prims;
    std::vector<std::shared_ptr<Light>> lights;
};

TEST_P(SceneIntersectionTest, DISABLED_BBVHvsQBVH) {
    auto bvh = std::make_shared<BBVHAccel>(prims);
    auto qbvh = std::make_shared<QBVHAccel>(prims);
    Scene scene1(bvh, lights);
    Scene scene2(qbvh, lights);

    Random rng((unsigned int)time(0));

    Bounds3d bounds = scene1.worldBound();
    Vector3d bsize = bounds.posMax() - bounds.posMin();
    Point3d centroid = bounds.posMin() + bsize * 0.5;
    bounds = Bounds3d(centroid - bsize, centroid + bsize);

    Point3d f(0.0, 0.0, 2.0);
    Point3d t(0.0, 0.0, -2.0);
    Vector3d dir = (t - f).normalized();
    Ray ray(f, dir);

    for (int i = 0; i < nTrial; i++) {
        Point3d f = bounds.posMin() + Vector3d(bsize.x() * rng.get1D(), bsize.y() * rng.get1D(), bsize.z() * rng.get1D());
        Point3d t = bounds.posMin() + Vector3d(bsize.x() * rng.get1D(), bsize.y() * rng.get1D(), bsize.z() * rng.get1D());
        Vector3d dir = (t - f).normalized();
        Ray ray(f, dir);

        SurfaceInteraction isect1, isect2;
        EXPECT_EQ(scene1.intersect(ray), scene2.intersect(ray));
        EXPECT_EQ(scene1.intersect(ray, &isect1), scene2.intersect(ray, &isect2));
        EXPECT_EQ(isect1.primitive(), isect2.primitive());
        EXPECT_EQ(isect1.pos(), isect2.pos());
    }
}

INSTANTIATE_TEST_CASE_P(, SceneIntersectionTest,
                        ::testing::Values(std::string("box.ply"),
                                          std::string("bunny.ply"), 
                                          std::string("kitten.ply")));
