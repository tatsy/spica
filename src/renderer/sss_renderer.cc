#define SPICA_SSS_RENDERER_EXPORT
#include "sss_renderer.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    namespace {
        const double sigma_a = 1.0e-4;
        const double sigmap_s = 0.5;
        const double eta = 1.3;
        const double maxError = 0.05;
    }

    SSSRenderer::Octree::Octree()
        : _root(NULL)
        , _numCopies(NULL)
    {
    }

    SSSRenderer::Octree::~Octree()
    {
        release();
    }

    SSSRenderer::Octree::Octree(const SSSRenderer::Octree& octree)
        : _root(NULL)
        , _numCopies(NULL)
    {
        this->operator=(octree);
    }

    SSSRenderer::Octree& SSSRenderer::Octree::operator=(const Octree& octree) {
        release();

        _numCopies = octree._numCopies;
        (*_numCopies) += 1;
        _root = octree._root;

        return *this;
    }

    void SSSRenderer::Octree::release() {
        if (_numCopies != NULL) {
            if ((*_numCopies) == 0) {
                deleteNode(_root);
                delete _numCopies;
                _numCopies = NULL;
            }
            else {
                (*_numCopies) -= 1;
            }
        }
    }

    void SSSRenderer::Octree::deleteNode(SSSRenderer::OctreeNode* node) {
        if (node != NULL) {
            for (int i = 0; i < 8; i++) {
                deleteNode(node->children[i]);
            }
            delete node;
        }
    }

    void SSSRenderer::Octree::construct(std::vector<SSSRenderer::IrradiancePoint>& iradPoints) {
        BBox bbox;
        for (int i = 0; i < iradPoints.size(); i++) {
            bbox.merge(iradPoints[i].pos);
        }

        _root = constructRec(iradPoints, bbox);
    }

    SSSRenderer::OctreeNode* SSSRenderer::Octree::constructRec(std::vector<IrradiancePoint>& iradPoints, const BBox& bbox) {
        if (iradPoints.empty()) {
            return NULL;
        } else if (iradPoints.size() == 1) {
            OctreeNode* node = new OctreeNode();
            node->pt = iradPoints[0];
            node->bbox = bbox;
            node->isLeaf = true;
            return node;
        }

        Vector3 posMid = (bbox.posMin() + bbox.posMax()) * 0.5;

        const int numPoints = static_cast<int>(iradPoints.size());
        std::vector<std::vector<IrradiancePoint> > childPoints(8);
        for (int i = 0; i < numPoints; i++) {
            const Vector3& v = iradPoints[i].pos;
            int id = (v.x() < posMid.x() ? 0 : 4) + (v.y() < posMid.y() ? 0 : 2) + (v.z() < posMid.z() ? 0 : 1);
            childPoints[id].push_back(iradPoints[i]);
        }

        // Compute child nodes
        OctreeNode* node = new OctreeNode();
        for (int i = 0; i < 8; i++) {
            BBox childBox;
            for (int j = 0; j < childPoints[i].size(); j++) {
                childBox.merge(childPoints[i][j].pos);
            }
            node->children[i] = constructRec(childPoints[i], childBox);
        }
        node->bbox = bbox;
        node->isLeaf = false;

        // Accumulate child nodes
        node->pt.pos = Vector3(0.0, 0.0, 0.0);
        node->pt.normal = Vector3(0.0, 0.0, 0.0);
        node->pt.area = 0.0;
        node->pt.irad = Color(0.0, 0.0, 0.0);       

        double weight = 0.0;
        int childCount = 0;
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                double w = node->children[i]->pt.irad.luminance();
                node->pt.pos += w * node->children[i]->pt.pos;
                node->pt.normal += w * node->children[i]->pt.normal;
                node->pt.area += node->children[i]->pt.area;
                node->pt.irad += node->children[i]->pt.irad;
                weight += node->children[i]->pt.irad.luminance();
                childCount += 1;
            }
        }

        if (weight > 0.0) {
            node->pt.pos /= weight;
            node->pt.normal /= weight;
        }

        if (childCount != 0) {
            node->pt.irad /= childCount;
        }

        return node;
    }

    Color SSSRenderer::Octree::iradSubsurface(const Vector3& pos, const DiffusionReflectance& Rd) {
        return iradSubsurfaceRec(_root, pos, Rd);
    }

    Color SSSRenderer::Octree::iradSubsurfaceRec(OctreeNode* node, const Vector3& pos, const DiffusionReflectance& Rd) {
        if (node == NULL) return Color(0.0, 0.0, 0.0);

        const double distSquared = (node->pt.pos - pos).squaredNorm();
        double dw = node->pt.area / distSquared;
        if (node->isLeaf || (dw < maxError && !node->bbox.inside(pos))) {
            return Rd(distSquared) * node->pt.irad * node->pt.area;
        } else {
            Color ret(0.0, 0.0, 0.0);
            for (int i = 0; i < 8; i++) {
                if (node->children[i] != NULL) {
                    ret += iradSubsurfaceRec(node->children[i], pos, Rd);
                }
            }
            return ret;
        }
    }

    SSSRenderer::SSSRenderer()
    {
    }

    SSSRenderer::~SSSRenderer()
    {
    }

    void SSSRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel, const int samplePerPoint) {
        const double minDist = 0.2;
        const double maxDist = 0.7;

        // Poisson disk sampling on SSS objects
        std::vector<Vector3> points;
        std::vector<Vector3> normals;
        for (int i = 0; i < scene.numObjects(); i++) {
            if (scene.getMaterial(i).reftype == REFLECTION_SUBSURFACE) {
                msg_assert(points.empty(), "# of objects with subsurface scattering property must be only one !!");

                const Primitive* obj = scene.get(i);
                std::string typname = typeid(*obj).name();
                msg_assert(typname == "class spica::Trimesh", "Object with subsurface scattering property must be Trimesh !!");

                const Trimesh* trimesh = reinterpret_cast<const Trimesh*>(obj);
                sampler::poissonDisk(*trimesh, minDist, maxDist, &points, &normals);
            }
        }

        // Compute irradiance on each sampled point
        const int numPoints = static_cast<int>(points.size());
        std::vector<Color> irads(numPoints, Color(0.0, 0.0, 0.0));

        std::cout << "Computing irradiance for sample points ..." << std::endl;
        int proc = 0;
        ompfor(int i = 0; i < numPoints; i++) {
            // Sample random direction
            for (int k = 0; k < samplePerPoint; k++) {
                Vector3 dir;
                sampler::onHemisphere(normals[i], &dir);
                Ray ray(points[i], dir);

                // Here, since irradiance is stored, 2.0 * pi is multiplied to radiance value
                irads[i] += helper::radiance(scene, ray, rng, 0) * (2.0 * PI / samplePerPoint);
            }

            omplock{
                proc++;
                printf("%6.2f %% processed ...\n", 100.0 * proc / numPoints);
            }
        }

        // Save radiance data for visual checking
        std::ofstream ofs("sample_rads.obj", std::ios::out);
        for (int i = 0; i < numPoints; i++) {
            Vector3 p = points[i];
            Vector3 clr = Vector3::minimum(irads[i], Vector3(1.0, 1.0, 1.0));
            ofs << "v " <<  p.x() << " " << p.y() << " " << p.z();
            ofs << " " << clr.x() << " " << clr.y() << " " << clr.z() << std::endl;
        }
        ofs.close();

        // Octree construction
        std::vector<IrradiancePoint> iradPoints(numPoints);
        for (int i = 0; i < numPoints; i++) {
            iradPoints[i].pos = points[i];
            iradPoints[i].normal = normals[i];
            iradPoints[i].area = PI * (0.5 * minDist) * (0.5 * minDist);
            iradPoints[i].irad = irads[i];
        }
        octree.construct(iradPoints);
        std::cout << "Octree constructed !!" << std::endl;

        // Main rendering process
        const int width = camera.imageW();
        const int height = camera.imageH();
        Image buffer(width, height);

        proc = 0;
        for(int i = 0; i < samplePerPixel; i++) {
            ompfor (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    buffer.pixel(width - x - 1, y) += executePathTracing(scene, camera, rng, x, y);
                }

                omplock{
                    proc++;
                    printf("%6.2f processed ...\n", 100.0 * proc / (height * samplePerPixel));
                }
            }

            char filename[256];
            Image image(width, height);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    image.pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }

            sprintf(filename, "subsurface_%03d.bmp", i + 1);
            image.saveBMP(filename);
        }
    }

    Color SSSRenderer::executePathTracing(const Scene& scene, const Camera& camera, const Random& rng, const int imageX, const int imageY) {
        Vector3 posOnSensor, posOnObjplane, posOnLens;
        double pImage, pLens;
        camera.samplePoints(imageX, imageY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);

        Ray ray(posOnLens, Vector3::normalize(posOnObjplane - posOnLens));

        Vector3 lens2sensor = posOnSensor - posOnLens;
        const double cosine = Vector3::dot(camera.direction(), lens2sensor.normalized());
        const double weight = cosine * cosine / lens2sensor.squaredNorm();

        return radiance(scene, ray, rng, 0) * (weight * camera.sensitivity() / (pImage * pLens));
    }

    Color SSSRenderer::radiance(const Scene& scene, const Ray& ray, const Random& rng, const int depth, const int depthLimit, const int depthMin) {
        Intersection isect;

        // NOT intersect the scene
        if (!scene.intersect(ray, isect)) {
            return scene.bgColor();
        }

        const Material& mtrl = scene.getMaterial(isect.objectId());
        const Hitpoint& hitpoint = isect.hitpoint();
        const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

        double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

        if (depth > depthLimit) {
            roulette *= pow(0.5, depth - depthLimit);
        }

        if (depth > depthMin) {
            if (roulette < rng.nextReal()) {
                return mtrl.emission;
            }
        } else {
            roulette = 1.0;
        }

        Color incomingRad;
        Color weight = Color(1.0, 1.0, 1.0);

        if (mtrl.reftype == REFLECTION_DIFFUSE) {
            Vector3 nextDir;
            sampler::onHemisphere(orientNormal, &nextDir);
            incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = mtrl.color / roulette;
        } else if (mtrl.reftype == REFLECTION_SPECULAR) {
            Vector3 nextDir = ray.direction() - (2.0 * hitpoint.normal().dot(ray.direction())) * hitpoint.normal();
            incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = mtrl.color / roulette;
        } else if (mtrl.reftype == REFLECTION_REFRACTION) {
            const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

            Vector3 reflectDir, transmitDir;
            double fresnelRe, fresnelTr;
            bool isTotRef = helper::isTotalRef(isIncoming,
                hitpoint.position(),
                ray.direction(),
                hitpoint.normal(),
                orientNormal,
                &reflectDir,
                &transmitDir,
                &fresnelRe,
                &fresnelTr);

            Ray reflectRay(hitpoint.position(), reflectDir);

            if (isTotRef) {
                // Total reflection
                incomingRad = radiance(scene, reflectRay, rng, depth + 1);
                weight = mtrl.color / roulette;
            } else {
                Ray transmitRay(hitpoint.position(), transmitDir);

                const double prob = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                if (depth > 2) {
                    if (rng.nextReal() < prob) {
                        // Reflect
                        incomingRad = radiance(scene, reflectRay, rng, depth + 1) * fresnelRe;
                        weight = mtrl.color / (prob * roulette);
                    } else {
                        // Transmit
                        incomingRad = radiance(scene, transmitRay, rng, depth + 1) * fresnelTr;
                        weight = mtrl.color / ((1.0 - prob) * roulette);
                    }
                } else {
                    // Both reflect and transmit
                    incomingRad = radiance(scene, reflectRay, rng, depth + 1) * fresnelRe + radiance(scene, transmitRay, rng, depth + 1) * fresnelTr;
                    weight = mtrl.color / roulette;
                }
            }
        } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
            const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

            Vector3 reflectDir, transmitDir;
            double fresnelRe, fresnelTr;
            bool isTotRef = helper::isTotalRef(isIncoming,
                hitpoint.position(),
                ray.direction(),
                hitpoint.normal(),
                orientNormal,
                &reflectDir,
                &transmitDir,
                &fresnelRe,
                &fresnelTr);

            Ray reflectRay(hitpoint.position(), reflectDir);

            if (isTotRef) {
                // Total reflection
                incomingRad = radiance(scene, reflectRay, rng, depth + 1);
                weight = mtrl.color / roulette;
            } else {
                Ray transmitRay(hitpoint.position(), transmitDir);

                const double prob = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                if (depth > 2) {
                    if (rng.nextReal() < prob) {
                        // Reflect
                        incomingRad = radiance(scene, reflectRay, rng, depth + 1) * fresnelRe;
                        weight = mtrl.color / (prob * roulette);
                    } else {
                        // Transmit
                        DiffusionReflectance Rd(sigma_a, sigmap_s, eta);
                        Color Mo = octree.iradSubsurface(hitpoint.position(), Rd);
                        incomingRad = (1.0 / PI) * (1.0 - Rd.Fdr(eta)) * Mo * fresnelTr;
                        weight = mtrl.color / ((1.0 - prob) * roulette);
                    }
                } else {
                    // Both reflect and transmit
                    DiffusionReflectance Rd(sigma_a, sigmap_s, eta);
                    Color Mo = octree.iradSubsurface(hitpoint.position(), Rd);
                    // Color transmitRad = (1.0 / PI) * (1.0 - Rd.Fdr(eta)) * Rd.Ft(hitpoint.normal(), ray.direction()) * Mo;
                    Color transmitRad = (1.0 / PI) * (1.0 - Rd.Fdr(eta)) * Mo;

                    Color reflectRad = radiance(scene, reflectRay, rng, depth + 1);
                    incomingRad = reflectRad * fresnelRe + transmitRad * fresnelTr;
                    weight = mtrl.color / roulette;
                }
            }
        }
        return mtrl.emission + weight.cwiseMultiply(incomingRad);
    }

}  // namespace spica

