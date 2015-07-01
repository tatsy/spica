#define SPICA_MATERIAL_EXPORT
#include "material.h"

namespace spica {

    Material::Material()
        : emission()
        , color()
        , reftype(REFLECTION_DIFFUSE)
        , brdf()
    {
    }

    Material::Material(const Color& emission_, const Color& color_, const ReflectionType reftype_, const BRDF& brdf_)
        : emission(emission_)
        , color(color_)
        , reftype(reftype_)
        , brdf(brdf_)
    {
    }

    Material::Material(const Material& mtrl)
        : emission()
        , color()
        , reftype()
        , brdf()
    {
        this->operator=(mtrl);
    }

    Material::~Material()
    {
        // TODO: intentional memory leak. Very very very dangerous
        // delete brdf;
    }

    Material& Material::operator=(const Material& mtrl) {
        this->emission = mtrl.emission;
        this->color = mtrl.color;
        this->reftype = mtrl.reftype;
        this->brdf = mtrl.brdf;
        return *this;
    }

}  // namespace spica
