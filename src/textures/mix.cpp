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
class MixTexture final : public Texture<Float, Spectrum> {
public:
    MTS_IMPORT_TYPES(Texture)

    MixTexture(const Properties &props) : Texture(props) {
        m_color0 = props.texture<Texture>("color0", 0.f);
        m_color1 = props.texture<Texture>("color1", 1.f);
        m_weight = props.texture<Texture>("weight", .5f);
    }

    UnpolarizedSpectrum eval(const SurfaceInteraction3f &it, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);
        UnpolarizedSpectrum result = zero<UnpolarizedSpectrum>();
        Float weight               = eval_weight(it, active);
        result                     = m_color0->eval(it, active) * (1.f - weight) +
                 m_color1->eval(it, active) * weight;
        return result;
    }

    MTS_INLINE Float eval_weight(const SurfaceInteraction3f &si,
                                 const Mask &active) const {
        return clamp(m_weight->eval_1(si, active), 0.f, 1.f);
    }

    Float eval_1(const SurfaceInteraction3f &it, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);
        Float result = 0;
        Float weight               = eval_weight(it, active);
        result       = m_color0->eval_1(it, active) * (1.f - weight) +
                 m_color1->eval_1(it, active) * weight;
        return result;
    }

    ScalarFloat mean() const override {
        return .5f * (m_color0->mean() + m_color1->mean());
    }

    void traverse(TraversalCallback *callback) override {
        callback->put_parameter("weight", m_weight);
        callback->put_object("color0", m_color0.get());
        callback->put_object("color1", m_color1.get());
    }

    bool is_spatially_varying() const override { return true; }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "MixTexture[" << std::endl
            << "  color0 = " << string::indent(m_color0) << std::endl
            << "  color1 = " << string::indent(m_color1) << std::endl
            << "  weight = " << string::indent(m_weight) << std::endl
            << "]";
        return oss.str();
    }

    MTS_DECLARE_CLASS()
protected:
    ref<Texture> m_color0;
    ref<Texture> m_color1;
    ref<Texture> m_weight;
};

MTS_IMPLEMENT_CLASS_VARIANT(MixTexture, Texture)
MTS_EXPORT_PLUGIN(MixTexture, "Mix texture")
NAMESPACE_END(mitsuba)
