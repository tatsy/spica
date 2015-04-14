#define RAINY_RENDERER_EXPORT
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "common.h"
#include "Vector3.h"
#include "material.h"
#include "Scene.h"

namespace rainy {

    const Color Renderer::backgroundColor = Color();
    const int   Renderer::maxDepth        = 5;
    const int   Renderer::depthLimit      = 64;

    Renderer::Renderer(int width, int height, int samples, int supsamples) 
        : _width(width)
        , _height(height)
        , _samplePerPixel(samples)
        , _supersamplePerAxis(supsamples)
    {
    }

    Renderer::Renderer(const Renderer& renderer)
        : _width(renderer._width)
        , _height(renderer._height)
        , _samplePerPixel(renderer._samplePerPixel)
        , _supersamplePerAxis(renderer._supersamplePerAxis)
    {
    }

    Renderer::~Renderer() 
    {
    }

    Renderer& Renderer::operator=(const Renderer& renderer) {
        this->_width = renderer._width;
        this->_height = renderer._height;
        this->_samplePerPixel = renderer._samplePerPixel;
        this->_supersamplePerAxis = renderer._supersamplePerAxis;
        return *this;
    }

    void Renderer::setSize(int width, int height) {
        this->_width = width;
        this->_height = height;
    }

    void Renderer::setSamples(int samples, int supsamples) {
        this->_samplePerPixel = samples;
        this->_supersamplePerAxis = supsamples;
    }

    int Renderer::render() {
        // Camera position
        const Vector3 cameraPos(50.0, 52.0, 220.0);
        const Vector3 cameraDir = Vector3(0.0, -0.04, -1.0).normalize();
        const Vector3 cameraUp(0.0, 1.0, 0.0);

        // Screen size in world space
        const double screenWidth = 30.0 * _width / _height;
        const double screenHeight = 30.0;

        // Distance to the screen
        const double distToScreen = 40.0;

        // Vectors spanning screen
        const Vector3 screenX = cameraDir.cross(cameraUp).normalize() * screenWidth;
        const Vector3 screenY = screenX.cross(cameraDir).normalize() * screenHeight;
        const Vector3 scrrenCenter = cameraPos + cameraDir * distToScreen;

        Color* image = new Color[_width * _height];

        Random rng = Random::getRNG();
        ompfor (int y = 0; y < _height; y++) {
            std::cout << "Row: " << y << " is processing..." << std::endl;

            for (int x = 0; x < _width; x++) {
                const int pixelIndex = (_height - y - 1) * _width + x;
                for (int sy = 0; sy < _supersamplePerAxis; sy++) {
                    for (int sx = 0; sx < _supersamplePerAxis; sx++) {
                        Color accum;
                        for (int s = 0; s < _samplePerPixel; s++) {
                            const double rate = (1.0 / _supersamplePerAxis);
                            const double rx = sx * rate + rate / 2.0;
                            const double ry = sy * rate + rate / 2.0;
                            const Vector3 screenPos = scrrenCenter + screenX * ((rx + x) / _width - 0.5) + screenY * ((ry + y) / _height - 0.5);
                            const Vector3 rayDirection = (screenPos - cameraPos).normalize();
                        
                            accum += radiance(Ray(cameraPos, rayDirection), rng, 0) / (_samplePerPixel * _supersamplePerAxis * _supersamplePerAxis);
                        }
                        image[pixelIndex] += accum;
                    }
                }
            }
        }

        savePPM("image.ppm", image, _width, _height);
        delete[] image;

        return 0;
    }

    Color Renderer::radiance(const Ray& ray, Random& rng, const int depth) {
        Intersection intersection;

        // NOT intersect the scene
        if (!intersectScene(ray, intersection)) {
            return backgroundColor;
        }

        const Sphere& currentObject = spheres[intersection.objectId()];
        const HitPoint& hitpoint = intersection.hitPoint();
        const Vector3 orientNormal = hitpoint.normal().dot(ray.direction()) < 0.0 ? hitpoint.normal() : (-1.0 * hitpoint.normal());

        double rouletteProb =std::max(currentObject.color().x(), std::max(currentObject.color().y(), currentObject.color().z()));

        if (depth > depthLimit) {
            rouletteProb *= pow(0.5, depth - depthLimit);
        }

        if (depth > maxDepth) {
            if (rng.randReal() > rouletteProb) {
                return currentObject.emission();
            }
        } else {
            rouletteProb = 1.0;
        }

        Color incomingRad;
        Color weight = Color(1.0, 0.0, 0.0);

        if (currentObject.reftype() == REFLECTION_DIFFUSE) {
            Vector3 u, v, w;
            w = orientNormal;
            if (abs(w.x()) > EPS) {
                u = Vector3(0.0, 1.0, 0.0).cross(w).normalize();
            } else {
                u = Vector3(1.0, 0.0, 0.0).cross(w).normalize();
            }

            v = w.cross(u);

            const double r1 = 2.0 * PI * rng.randReal();
            const double r2 = rng.randReal();
            const double r2s = sqrt(r2);
            Vector3 nextDir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)).normalize();

            incomingRad = radiance(Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = currentObject.color() / rouletteProb;

        } else if (currentObject.reftype() == REFLECTION_SPECULAR) {
            Vector3 nextDir = ray.direction() - (2.0 * hitpoint.normal().dot(ray.direction())) * hitpoint.normal();
            incomingRad = radiance(Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = currentObject.color() / rouletteProb;

        } else if (currentObject.reftype() == REFLECTION_REFRACTION) {
            Vector3 reflectDir = ray.direction() - (2.0 * hitpoint.normal().dot(ray.direction())) * hitpoint.normal();
            const Ray reflectRay = Ray(hitpoint.position(), reflectDir);

            // Incoming or outgoing
            const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

            // Snell's rule
            const double nc = 1.0;
            const double nt = indexOfRef;
            const double nnt = isIncoming ? nc / nt : nt / nc;
            const double ddn = ray.direction().dot(orientNormal);
            const double cos2t = 1.0 - nnt * nnt * (1.0  - ddn * ddn);

            if (cos2t < 0.0) { // Total reflection
                incomingRad = radiance(reflectRay, rng, depth + 1);
                weight += currentObject.color() / rouletteProb;
            } else {
                Vector3 refractDir = (ray.direction() * nnt - hitpoint.normal() * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalize();
                const Ray refractRay = Ray(hitpoint.position(), refractDir);

                // Schlick's approximation of Fresnel coefficient
                const double a = nt - nc;
                const double b = nt + nc;
                const double R0 = (a * a) / (b * b);

                const double c = 1.0 - (isIncoming ? -ddn : - refractDir.dot(orientNormal));
                const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
                const double nnt2 = pow(isIncoming ? nc / nt : nt / nc, 2.0);
                const double Tr = (1.0 - Re) * nnt2;

                const double prob = 0.25 + 0.5 * Re;
                if (depth > 2) {
                    if (rng.randReal() < prob) {
                        incomingRad = radiance(reflectRay, rng, depth + 1) * Re;
                        weight = currentObject.color() / (prob * rouletteProb);
                    } else {
                        incomingRad = radiance(refractRay, rng, depth + 1) * Tr;
                        weight = currentObject.color() / ((1.0 - prob) * rouletteProb);
                    }
                } else {
                    incomingRad = radiance(reflectRay, rng, depth + 1) * Re + radiance(refractRay, rng, depth + 1) * Tr;
                    weight = currentObject.color() / rouletteProb;
                }
            }
        }

        return currentObject.emission() + weight.cwiseMultiply(incomingRad);
    }

    void Renderer::savePPM(std::string filename, Color* image, int width, int height) {
        std::ofstream ofs(filename.c_str(), std::ios::out);
        ofs << "P3" << std::endl;
        ofs << width << " " << height << " 255" << std::endl;

        for (int i = 0; i < width * height; i++) {
            int r = std::max(0, std::min((int)image[i].x(), 255));
            int g = std::max(0, std::min((int)image[i].y(), 255));
            int b = std::max(0, std::min((int)image[i].z(), 255));
            ofs << r << " " << g << " " << b << std::endl;
        }
        ofs.close();
    }
}
