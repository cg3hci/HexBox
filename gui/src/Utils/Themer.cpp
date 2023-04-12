#include <HMP/Gui/Utils/Themer.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Transform.hpp>

namespace HMP::Gui::Utils
{

    void Themer::setTheme(const Theme& _theme)
    {
        m_theme = _theme;
        applyImGui();
        onThemeChange();
    }

    const Theme& Themer::operator*() const
    {
        return m_theme;
    }

    const Theme* Themer::operator->() const
    {
        return &m_theme;
    }

    void Themer::applyImGui() const
    {
        static constexpr float lightStyleHue{ 213.0f / 360.0f };
        static constexpr float darkStyleHue{ 213.0f / 360.0f };
        const float styleHue{ m_theme.dark ? darkStyleHue : lightStyleHue };
        ImGuiStyle& style{ ImGui::GetStyle() };
        style = {};
        if (m_theme.dark)
        {
            ImGui::StyleColorsDark();
        }
        else
        {
            ImGui::StyleColorsLight();
        }
        style.ScaleAllSizes(m_theme.sbScale);
        for (ImVec4& color : style.Colors)
        {
            float h, s, v;
            ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, h, s, v);
            h += m_theme.hue - styleHue;
            h = static_cast<float>(Utils::Transform::wrapAngle(static_cast<Real>(h * 360.0f))) / 360.0f;
            ImGui::ColorConvertHSVtoRGB(h, s, v, color.x, color.y, color.z);
        }
    }

}