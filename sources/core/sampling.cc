#define SPICA_API_EXPORT
#include "sampling.h"

#include <typeinfo>

#include "random_queue.h"
#include "hash_grid.h"

#include "point2d.h"
#include "../renderer/renderer_helper.h"

namespace spica {

    namespace {
        
        Random rng = Random();

    }  // anonymous namespace

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


    namespace sampler {

        void onSphere(const Sphere& sphere, Point* position, Normal* normal) {
            double r1 = rng.nextReal();
            double r2 = rng.nextReal();
            onSphere(sphere, position, normal, r1, r2);
        }

        void onSphere(const Sphere& sphere, Point* position, Normal* normal, double r1, double r2) {
            double s = 2.0 * r1 - 1.0;
            double c = sqrt(1.0 - s * s);
            double p = 2.0 * PI * r2;
            double x = c * cos(p);
            double y = c * sin(p);
            double z = s;

            *normal = Normal(x, y, z);
            *position = sphere.radius() * Point(x, y, z) + sphere.center();
        }

        void onDisk(const Disk& disk, Point* position, Normal* normal) {
            double r0 = sqrt(rng.nextReal());
            double r1 = rng.nextReal() * (2.0 * PI);
            double rx = disk.radius() * r0 * cos(r1);
            double ry = disk.radius() * r0 * sin(r1);
            Vector3D u, v, w;
            w = static_cast<Vector3D>(disk.normal());
            if (std::abs(w.x()) > EPS) {
                u = Vector3D::cross(Vector3D(0.0, 1.0, 0.0), w).normalized();
            } else {
                u = Vector3D::cross(Vector3D(1.0, 0.0, 0.0), w).normalized();
            }
            v = Vector3D::cross(u, w);

            *position = disk.center() + u * rx + v * ry;
            *normal = disk.normal();
        }

        void onTriangle(const Triangle& tri, Point* position, Normal* normal, double r1, double r2) {
            if (r1 + r2 >= 1.0) {
                r1 = 1.0 - r1;
                r2 = 1.0 - r2;
            }
            (*position) = tri[0] +  r1 * (tri[1] - tri[0]) + r2 * (tri[2] - tri[0]);
            (*normal) = tri.normal();
        }

        void onTriangle(const Triangle& tri, Point* position, Normal* normal) {
            double r1 = rng.nextReal();
            double r2 = rng.nextReal();
            onTriangle(tri, position, normal, r1, r2);
        }

        void onQuad(const Quad& quad, Point* position, Normal* normal, double r1, double r2) {
            // TODO: this sampler can properly work only for rectangles and squares
            const Vector3D e1 = quad[1] - quad[0];
            const Vector3D e2 = quad[3] - quad[0];
            *position = quad[0] + r1 * e1 + r2 * e2;
            *normal = quad.normal();
        }

        void onQuad(const Quad& quad, Point* position, Normal* normal) {
            double r1 = rng.nextReal();
            double r2 = rng.nextReal();
            onQuad(quad, position, normal, r1, r2);            
        }

        void on(const Shape* primitive, Point* position, Normal* normal, double r1, double r2) {
            if (typeid(*primitive) == typeid(Quad)) {
                const Quad* quad = reinterpret_cast<const Quad*>(primitive);
                onQuad(*quad, position, normal, r1, r2);
            } else if (typeid(*primitive) == typeid(Sphere)) {
                const Sphere* sphere = reinterpret_cast<const Sphere*>(primitive);
                onSphere(*sphere, position, normal, r1, r2);
            } else {
                const std::string typname = typeid(*primitive).name();
                Assertion(false, ("Invalid geometry type: " + typname).c_str());
            }
        }

        void on(const Shape* primitive, Point* position, Normal* normal) {
            if (typeid(*primitive) == typeid(Sphere)) {
                const Sphere* sphere = reinterpret_cast<const Sphere*>(primitive);
                onSphere(*sphere, position, normal);
            } else if (typeid(*primitive) == typeid(Quad)) {
                const Quad* quad = reinterpret_cast<const Quad*>(primitive);
                onQuad(*quad, position, normal);
            } else if (typeid(*primitive) == typeid(Disk)) {
                const Disk* disk = reinterpret_cast<const Disk*>(primitive);
                onDisk(*disk, position, normal);
            } else {
                const std::string typname = typeid(*primitive).name();
                Assertion(false, ("Invalid geometry type: " + typname).c_str());
            }
        }

        void poissonDisk(const std::vector<Triangle>& triangles, const double minDist, std::vector<Point>* points, std::vector<Normal>* normals) {
            // Sample random points on trimesh
            BBox bbox;
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
                    Point p = tri[0] + u * (tri[1] - tri[0]) + v * (tri[2] - tri[0]);
                    candPoints.push_back(p);
                    candNormals.push_back(tri.normal());
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

    }  // namespace sampler

}  // namespace spica
