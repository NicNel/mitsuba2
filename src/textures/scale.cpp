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
class ScalingTexture final : public Texture<Float, Spectrum> {
public:
    MTS_IMPORT_TYPES(Texture)

    ScalingTexture(const Properties &props) : Texture(props) {
        m_nested = props.texture<Texture>("texture", 0.f);
        m_scale  = props.float_("scale", 1.f);
    }

    UnpolarizedSpectrum eval(const SurfaceInteraction3f &it, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);
        return m_nested->eval(it, active) * m_scale;
    }

    Float eval_1(const SurfaceInteraction3f &it, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);

        return m_nested->eval_1(it, active) * m_scale;
    }

    Vector2f eval_1_grad(const SurfaceInteraction3f &si,
                         Mask active = true) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);

            return m_nested->eval_1_grad(si, active) * m_scale;
        }

    Color3f eval_3(const SurfaceInteraction3f &si,
                       Mask active = true) const override {
            MTS_MASKED_FUNCTION(ProfilerPhase::TextureEvaluate, active);

            return m_nested->eval_3(si, active) * m_scale;
        }
    
    ScalarFloat mean() const override { return m_nested->mean(); }

    void traverse(TraversalCallback *callback) override {
        callback->put_object("texture", m_nested.get());
        callback->put_parameter("scale", m_scale);
    }

    bool is_spatially_varying() const override { return true; }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "ScalingTexture[" << std::endl
            << "  texture = " << string::indent(m_nested) << std::endl
            << "  scale = " << string::indent(m_scale) << std::endl
            << "]";
        return oss.str();
    }

    MTS_DECLARE_CLASS()
protected:
    ref<Texture> m_nested;
    Float m_scale;
};

MTS_IMPLEMENT_CLASS_VARIANT(ScalingTexture, Texture)
MTS_EXPORT_PLUGIN(ScalingTexture, "Scaling texture")
NAMESPACE_END(mitsuba)
