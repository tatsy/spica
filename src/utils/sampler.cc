#define SPICA_SAMPLER_EXPORT
#include "sampler.h"

#include <typeinfo>

#include "random_queue.h"
#include "hash_grid.h"

namespace spica {

    namespace {
        
        Random rng = Random();

    }  // anonymous namespace

    namespace sampler {

        void onHemisphere(const Vector3& normal, Vector3* direction) {
            double r1 = rng.nextReal();
            double r2 = rng.nextReal();
            onHemisphere(normal, direction, r1, r2);
        }

        void onHemisphere(const Vector3& normal, Vector3* direction, double r1, double r2) {
            Vector3 u, v, w;
            w = normal;
            if (abs(w.x()) > EPS) {
                u = Vector3(0.0, 1.0, 0.0).cross(w).normalized();
            } else {
                u = Vector3(1.0, 0.0, 0.0).cross(w).normalized();
            }

            v = w.cross(u);

            const double t = 2.0 * PI * r1;
            const double z2 = r2;
            const double z2s = sqrt(z2);
            *direction = (u * cos(t) * z2s + v * sin(t) * z2s + w * sqrt(1.0 - z2)).normalized();        
        }

        void onSphere(const Sphere& sphere, Vector3* position, Vector3* normal) {
            double s = 2.0 * rng.nextReal() - 1.0;
            double c = sqrt(1.0 - s * s);
            double p = 2.0 * PI * rng.nextReal();
            double x = c * cos(p);
            double y = c * sin(p);
            double z = s;

            *normal = Vector3(x, y, z);
            *position = sphere.radius() * Vector3(x, y, z) + sphere.center();
        }

        void onDisk(const Disk& disk, Vector3* position, Vector3* normal) {
            double r0 = sqrt(rng.nextReal());
            double r1 = rng.nextReal() * (2.0 * PI);
            double rx = disk.radius() * r0 * cos(r1);
            double ry = disk.radius() * r0 * sin(r1);
            Vector3 u, v, w;
            w = disk.normal();
            if (abs(w.x()) > EPS) {
                u = Vector3::cross(Vector3(0.0, 1.0, 0.0), w).normalized();
            } else {
                u = Vector3::cross(Vector3(1.0, 0.0, 0.0), w).normalized();
            }
            v = Vector3::cross(u, w);

            *position = disk.center() + u * rx + v * ry;
            *normal = disk.normal();
        }

        void onTriangle(const Triangle& tri, Vector3* position, Vector3* normal, double r1, double r2) {
            if (r1 + r2 >= 1.0) {
                r1 = 1.0 - r1;
                r2 = 1.0 - r2;
            }
            (*position) = r1 * (tri.p1() - tri.p0()) + r2 * (tri.p2() - tri.p0());
            (*position) = tri.normal();
        }

        void onQuad(const Quad& quad, Vector3* position, Vector3* normal, double r1, double r2) {
            // TODO: this sampler can properly work only for rectangles and squares
            Vector3 e1 = quad.p1() - quad.p0();
            Vector3 e2 = quad.p3() - quad.p0();
            *position = quad.p0() + r1 * e1 + r2 * e2;
            *normal = quad.normal();
        }

        void onQuad(const Quad& quad, Vector3* position, Vector3* normal) {
            double r1 = rng.nextReal();
            double r2 = rng.nextReal();
            onQuad(quad, position, normal, r1, r2);            
        }

        void on(const Primitive* primitive, Vector3* position, Vector3* normal, double r1, double r2) {
            if (typeid(*primitive) == typeid(Quad)) {
                const Quad* quad = reinterpret_cast<const Quad*>(primitive);
                onQuad(*quad, position, normal, r1, r2);
            } else {
                const std::string typname = typeid(*primitive).name();
                msg_assert(false, ("Invalid geometry type: " + typname).c_str());
            }
        }

        void on(const Primitive* primitive, Vector3* position, Vector3* normal) {
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
                msg_assert(false, ("Invalid geometry type: " + typname).c_str());
            }
        }

        void poissonDisk(const Trimesh& trimesh, const double minDist, std::vector<Vector3>* points, std::vector<Vector3>* normals) {
            // Sample random points on trimesh
            BBox bbox;
            std::vector<Vector3> candPoints;
            std::vector<Vector3> candNormals;
            for (int i = 0; i < trimesh.numFaces(); i++) {
                Triangle tri = trimesh.getTriangle(i);
                const double A = tri.area();
                const int nSample = static_cast<int>(std::ceil(4.0 * A / (minDist * minDist)));
                for (int k = 0; k < nSample; k++) {
                    double u = rng.nextReal();
                    double v = rng.nextReal();
                    if (u + v >= 1.0) {
                        u = 1.0 - u;
                        v = 1.0 - v;
                    }
                    Vector3 p = tri.p0() + u * (tri.p1() - tri.p0()) + v * (tri.p2() - tri.p0());
                    candPoints.push_back(p);
                    candNormals.push_back(tri.normal());
                    bbox.merge(p);
                }
            }

            // Create hash grid
            const int numCands = static_cast<int>(candPoints.size());
            Vector3 bsize = bbox.posMax() - bbox.posMin();
            const double scale = 1.0 / (2.0 * minDist);
            const int numPoints = candPoints.size();
            HashGrid<int> hashgrid;
            hashgrid.init(numPoints, scale, bbox);

            RandomQueue<int> que;
            for (int i = 0; i < numCands; i++) {
                que.push(i);
            }

            std::vector<int> sampledIDs;
            Vector3 marginv(2.0 * minDist, 2.0 * minDist, 2.0 * minDist);
            while (!que.empty()) {
                int id = que.pop();
                Vector3 v = candPoints[id];
                std::vector<int>& cellvs = hashgrid[v];

                bool accept = true;
                for (int k = 0; k < cellvs.size(); k++) {
                    if ((candPoints[cellvs[k]] - v).squaredNorm() <= minDist * minDist) {
                        accept = false;
                        break;
                    }
                }

                if (accept) {
                    Vector3 boxMin = v - marginv;
                    Vector3 boxMax = v + marginv;
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
