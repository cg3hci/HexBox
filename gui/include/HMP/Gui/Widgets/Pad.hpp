#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::Widgets
{

    class Pad final: public SidebarWidget
    {

    private:

        Real m_length{ 0.0 };
        I m_smoothIterations{ 1 };
        Real m_smoothSurfVertWeight{ 1.0 };
        Real m_cornerShrinkFactor{ 0.5 };

        void requestPad();

        void drawSidebar() override;

    public:

        Pad();

    };

}