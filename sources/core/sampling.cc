#define SPICA_API_EXPORT
#include "sampling.h"

#include <typeinfo>

#include "random_queue.h"
#include "hash_grid.h"

#include "point2d.h"
#include "interaction.h"
#include "../renderer/renderer_helper.h"

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

int Distribution1D::findInterval(const std::vector<double>& cdf, double v) {
    return std::upper_bound(cdf.begin(), cdf.end(), v) - cdf.begin();    
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

Point2D Distribution2D::sample(const Point2D& rands, double* pdf) const {
    double pdfs[2];
    int v;
    const double d1 = pMarg_.sample(rands[1], &pdfs[1], &v);
    const double d0 = pCond_[v].sample(rands[0], &pdfs[0]);
    *pdf = pdfs[0] * pdfs[1];
    return Point2D(d0, d1);
}

double Distribution2D::pdf(const Point2D& p) const {
    const int iu = clamp(static_cast<int>(p[0] * pCond_[0].count()), 0, pCond_[0].count() - 1);
    const int iv = clamp(static_cast<int>(p[1] * pMarg_.count()), 0, pMarg_.count() - 1);
    return pCond_[iv](iv) / pMarg_.integral();
}

void sampleUniformHemisphere(const Normal& normal, Vector3D* direction, const Point2D& rands) {
    Vector3D u, v, w;
    w = static_cast<Vector3D>(normal);
    helper::calcLocalCoords(w, &u, &v);

    const double t = 2.0 * PI * rands[0];
    const double z2 = rands[1];
    const double z2s = sqrt(z2);
    *direction = (u * cos(t) * z2s + v * sin(t) * z2s + w * sqrt(1.0 - z2)).normalized();        
}

void samplePoissonDisk(const std::vector<Triangle>& triangles, const double minDist, std::vector<Point>* points, std::vector<Normal>* normals) {
    Random rng((unsigned int)time(0));

    // Sample random points on trimesh
    Bound3d bbox;
    std::vector<Point> candPoints;
    std::vector<Normal> candNormals;
    for (int i = 0; i < triangles.size(); i++) {
        const Triangle& tri = triangles[i];
        const double A = tri.area();
        const int nSample = static_cast<int>(std::ceil(4.0 * A / (minDist * minDist)));
        for (int k = 0; k < nSample; k++) {
            double u = rng.nextReal();
            double v = rng.nextReal();
            if (u + v >= 1.0) {
                u = 1.0 - u;
                v = 1.0 - v;
            }

            Point p = (1.0 - u - v) * tri[0] + u * tri[1] + v * tri[2];
            Normal n = (1.0 - u - v) * tri.normal(0) + u * tri.normal(1) + v * tri.normal(2);
            candPoints.push_back(p);
            candNormals.push_back(n);
            bbox.merge(p);
        }
    }

    // Create hash grid
    const int numCands = static_cast<int>(candPoints.size());
    Vector3D bsize = bbox.posMax() - bbox.posMin();
    const double scale = 1.0 / (2.0 * minDist);
    const int numPoints = candPoints.size();
    HashGrid<int> hashgrid;
    hashgrid.init(numPoints, scale, bbox);

    RandomQueue<int> que;
    for (int i = 0; i < numCands; i++) {
        que.push(i);
    }

    std::vector<int> sampledIDs;
    Vector3D marginv(2.0 * minDist, 2.0 * minDist, 2.0 * minDist);
    while (!que.empty()) {
        int id = que.pop();
        Point v = candPoints[id];
        const std::vector<int>& cellvs = hashgrid[v];

        bool accept = true;
        for (int k = 0; k < cellvs.size(); k++) {
            if ((candPoints[cellvs[k]] - v).squaredNorm() <= minDist * minDist) {
                accept = false;
                break;
            }
        }

        if (accept) {
            Point boxMin = v - marginv;
            Point boxMax = v + marginv;
            hashgrid.add(id, boxMin, boxMax);
            sampledIDs.push_back(id);
        }
    }

    // Store sampled points
    std::vector<int>::iterator it;
    for (it = sampledIDs.begin(); it != sampledIDs.end(); ++it) {
        points->push_back(candPoints[*it]);
        normals->push_back(candNormals[*it]);
    }
}

}  // namespace spica
