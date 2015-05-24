#define SPICA_CAMERA_EXPORT
#include "camera.h"
#include "../utils/common.h"

namespace spica {

    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    Camera::Camera()
        : width_(0)
        , height_(0)
        , distSensorToLens_(0.0)
        , sensor_()
        , lens_()
        , objplane_()
    {
    }

    Camera::Camera(int imageW,
                   int imageH,
                   const Vector3& sensorCenter,
                   const Vector3& sensorDir,
                   const Vector3& sensorUp,
                   double sensorSize,
                   double distSensorToLens,
                   double focalLength,
                   double lensRadius,
                   double sensorSensitivity)
        : width_(imageW)
        , height_(imageH)
        , distSensorToLens_(distSensorToLens)
        , sensor_()
        , lens_()
        , objplane_()
    {
        sensor_.center = sensorCenter;
        sensor_.direction = sensorDir;
        sensor_.up = sensorUp;

        sensor_.width  = sensorSize * imageW / imageH;
        sensor_.height = sensorSize;
        
        sensor_.cellW = sensor_.width  / imageW;
        sensor_.cellH = sensor_.height / imageH;

        sensor_.unitU = sensorDir.cross(sensorUp).normalized();
        sensor_.unitV = Vector3::cross(sensor_.unitU, sensorDir).normalized();

        Vector3 objplaneCenter = sensorCenter + (focalLength + distSensorToLens) * sensorDir;
        objplane_.width = sensor_.width;
        objplane_.height = sensor_.height;
        objplane_.center = objplaneCenter;
        objplane_.normal = sensorDir.normalized();
        objplane_.unitU = sensor_.unitU;
        objplane_.unitV = sensor_.unitV;
        
        lens_.radius = lensRadius;
        lens_.focalLength = focalLength;
        lens_.center = sensorCenter + distSensorToLens * sensorDir;
        lens_.unitU = sensor_.unitU;
        lens_.unitV = sensor_.unitV;
        lens_.normal = sensorDir.normalized();

        sensor_.sensitivity = sensorSensitivity / (sensor_.cellW * sensor_.cellH);
    }

    Camera::Camera(const Camera& camera)
        : width_(camera.width_)
        , height_(camera.height_)
        , distSensorToLens_(camera.distSensorToLens_)
        , sensor_(camera.sensor_)
        , lens_(camera.lens_)
        , objplane_(camera.objplane_)
    {
    }

    Camera::~Camera()
    {
    }

    Camera& Camera::operator=(const Camera& camera) {
        this->width_            = camera.width_;
        this->height_           = camera.height_;
        this->distSensorToLens_ = camera.distSensorToLens_;
        this->sensor_           = camera.sensor_;
        this->lens_             = camera.lens_;
        this->objplane_         = camera.objplane_;
        return *this;
    }

    double Camera::PImageToPAx1(const double PImage, const Vector3& x0xV, const Vector3& x0x1, const Vector3& orientingNormal) const {
        double ratio = distSensorToLens_ / lens_.focalLength;
        double lengthRatio = x0xV.dot(x0xV) / x0x1.dot(x0x1);
        double dirRatio = - Vector3::dot(x0x1.normalized(), orientingNormal) / Vector3::dot(x0x1.normalized(), sensor_.direction);
        return PImage * ratio * ratio * lengthRatio * dirRatio;
    }

    double plane_intersection(const Vector3& normal, const Vector3& pos, const Ray& ray) {
        const double pn = pos.dot(normal);
        const double on = ray.origin().dot(normal);
        const double dn = ray.direction().dot(normal);

        if (abs(dn) > EPS) {
            const double t = (pn - on) / dn;
            return t;
        }
        return -INFTY;
    }

    double Camera::intersectLens(const Ray& ray, Vector3& positionOnLens, Vector3& positionOnObjplane, Vector3& positionOnSensor, Vector3& uvOnSensor) const {
        const double distToLens = plane_intersection(lens_.normal, lens_.center, ray);
        if (EPS < distToLens) {
            positionOnLens = ray.origin() + distToLens * ray.direction();
            if ((positionOnLens - lens_.center).norm() < lens_.radius && lens_.normal.dot(ray.direction()) <= 0.0) {
                const double objplaneT = plane_intersection(objplane_.normal, objplane_.center, ray);
                positionOnObjplane = ray.origin() + objplaneT * ray.direction();
                const double uOnObjplane = (positionOnObjplane - objplane_.center).dot(objplane_.unitU) / objplane_.width;
                const double vOnObjplane = (positionOnObjplane - objplane_.center).dot(objplane_.unitV) / objplane_.height;

                const double ratio = distSensorToLens_ / lens_.focalLength;
                const double uOnSensor = -ratio * uOnObjplane;
                const double vOnSensor = -ratio * vOnObjplane;
                positionOnSensor = sensor_.center + (uOnSensor * sensor_.width) * sensor_.unitU + (vOnSensor * sensor_.height) * sensor_.unitV;

                if (-0.5 <= uOnSensor && uOnSensor < 0.5 && -0.5 <= vOnSensor && vOnSensor < 0.5) {
                    uvOnSensor.setX((uOnSensor + 0.5) * width_);
                    uvOnSensor.setY((vOnSensor + 0.5) * height_);
                    return distToLens;
                }
            }
        }
        return -INFTY;
    }

    double Camera::contribSensitivity(const Vector3& x0xV, const Vector3& x0xI, const Vector3& x0x1) const {
        double r0 = x0xV.dot(x0xV) / x0xI.dot(x0xI);
        double a = distSensorToLens_ * Vector3::dot(x0xI.normalized(), -sensor_.direction.normalized());
        double b = lens_.focalLength * Vector3::dot(x0x1.normalized(),  sensor_.direction.normalized());

        double r1 = a / (b + EPS);
        return sensor_.sensitivity * r0 * r1 * r1;
    }

    void Camera::samplePoints(const int imageX, const int imageY, const Random& rng, Vector3& positionOnSensor, Vector3& positionOnObjplane, Vector3& positionOnLens, double& PImage, double& PLens) const {
        const double uOnPixel = rng.nextReal();
        const double vOnPixel = rng.nextReal();

        const double uOnSensor = ((imageX + uOnPixel) / this->width_  - 0.5);
        const double vOnSensor = ((imageY + vOnPixel) / this->height_ - 0.5);
        positionOnSensor = sensor_.center + (uOnSensor * sensor_.width) * sensor_.unitU + (vOnSensor * sensor_.height) * sensor_.unitV;

        const double ratio = lens_.focalLength / distSensorToLens_;
        const double uOnObjplane = -ratio * uOnSensor;
        const double vOnObjplane = -ratio * vOnSensor;
        positionOnObjplane = objplane_.center + (uOnObjplane * objplane_.width) * objplane_.unitU + (vOnObjplane * objplane_.height) * objplane_.unitV;

        const double r0 = sqrt(rng.nextReal());
        const double r1 = rng.nextReal() * (2.0 * PI);
        const double uOnLens = r0 * cos(r1);
        const double vOnLens = r0 * sin(r1);
        positionOnLens = lens_.center + (uOnLens * lens_.radius) * lens_.unitU + (vOnLens * lens_.radius) * lens_.unitV;

        PImage = 1.0 / (sensor_.cellW * sensor_.cellH);
        PLens  = 1.0 / lens_.area();
    }

    CameraSample Camera::sample(const double imageX, const double imageY, RandomSeq& rseq) const {
        const double uOnPixel = rseq.next();
        const double vOnPixel = rseq.next();

        CameraSample sample;
        sample.camera = this;

        const double uOnSensor = ((imageX + uOnPixel) / this->width_  - 0.5);
        const double vOnSensor = ((imageY + vOnPixel) / this->height_ - 0.5);
        sample.posSensor = sensor_.center + (uOnSensor * sensor_.width) * sensor_.unitU + (vOnSensor * sensor_.height) * sensor_.unitV;

        const double ratio = lens_.focalLength / distSensorToLens_;
        const double uOnObjplane = -ratio * uOnSensor;
        const double vOnObjplane = -ratio * vOnSensor;
        sample.posObjectPlane = objplane_.center + (uOnObjplane * objplane_.width) * objplane_.unitU + (vOnObjplane * objplane_.height) * objplane_.unitV;

        const double r0 = sqrt(rseq.next());
        const double r1 = rseq.next() * (2.0 * PI);
        const double uOnLens = r0 * cos(r1);
        const double vOnLens = r0 * sin(r1);
        sample.posLens = lens_.center + (uOnLens * lens_.radius) * lens_.unitU + (vOnLens * lens_.radius) * lens_.unitV;

        sample.pdfImage = 1.0 / (sensor_.cellW * sensor_.cellH);
        sample.pdfLens  = 1.0 / lens_.area();

        return sample;
    }

}

