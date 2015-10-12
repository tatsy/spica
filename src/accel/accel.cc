#define SPICA_ACCEL_EXPORT
#include "accel.h"

#include "bbvh_accel.h"
#include "qbvh_accel.h"
#include "kd_tree_accel.h"

namespace spica {

    Accelerator::Accelerator()
        : _ptr{}
        , _type{AccelType::QBVH} {
    }

    Accelerator::Accelerator(AccelType type)
        : _ptr{}
        , _type{type} {
    }

    Accelerator::Accelerator(const Accelerator& accel) 
        : Accelerator{} {
        this->operator=(accel);
    }

    Accelerator::~Accelerator() {
    }

    Accelerator& Accelerator::operator=(const Accelerator& accel) {
        this->_ptr  = accel._ptr;
        this->_type = accel._type;
        return *this;
    }

    void Accelerator::setAccelType(AccelType type) {
        this->_type = type;
    }

    void Accelerator::construct(const std::vector<Triangle>& triangles) {
        switch (_type) {
        case AccelType::BBVH:
            _ptr = std::make_shared<BBVHAccel>();
            break;

        case AccelType::QBVH:
            _ptr = std::make_shared<QBVHAccel>();
            break;

        case AccelType::KdTree:
            _ptr = std::make_shared<KdTreeAccel>();
            break;

        default:
            std::cout << "[ERROR] unknown accelerator type specified!!" << std::endl;
            std::abort();
        }
        _ptr->construct(triangles);
    }

    int Accelerator::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        return _ptr->intersect(ray, hitpoint);
    }

    Accelerator::operator bool() const {
        return static_cast<bool>(_ptr);
    }

}  // namespace spica
