#define SPICA_DOF_CAMERA_EXPORT
#include "dof_camera.h"
#include "../core/common.h"

namespace spica {

    // ------------------------------------------------------------
    // DoF camera
    // ------------------------------------------------------------

    DoFCamera::DoFCamera()
        : ICamera()
        , distSensorToLens_(0.0)
        , sensor_()
        , lens_()
        , objplane_() {
    }

    DoFCamera::DoFCamera(int imageW,
                         int imageH,
                         const Vector3D& sensorCenter,
                         const Vector3D& sensorDir,
                         const Vector3D& sensorUp,
                         double sensorSize,
                         double distSensorToLens,
                         double focalLength,
                         double lensRadius,
                         double sensorSensitivity)
        : ICamera(sensorCenter, sensorDir, sensorUp, imageW, imageH, sensorSensitivity)
        , distSensorToLens_(distSensorToLens)
        , sensor_()
        , lens_()
        , objplane_() {
        sensor_.center = sensorCenter;
        sensor_.direction = sensorDir;
        sensor_.up = sensorUp;

        sensor_.width  = sensorSize * imageW / imageH;
        sensor_.height = sensorSize;
        
        sensor_.cellW = sensor_.width  / imageW;
        sensor_.cellH = sensor_.height / imageH;

        sensor_.unitU = sensorDir.cross(sensorUp).normalized();
        sensor_.unitV = Vector3D::cross(sensor_.unitU, sensorDir).normalized();

        Vector3D objplaneCenter = sensorCenter + (focalLength + distSensorToLens) * sensorDir;
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

    DoFCamera::DoFCamera(const DoFCamera& camera)
        : ICamera(camera)
        , distSensorToLens_(camera.distSensorToLens_)
        , sensor_(camera.sensor_)
        , lens_(camera.lens_)
        , objplane_(camera.objplane_) {
    }

    DoFCamera::~DoFCamera() {
    }

    DoFCamera& DoFCamera::operator=(const DoFCamera& camera) {
        ICamera::operator=(camera);
        this->distSensorToLens_ = camera.distSensorToLens_;
        this->sensor_           = camera.sensor_;
        this->lens_             = camera.lens_;
        this->objplane_         = camera.objplane_;
        return *this;
    }

    double DoFCamera::PImageToPAx1(const double PImage, const Vector3D& x0xV, const Vector3D& x0x1, const Vector3D& orientingNormal) const {
        double ratio = distSensorToLens_ / lens_.focalLength;
        double lengthRatio = x0xV.dot(x0xV) / x0x1.dot(x0x1);
        double dirRatio = - Vector3D::dot(x0x1.normalized(), orientingNormal) / Vector3D::dot(x0x1.normalized(), sensor_.direction);
        return PImage * ratio * ratio * lengthRatio * dirRatio;
    }

    double plane_intersection(const Vector3D& normal, const Vector3D& pos, const Ray& ray) {
        const double pn = pos.dot(normal);
        const double on = ray.origin().dot(normal);
        const double dn = ray.direction().dot(normal);

        if (std::abs(dn) > EPS) {
            const double t = (pn - on) / dn;
            return t;
        }
        return -INFTY;
    }

    double DoFCamera::intersectLens(const Ray& ray, Vector3D& positionOnLens, Vector3D& positionOnObjplane, Vector3D& positionOnSensor, Vector3D& uvOnSensor) const {
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
                    uvOnSensor.xRef() = (uOnSensor + 0.5) * _imageW;
                    uvOnSensor.yRef() = (vOnSensor + 0.5) * _imageH;
                    return distToLens;
                }
            }
        }
        return -INFTY;
    }

    double DoFCamera::contribSensitivity(const Vector3D& x0xV, const Vector3D& x0xI, const Vector3D& x0x1) const {
        double r0 = x0xV.dot(x0xV) / x0xI.dot(x0xI);
        double a = distSensorToLens_ * Vector3D::dot(x0xI.normalized(), -sensor_.direction.normalized());
        double b = lens_.focalLength * Vector3D::dot(x0x1.normalized(),  sensor_.direction.normalized());

        double r1 = a / (b + EPS);
        return sensor_.sensitivity * r0 * r1 * r1;
    }

    void DoFCamera::samplePoints(const int imageX, const int imageY, Random& rng, Vector3D& positionOnSensor, Vector3D& positionOnObjplane, Vector3D& positionOnLens, double& PImage, double& PLens) const {
        const double uOnPixel = rng.nextReal();
        const double vOnPixel = rng.nextReal();

        const double uOnSensor = ((imageX + uOnPixel) / _imageW - 0.5);
        const double vOnSensor = ((imageY + vOnPixel) / _imageH - 0.5);
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

    CameraSample DoFCamera::sample(const double imageX, const double imageY, Stack<double>& rstk) const {
        const double uOnPixel = rstk.pop();
        const double vOnPixel = rstk.pop();

        const double uOnSensor = ((imageX + uOnPixel) / _imageW - 0.5);
        const double vOnSensor = ((imageY + vOnPixel) / _imageH - 0.5);
        const Vector3D posSensor = sensor_.center + (uOnSensor * sensor_.width) * sensor_.unitU + (vOnSensor * sensor_.height) * sensor_.unitV;

        const double ratio = lens_.focalLength / distSensorToLens_;
        const double uOnObjplane = -ratio * uOnSensor;
        const double vOnObjplane = -ratio * vOnSensor;
        const Vector3D posObjectPlane = objplane_.center + (uOnObjplane * objplane_.width) * objplane_.unitU + (vOnObjplane * objplane_.height) * objplane_.unitV;

        const double r0 = sqrt(rstk.pop());
        const double r1 = 2.0 * PI * rstk.pop();
        const double uOnLens = r0 * cos(r1);
        const double vOnLens = r0 * sin(r1);
        const Vector3D posLens = lens_.center + (uOnLens * lens_.radius) * lens_.unitU + (vOnLens * lens_.radius) * lens_.unitV;

        const double pdfImage = 1.0 / (sensor_.cellW * sensor_.cellH);
        const double pdfLens  = 1.0 / lens_.area();

        const Vector3D lensToSensor = posSensor - posLens;
        const double cosine = Vector3D::dot(_direction, lensToSensor.normalized());
        const double weight = cosine * cosine / lensToSensor.squaredNorm();
        const double samplePdf = pdfLens * pdfImage / weight;

        return CameraSample(Ray(posLens, (posObjectPlane - posLens).normalized()),
                            samplePdf, posSensor, posObjectPlane, posLens);
    }

    ICamera* DoFCamera::clone() const {
        return new DoFCamera(*this);
    }

}  // namespace spica
