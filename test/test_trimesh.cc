#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

namespace {
    
    bool trimeshIsectGT(const Trimesh& trimesh, const Ray& ray, Hitpoint* hitpoint) {
        bool ret = false;
        for (int i = 0; i < trimesh.numFaces(); i++) {
            Triangle tri = trimesh.getTriangle(i);
            Hitpoint hpTemp;
            if (tri.intersect(ray, &hpTemp)) {
                if (hitpoint->distance() > hpTemp.distance() && vect::dot(ray.direction(), tri.normal()) < 0.0) {
                    *hitpoint = hpTemp;
                    ret = true;
                }
            }
        }
        return ret;
    }

}  // anonymous namespace

class TrimeshTest : public ::testing::Test {
protected:
    TrimeshTest() {}
    virtual ~TrimeshTest() {}
};

// ------------------------------
// Trimesh class test
// ------------------------------

TEST_F(TrimeshTest, InvalidLoad) {
    Trimesh trimesh;
    ASSERT_DEATH(trimesh.load(kDataDirectory + "box.obj"), "");
    ASSERT_DEATH(trimesh.load(kDataDirectory + "bax.ply"), "");
}

TEST_F(TrimeshTest, VertexFaceIdInstance) {
    std::vector<Point> vertices;
    vertices.push_back(Point(-10.0, 0.0, -10.0));
    vertices.push_back(Point(-10.0, 0.0,  10.0));
    vertices.push_back(Point( 10.0, 0.0, -10.0));
    vertices.push_back(Point( 10.0, 0.0,  10.0));
    std::vector<Triplet> indices = { Triplet(0, 1, 3), Triplet(3, 2, 0) };
    
    Trimesh trimesh(vertices, indices);
    EXPECT_EQ(2, trimesh.numFaces());
    EXPECT_EQ(4, trimesh.numVerts());
    EXPECT_EQ(400.0, trimesh.area());
}

TEST_F(TrimeshTest, LoadTest) {
    Trimesh objmesh, plymesh;
    EXPECT_NO_FATAL_FAILURE(objmesh.load(kDataDirectory + "bunny.obj"));
    EXPECT_NO_FATAL_FAILURE(plymesh.load(kDataDirectory + "bunny.ply"));

    EXPECT_EQ(objmesh.numVerts(), plymesh.numVerts());
    EXPECT_EQ(objmesh.numFaces(), plymesh.numFaces());

    const double tol = 0.01;

    for (int i = 0; i < objmesh.numVerts(); i++) {
        EXPECT_NEAR(objmesh.getVertex(i).x(), plymesh.getVertex(i).x(), tol);
        EXPECT_NEAR(objmesh.getVertex(i).y(), plymesh.getVertex(i).y(), tol);
        EXPECT_NEAR(objmesh.getVertex(i).z(), plymesh.getVertex(i).z(), tol);

        EXPECT_NEAR(objmesh.getNormal(i).x(), plymesh.getNormal(i).x(), tol);
        EXPECT_NEAR(objmesh.getNormal(i).y(), plymesh.getNormal(i).y(), tol);
        EXPECT_NEAR(objmesh.getNormal(i).z(), plymesh.getNormal(i).z(), tol);
    }

    std::vector<Triplet> objids = objmesh.getIndices();
    std::vector<Triplet> plyids = plymesh.getIndices();
    for (int i = 0; i < objmesh.numFaces(); i++) {
        EXPECT_EQ(objids[i][0], plyids[i][0]);
        EXPECT_EQ(objids[i][1], plyids[i][1]);
        EXPECT_EQ(objids[i][2], plyids[i][2]);
    }
}

TEST_F(TrimeshTest, CopyAndMove) {
    Trimesh trimesh(kDataDirectory + "bunny.ply");
    Trimesh trimesh2(trimesh);

    EXPECT_EQ(trimesh.numFaces(), trimesh2.numFaces());
    EXPECT_EQ(trimesh.numVerts(), trimesh2.numVerts());

    for (int i = 0; i < trimesh.numVerts(); i++) {
        EXPECT_EQ_VEC(trimesh.getVertex(i), trimesh2.getVertex(i));
    }

    Trimesh trimesh3(std::move(trimesh2));
    EXPECT_EQ(trimesh.numFaces(), trimesh3.numFaces());
    EXPECT_EQ(trimesh.numVerts(), trimesh3.numVerts());

    for (int i = 0; i < trimesh.numVerts(); i++) {
        EXPECT_EQ_VEC(trimesh.getVertex(i), trimesh3.getVertex(i));
    }

    ASSERT_DEATH(trimesh2.getVertex(0), "");
}

TEST_F(TrimeshTest, BoxIntersection) {
    Trimesh trimesh(kDataDirectory + "box.ply");
    trimesh.buildAccel(AccelType::QBVH);

    Ray ray(Point(0.0, 0.0, 100.0), Vector3D(0.0, 0.0, -1.0));
    Hitpoint hitpoint;
    EXPECT_TRUE(trimesh.intersect(ray, &hitpoint));
    EXPECT_EQ(99.5, hitpoint.distance());
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.5), hitpoint.position());
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 1.0), hitpoint.normal());
}

TEST_F(TrimeshTest, BunnyIntersection) {
    Trimesh trimesh(kDataDirectory + "bunny.ply");
    trimesh.buildAccel(AccelType::KdTree);

    Ray ray(Point(0.0, 0.0, 100.0), Vector3D(0.0, 0.0, -1.0));

    Hitpoint hpGT;
    bool isHit = trimeshIsectGT(trimesh, ray, &hpGT);

    Hitpoint hitpoint;
    EXPECT_EQ(isHit, trimesh.intersect(ray, &hitpoint));
    EXPECT_EQ(hpGT.distance(), hitpoint.distance());

    // Test copied data
    Trimesh cp(trimesh);
    EXPECT_EQ(trimesh.numVerts(), cp.numVerts());
    EXPECT_EQ(trimesh.numFaces(), cp.numFaces());
    for (int i = 0; i < cp.numFaces(); i++) {
        Triangle t1 = trimesh.getTriangle(i);
        Triangle t2 = cp.getTriangle(i);
        for (int k = 0; k < 3; k++) {
            EXPECT_EQ_VEC(t1[k], t2[k]);
        }
    }

    for (int i = 0; i < cp.numVerts(); i++) {
        EXPECT_EQ_VEC(trimesh.getNormal(i), cp.getNormal(i));
    }

    EXPECT_EQ(isHit, cp.intersect(ray, &hitpoint));
    EXPECT_EQ(hpGT.distance(), hitpoint.distance());
}

TEST_F(TrimeshTest, RandomKdTreeIntersection) {
    const int nTrial = 100;
    Random rng = Random();

    Trimesh trimesh;
    trimesh.load(kDataDirectory + "bunny.ply");
    trimesh.buildAccel(AccelType::KdTree);

    for (int i = 0; i < nTrial; i++) {
        Point from   = Point(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 0.0);
        Point to     = Point(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 10.0);
        Vector3D dir = (to - from).normalized();
        Ray ray(from, dir);

        Hitpoint ans;
        bool isHit = trimeshIsectGT(trimesh, ray, &ans);

        Hitpoint hitpoint;
        EXPECT_EQ(isHit, trimesh.intersect(ray, &hitpoint))
            << "  from: " << from.toString() << std::endl
            << "    to: " << to.toString()   << std::endl
            << "   pos: " << ans.position().toString() << std::endl
            << "normal: " << ans.normal().toString()   << std::endl;
        EXPECT_EQ(ans.distance(), hitpoint.distance());
    }
}

TEST_F(TrimeshTest, RandomQVBHIntersection) {
    const int nTrial = 100;
    Random rng = Random();

    Trimesh trimesh;
    trimesh.load(kDataDirectory + "bunny.ply");
    trimesh.buildAccel(AccelType::QBVH);

    for (int i = 0; i < nTrial; i++) {
        Point from  = Point(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 0.0);
        Point to    = Point(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 10.0);
        Vector3D dir = (to - from).normalized();
        Ray ray(from, dir);

        Hitpoint ans;
        bool isHit = trimeshIsectGT(trimesh, ray, &ans);

        Hitpoint hitpoint;
        EXPECT_EQ(isHit, trimesh.intersect(ray, &hitpoint))
            << "  from: " << from.toString() << std::endl
            << "    to: " << to.toString()   << std::endl
            << "   pos: " << ans.position().toString() << std::endl
            << "normal: " << ans.normal().toString()   << std::endl;
        EXPECT_EQ(ans.distance(), hitpoint.distance());
    }
}

TEST_F(TrimeshTest, Translate) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        Trimesh tmesh1(kDataDirectory + "bunny.ply");
        Trimesh tmesh2 = tmesh1;

        Vector3D delta(rng.nextReal() * 2.0 - 1.0,
                       rng.nextReal() * 2.0 - 1.0,
                       rng.nextReal() * 2.0 - 1.0);

        tmesh2.translate(delta);
        for (int i = 0; i < tmesh1.numVerts(); i++) {
            EXPECT_EQ_VEC(tmesh1.getVertex(i) + delta, tmesh2.getVertex(i));
        }
    }
}

TEST_F(TrimeshTest, Scale) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        Trimesh tmesh1(kDataDirectory + "bunny.ply");
        Trimesh tmesh2 = tmesh1;

        const double scale = rng.nextReal() * 2.0 - 1.0;
        tmesh2.scale(scale);
        for (int i = 0; i < tmesh1.numVerts(); i++) {
            EXPECT_EQ_VEC(tmesh1.getVertex(i) * scale, tmesh2.getVertex(i));
        }
    }
}

