#include <HMP/Gui/Widgets/Smooth.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Actions/Smooth.hpp>
#include <HMP/Gui/App.hpp>

namespace HMP::Gui::Widgets
{

    Smooth::Smooth(): SidebarWidget{ "Smooth" } {}

    void Smooth::requestSmooth()
    {
        app().applyAction(*new HMP::Actions::Smooth{ m_surfaceIterations, m_internalIterations, m_surfVertWeight });
    }

    void Smooth::drawSidebar()
    {
        Utils::Controls::sliderI("Surface iterations", m_surfaceIterations, 0, 20);
        Utils::Controls::sliderI("Internal iterations", m_internalIterations, 0, 20);
        Utils::Controls::sliderPercentage("Smooth surface weight", m_surfVertWeight, 0.5, 2.0);
        if (ImGui::Button("Smooth"))
        {
            requestSmooth();
        }
    }

}