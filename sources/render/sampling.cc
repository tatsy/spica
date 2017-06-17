#define SPICA_API_EXPORT
#include "sampling.h"

#include <typeinfo>

#include "core/random_queue.h"
#include "core/hash_grid.h"

#include "core/memory.h"
#include "core/point2d.h"
#include "render/interaction.h"

//#include "../material/material.h"
//#include "../scenes/scene.h"
//#include "../core/camera.h"

namespace spica {

Distribution1D::Distribution1D()
    : func_{}
    , cdf_{}
    , integral_{0.0} {
}

Distribution1D::Distribution1D(const std::vector<double>& data)
    : func_{data}
    , cdf_{}
    , integral_{0.0} {
    const int n = static_cast<int>(data.size());
    cdf_.resize(n + 1);

    // Compute cumulative distribution function
    cdf_[0] = 0.0;
    for (int i = 1; i < n + 1; i++) {
        cdf_[i] = cdf_[i - 1] + func_[i - 1] / n;
    }

    integral_ = cdf_[n];
    if (integral_ == 0.0) {
        for (int i = 1; i < n + 1; i++) {
            cdf_[i] = static_cast<double>(i) / n;
        } 
    } else {
        for (int i = 1; i < n + 1; i++) {
            cdf_[i] /= integral_;
        }
    }
}

Distribution1D::Distribution1D(const Distribution1D& d)
    : Distribution1D{} {
    this->operator=(d);
}

Distribution1D::~Distribution1D() {
}

Distribution1D& Distribution1D::operator=(const Distribution1D& d) {
    this->func_ = d.func_;
    this->cdf_  = d.cdf_;
    this->integral_ = d.integral_;
    return *this;
}

double Distribution1D::operator()(int i) const {
    Assertion(i >= 0 && i < func_.size(), "Index out of bounds!!");
    return func_[i];
}

double Distribution1D::sample(double rand, double* pdf, int* offset) const {
    int off = findInterval(cdf_, rand);
    if (offset) *offset = off;
        
    double du = rand - cdf_[off];
    if(cdf_[off + 1] - cdf_[off] > 0.0) {
        du /= (cdf_[off + 1] - cdf_[off]);
    }

    *pdf    = func_[off] / integral_;
    return (off + du) / func_.size();
}

int Distribution1D::sampleDiscrete(double rand, double* pdf) const {
    int off = findInterval(cdf_, rand);
    if (pdf) *pdf = func_[off] / (integral_ * count());
    return off;
}

double Distribution1D::pdfDiscrete(int index) const {
    Assertion(index >= 0 && index < count(), "Index out of bounds");
    return func_[index] / (integral_ * count());
}

int Distribution1D::findInterval(const std::vector<double>& cdf, double v) {
    int ret = std::upper_bound(cdf.begin(), cdf.end(), v) - cdf.begin() - 1;    
    return std::min(ret, (int)cdf.size() - 1);
}

Distribution2D::Distribution2D()
    : pCond_{}
    , pMarg_{} {
}

Distribution2D::Distribution2D(const std::vector<double>& data, int width, int height)
    : pCond_{}
    , pMarg_{} {
    pCond_.reserve(height);

    auto it = data.begin();
    for (int y = 0; y < height; y++) {
        pCond_.emplace_back(std::vector<double>(it, it + width));
        it += width;
    }

    std::vector<double> mergFunc(height);
    for (int y = 0; y < height; y++) {
        mergFunc[y] = pCond_[y].integral();
    }
    pMarg_ = Distribution1D(mergFunc);
}

Distribution2D::Distribution2D(const Distribution2D& d)
    : Distribution2D{} {
    this->operator=(d);
}

Distribution2D::~Distribution2D() {
}

Distribution2D& Distribution2D::operator=(const Distribution2D& d) {
    this->pCond_ = d.pCond_;
    this->pMarg_ = d.pMarg_;
    return *this;
}

Point2d Distribution2D::sample(const Point2d& rands, double* pdf) const {
    double pdfs[2];
    int v;
    const double d1 = pMarg_.sample(rands[1], &pdfs[1], &v);
    const double d0 = pCond_[v].sample(rands[0], &pdfs[0]);
    *pdf = pdfs[0] * pdfs[1];
    return Point2d(d0, d1);
}

double Distribution2D::pdf(const Point2d& p) const {
    const int iu = clamp(static_cast<int>(p[0] * pCond_[0].count()), 0, pCond_[0].count() - 1);
    const int iv = clamp(static_cast<int>(p[1] * pMarg_.count()), 0, pMarg_.count() - 1);
    return pCond_[iv](iu) / pMarg_.integral();
}

Point2d sampleConcentricDisk(const Point2d& rands) {
    Point2d uOffset = 2.0 * rands - Point2d(1.0, 1.0);
    if (uOffset.x() == 0.0 && uOffset.y() == 0.0) return Point2d(0.0, 0.0);

    double theta, r;
    if (std::abs(uOffset.x()) > std::abs(uOffset.y())) {
        r = uOffset.x();
        theta = PI * (uOffset.y() / uOffset.x()) / 4.0;
    } else {
        r = uOffset.y();
        theta = (PI / 2.0) - PI * (uOffset.x() / uOffset.y()) / 4.0;
    }
    return r * Point2d(std::cos(theta), std::sin(theta));
}

Vector3d sampleUniformSphere(const Point2d& rands) {
    double z = 2.0 * rands[0] - 1.0;
    double cosTheta = sqrt(1.0 - z * z);
    double phi = 2.0 * PI * rands[1];
    double x = cos(phi) * cosTheta;
    double y = sin(phi) * cosTheta;
    return { x, y, z };
}

Vector3d sampleCosineHemisphere(const Point2d& rands) {
    Point2d d = sampleConcentricDisk(rands);
    double z = sqrt(std::max(0.0, 1.0 - d.x() * d.x() - d.y() * d.y()));
    return Vector3d{ d.x(), d.y(), z };
}

void sampleUniformHemisphere(const Normal3d& normal, Vector3d* direction, const Point2d& rands) {
    Vector3d u, v, w;
    w = static_cast<Vector3d>(normal);
    vect::coordinateSystem(w, &u, &v);

    const double t = 2.0 * PI * rands[0];
    const double z2 = rands[1];
    const double z2s = sqrt(z2);
    *direction = (u * cos(t) * z2s + v * sin(t) * z2s + w * sqrt(1.0 - z2)).normalized();        
}

/*
void samplePoissonDisk(const Scene& scene, const Point3d& pCamera,
                       double minDist, std::vector<Interaction>* points) {
    std::vector<Triangle> tris;
    for (const auto& p : scene.primitives()) {
        if (p->material() && p->material()->isSubsurface()) {
            auto ts = p->triangulate();
            tris.insert(tris.end(), ts.begin(), ts.end());
        }
    }

    // Initialize utility variables in this method
    const auto seed = static_cast<unsigned int>(time(0));
    Random rng(seed);

    // Sample random points on trimesh
    Bounds3d bounds;
    std::vector<Point3d> candPoints;
    std::vector<Normal3d> candNormals;
    for (int i = 0; i < tris.size(); i++) {
        const Triangle& tri = tris[i];
        const double A = tri.area();
        const int nSample = static_cast<int>(std::ceil(4.0 * A / (minDist * minDist)));
        for (int k = 0; k < nSample; k++) {
            double u = rng.nextReal();
            double v = rng.nextReal();
            if (u + v >= 1.0) {
                u = 1.0 - u;
                v = 1.0 - v;
            }
            Point3d  p = (1.0 - u - v) * tri[0] + u * tri[1] + v * tri[2];
            Normal3d n = (1.0 - u  -v) * tri.normal(0) + u * tri.normal(1) + 
                         v * tri.normal(2);
            candPoints.push_back(p);
            candNormals.push_back(n);
            bounds.merge(p);
        }
    }

    // Create hash grid
    const int numCands = static_cast<int>(candPoints.size());
    Vector3d bsize = bounds.posMax() - bounds.posMin();
    const double scale = 1.0 / (2.0 * minDist);
    const int numPoints = candPoints.size();
    HashGrid<int> hashgrid;
    hashgrid.init(numPoints, scale, bounds);

    RandomQueue<int> que;
    for (int i = 0; i < numCands; i++) {
        que.push(i);
    }

    std::vector<int> sampledIDs;
    Vector3d margin(2.0 * minDist, 2.0 * minDist, 2.0 * minDist);
    while (!que.empty()) {
        int id = que.pop();
        Point3d v = candPoints[id];
        const std::vector<int>& cellvs = hashgrid[v];

        bool accept = true;
        for (int k = 0; k < cellvs.size(); k++) {
            if ((candPoints[cellvs[k]] - v).squaredNorm() < minDist * minDist) {
                accept = false;
                break;
            }
        }

        if (accept) {
            Point3d boxMin = v - margin;
            Point3d boxMax = v + margin;
            hashgrid.add(id, boxMin, boxMax);
            sampledIDs.push_back(id);
        }
    }

    // Store sampled points
    std::vector<int>::iterator it;
    for (auto i : sampledIDs) {
        points->emplace_back(candPoints[i], candNormals[i]);
    }
}

void samplePoissonDisk(const Scene& scene, const Point3d& pCamera,
                       double minDist, std::vector<SurfaceInteraction>* points) {
    // Initialize utility variables in this method
    const auto seed = static_cast<unsigned int>(time(0));
    Random rng(seed);

    const int maxDepth = 30;
    const int maxFails = 2000;
    const int pathTracePerIter = 20000;

    // Initialize hash grid
    Bounds3d bounds = scene.worldBound();
    Vector3d bsize = bounds.posMax() - bounds.posMin();
    const double scale = 1.0 / (2.0 * minDist);
    const int numPoints = pathTracePerIter * 5;
    Vector3d margin(2.0 * minDist, 2.0 * minDist, 2.0 * minDist);
    HashGrid<Point3d> hashgrid;
    hashgrid.init(numPoints, scale, bounds);

    MemoryArena arena;
    int repeatFails = 0;
    for (;;) {
        // Collect candidate intersections
        std::vector<SurfaceInteraction> candidates;
        for (int p = 0; p < pathTracePerIter; p++) {
            Vector3d dir = sampleUniformSphere(rng.get2D());
            Ray ray(pCamera, dir);
            for (int depth = 0; depth < maxDepth; depth++) {
                SurfaceInteraction isect;
                if (!scene.intersect(ray, &isect)) {
                    break;
                }

                if (isect.primitive()->material()->isSubsurface()) {
                    candidates.push_back(isect);                    
                }

                Vector3d dir = sampleUniformSphere(rng.get2D());
                if (vect::dot(dir, isect.normal()) < 0.0) {
                    dir = -dir;
                }
                ray = isect.spawnRay(dir);
            }
        }

        // Check poisson disk criteria
        const int nCandidates = static_cast<int>(candidates.size());
        for (int i = 0; i < nCandidates; i++) {
            const auto& nearPoints = hashgrid[candidates[i].pos()];
            bool accept = true;
            for (auto q : nearPoints) {
                if ((candidates[i].pos() - q).squaredNorm() < minDist * minDist) {
                    accept = false;
                    break;
                }
            }

            if (accept) {
                repeatFails = 0;
                Point3d boxMin = candidates[i].pos() - margin;
                Point3d boxMax = candidates[i].pos() + margin;
                hashgrid.add(candidates[i].pos(), boxMin, boxMax);
                points->push_back(candidates[i]);                
            } else {
                repeatFails++;
                if (repeatFails >= maxFails) {
                    return;
                }
            }
        }
    }
}
*/

}  // namespace spica
