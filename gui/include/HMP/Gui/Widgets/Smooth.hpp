#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::Widgets
{

    class Smooth final: public SidebarWidget
    {

    private:

        I m_surfaceIterations{ 1 };
        I m_internalIterations{ 1 };
        Real m_surfVertWeight{ 1.0 };

        void drawSidebar() override;

    public:

        Smooth();

        void requestSmooth();

    };

}