#include <mitsuba/render/texture.h>
#include <mitsuba/render/interaction.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/core/transform.h>

NAMESPACE_BEGIN(mitsuba)

/**!

.. _texture-checkerboard:

Checkerboard texture (:monosp:`checkerboard`)
---------------------------------------------

.. pluginparameters::

 * - color0, color1
   - |spectrum| or |texture|
   - Color values for the two differently-colored patches (Default: 0.4 and 0.2)
 * - to_uv
   - |transform|
   - Specifies an optional 3x3 UV transformation matrix. A 4x4 matrix can also be provided.
     In that case, the last row and columns will be ignored.  (Default: none)

This plugin provides a simple procedural checkerboard texture with customizable colors.

.. subfigstart::
.. subfigure:: ../../resources/data/docs/images/render/texture_checkerboard.jpg
   :caption: Checkerboard applied to the material test object as well as the ground plane.
.. subfigend::
    :label: fig-texture-checkerboard

 */

template <typename Float, typename Spectrum>
class Fresnel final : public Texture<Float, Spectrum> {
public:
    MTS_IMPORT_TYPES(Texture)

    Fresnel(const Properties &props) : Texture(props) {
        m_color0 = props.texture<Texture>("color0", .0f);
        m_color1 = props.texture<Texture>("color1", 1.0f);
        m_ior    = props.float_("ior", 1.5f);
        //m_transform = props.transform("to_uv", ScalarTransform4f()).extract();
    }

    UnpolarizedSpectrum eval(const SurfaceInteraction3f &it, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);

        UnpolarizedSpectrum result = zero<UnpolarizedSpectrum>();
        UnpolarizedSpectrum R1    = m_color0->eval(it, active);
        UnpolarizedSpectrum R2    = m_color1->eval(it, active);
        UnpolarizedSpectrum value  = CustomSheenF(R1, R2, it.wi, m_ior);

        return value;
    }

    Float eval_1(const SurfaceInteraction3f &it, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);

        //Point2f uv = m_transform.transform_affine(it.uv);
        //mask_t<Point2f> mask = (uv - floor(uv)) > .5f;
        Float result = 0.f;

        //Mask m0 = neq(mask.x(), mask.y()),
        //     m1 = !m0;

        //m0 &= active; m1 &= active;

        //if (any_or<true>(m0))
        //    masked(result, m0) = m_color0->eval_1(it, m0);

        //if (any_or<true>(m1))
        //    masked(result, m1) = m_color1->eval_1(it, m1);

        return result;
    }

    ScalarFloat mean() const override {
        return .5f * (m_color0->mean() + m_color1->mean());
    }

    UnpolarizedSpectrum CustomSheenF(UnpolarizedSpectrum R1,
                                     UnpolarizedSpectrum R2, Vector3f wi,
                                     Float eta) const {
        // Evaluate Fresnel factor _F_ for conductor BRDF
        Float frCosTheta_i = Frame3f::cos_theta(wi);
        Float amt          = fresnel_dielectric_cos(frCosTheta_i, eta);
        // mix
        return (1.0f - amt) * R1 + amt * R2;
    }

    // from blender source code
    Float fresnel_dielectric_cos(Float cosi, Float eta) const {
        /* compute fresnel reflectance without explicitly computing
         * the refracted direction */
        Float c = abs(cosi);
        Float g = eta * eta - 1.0f + c * c;
        Float result;
        
        mask_t<Float> g_m = g > 0.0f;
        
        if (any_or<true>(g_m)) {
            g       = sqrt(g);
            Float A       = (g - c) / (g + c);
            Float B = (c * (g + c) - 1.0f) / (c * (g - c) + 1.0f);
            result  = 0.5f * A * A * (1.0f + B * B);
        } else
            result = 1.0f; /* TIR (no refracted component) */

        return result;
    }

    void traverse(TraversalCallback *callback) override {
        callback->put_parameter("ior", m_ior);
        callback->put_object("color0", m_color0.get());
        callback->put_object("color1", m_color1.get());
    }

    bool is_spatially_varying() const override { return true; }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "Fresnel[" << std::endl
            << "  color0 = " << string::indent(m_color0) << std::endl
            << "  color1 = " << string::indent(m_color1) << std::endl
            << "  ior = " << string::indent(m_ior) << std::endl
            //<< "  transform = " << string::indent(m_transform) << std::endl
            << "]";
        return oss.str();
    }

    MTS_DECLARE_CLASS()
protected:
    ref<Texture> m_color0;
    ref<Texture> m_color1;
    //ScalarTransform3f m_transform;
    ScalarFloat m_ior;
};

MTS_IMPLEMENT_CLASS_VARIANT(Fresnel, Texture)
MTS_EXPORT_PLUGIN(Fresnel, "Fresnel mix texture")
NAMESPACE_END(mitsuba)
