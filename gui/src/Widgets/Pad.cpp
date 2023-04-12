#include <HMP/Gui/Widgets/Pad.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Actions/Pad.hpp>

namespace HMP::Gui::Widgets
{

    Pad::Pad(): SidebarWidget{ "Pad" } {}

    void Pad::requestPad()
    {
        app().applyAction(*new HMP::Actions::Pad{ m_length, m_smoothIterations, m_smoothSurfVertWeight, m_cornerShrinkFactor });
    }

    void Pad::drawSidebar()
    {
        Utils::Controls::sliderI("Smooth iterations", m_smoothIterations, 0, 20);
        Utils::Controls::sliderPercentage("Smooth surface weight", m_smoothSurfVertWeight, 0.5, 2.0);
        Utils::Controls::sliderPercentage("Corner shrink factor", m_cornerShrinkFactor);
        const Real meshSize{ app().mesh.bbox().diag() };
        Utils::Controls::sliderReal("Length", m_length, meshSize / 500.0, meshSize / 5.0);
        if (ImGui::Button("Pad"))
        {
            requestPad();
        }
    }

}