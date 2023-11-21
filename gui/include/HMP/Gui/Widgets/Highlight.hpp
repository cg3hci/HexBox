#pragma once

#include <HMP/Gui/Widget.hpp>

namespace HMP::Gui::Widgets
{

	class Highlight final: public Widget
	{

	private:

		void drawCanvas() override;

        bool mouseClicked(bool _right);
    };

}