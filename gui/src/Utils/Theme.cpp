#include <HMP/Gui/Utils/Theme.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <cinolib/color.h>
#include <cmath>

namespace HMP::Gui::Utils
{

    inline constexpr cinolib::Color hsv(float _r, float _g, float _b, float _a = 1.0f)
    {
        return cinolib::Color::hsv2rgb(_r, _g, _b, _a);
    }

    using Drawing::toImU32;
    using Controls::toImVec4;

    static constexpr float okHue{ 0.4f }, warnHue{ 0.15f }, errHue{ 0.02f };

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
    static constexpr float dagNodeRefineHueShift{ 0.4f }, dagNodeExtrudeHueShift{ 0.6f }, dagNodeDeleteHueShift{ 0.8f };
#endif

    Theme Theme::makeLight(float _hueDeg, float _scale)
    {
        const float hue{ _hueDeg / 360.0f };
        return Theme{
            .ovScale = (1.0f + _scale * 2.0f) / 3.0f,
            .sbScale = _scale,
            .hue = hue,
            .dark = false,
            .sbOk{ toImVec4(hsv(okHue, 0.8f, 0.4f)) },
            .sbWarn{ toImVec4(hsv(warnHue, 0.8f, 0.45f))},
            .sbErr{ toImVec4(hsv(errHue, 0.85f, 0.45f))},
            .bg{ hsv(hue, 0.0f, 0.97f) },
            .ovHi = toImU32(hsv(hue, 0.8f, 0.9f)),
            .ovMut = toImU32(hsv(hue, 0.0f, 0.0f, 0.28f)),
            .ovWarn = toImU32(hsv(warnHue, 0.8f, 0.55f)),
            .ovErr = toImU32(hsv(errHue, 1.0f, 0.75f)),
            .ovPolyHi = toImU32(hsv(hue, 0.3f, 0.85f, 0.2f)),
            .ovFaceHi = toImU32(hsv(hue, 0.75f, 1.0f, 0.3f)),
            .srcFace{ hsv(hue, 0.0f, 0.8f) },
            .srcEdge{ hsv(hue, 0.0f, 0.22f) },
            .tgtFace{ hsv(hue, 0.0f, 0.0f, 0.08f) },
            .tgtEdge{ hsv(hue, 0.0f, 0.0f, 0.24f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
            .dagNodeEl = toImU32(hsv(hue, 0.0f, 0.6f)),
            .dagNodeElHi = toImU32(hsv(hue, 0.7f, 0.9f)),
            .dagNodeElMut = toImU32(hsv(hue, 0.0f, 0.83f)),
            .dagNodeRefine = toImU32(hsv(std::fmod(hue + dagNodeRefineHueShift, 1.0f), 0.7f, 0.75f)),
            .dagNodeExtrude = toImU32(hsv(std::fmod(hue + dagNodeExtrudeHueShift, 1.0f), 0.7f, 0.75f)),
            .dagNodeDelete = toImU32(hsv(std::fmod(hue + dagNodeDeleteHueShift, 1.0f), 0.7f, 0.75f)),
#endif
            .ovAxesSat = 0.55f,
            .ovAxesVal = 0.9f,
            .ovPathSat = 0.8f,
            .ovPathVal = 0.75f
        };
    }

    Theme Theme::makeDark(float _hueDeg, float _scale)
    {
        const float hue{ _hueDeg / 360.0f };
        return Theme{
            .ovScale = (1.0f + _scale * 2.0f) / 3.0f,
            .sbScale = _scale,
            .hue = hue,
            .dark = true,
            .sbOk{ toImVec4(hsv(okHue, 0.5f, 0.8f))},
            .sbWarn{ toImVec4(hsv(warnHue, 0.5f, 0.81f))},
            .sbErr{ toImVec4(hsv(errHue, 0.5f, 0.83f))},
            .bg{ hsv(hue, 0.0f, 0.09f) },
            .ovHi = toImU32(hsv(hue, 0.5f, 1.0f)),
            .ovMut = toImU32(hsv(hue, 0.0f, 1.0f, 0.27f)),
            .ovWarn = toImU32(hsv(warnHue, 0.5f, 0.55f)),
            .ovErr = toImU32(hsv(errHue, 0.7f, 0.7f)),
            .ovPolyHi = toImU32(hsv(hue, 0.75f, 0.5f, 0.1f)),
            .ovFaceHi = toImU32(hsv(hue, 0.75f, 1.0f, 0.2f)),
            .srcFace{ hsv(hue, 0.0f, 0.25f) },
            .srcEdge{ hsv(hue, 0.0f, 0.0f) },
            .tgtFace{ hsv(hue, 0.0f, 1.0f, 0.09f) },
            .tgtEdge{ hsv(hue, 0.0f, 1.0f, 0.27f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
            .dagNodeEl = toImU32(hsv(hue, 0.0f, 0.37f)),
            .dagNodeElHi = toImU32(hsv(hue, 0.55f, 1.0f)),
            .dagNodeElMut = toImU32(hsv(hue, 0.0f, 0.22f)),
            .dagNodeRefine = toImU32(hsv(std::fmod(hue + dagNodeRefineHueShift, 1.0f), 0.5f, 0.75f)),
            .dagNodeExtrude = toImU32(hsv(std::fmod(hue + dagNodeExtrudeHueShift, 1.0f), 0.5f, 0.75f)),
            .dagNodeDelete = toImU32(hsv(std::fmod(hue + dagNodeDeleteHueShift, 1.0f), 0.5f, 0.75f)),
#endif
            .ovAxesSat = 0.6f,
            .ovAxesVal = 0.8f,
            .ovPathSat = 0.65f,
            .ovPathVal = 0.85f
        };
    }

    Theme Theme::make(bool _dark, float _hueDeg, float _scale)
    {
        return _dark ? makeDark(_hueDeg, _scale) : makeLight(_hueDeg, _scale);
    }

}