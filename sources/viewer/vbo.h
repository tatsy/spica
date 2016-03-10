#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VBO_H_
#define _SPICA_VBO_H_

#include <vector>
#include <type_traits>

#include "../shape/spica_shape.h"

extern void* enabler;

namespace spica {

/** Vertex buffer object.
 */
class SPICA_EXPORTS VBO {
public:
    // Public methods

    /** The VBO constructor. */
    VBO();
    /** The VBO constructor (copy). */
    VBO(const VBO& vbo);
    /** The VBO destructor. */
    ~VBO();

    /** Assignment operator. */
    VBO& operator=(const VBO& vbo);

    void add(const Triangle& t, const RGBSpectrum& color);

    inline int numIndices() const { return (int)indices_.size(); }

    inline const float* vertices() const { return &vertices_[0]; }
    inline const float* normals() const { return &normals_[0]; }
    inline const float* colors() const { return &colors_[0]; }
    inline const unsigned int* indices() const { return &indices_[0]; }

private:
    // Private methods
    void add(const Point3d& v, const Normal3d& normal, const RGBSpectrum& color);

    // Private fields
    std::vector<float> vertices_;
    std::vector<float> normals_;
    std::vector<float> colors_;
    std::vector<unsigned int> indices_;

};

}  // namespace spica

#endif  // _SPICA_VBO_H_
