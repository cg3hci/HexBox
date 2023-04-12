#pragma once

#include <HMP/Gui/Widget.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <string>

namespace HMP::Gui
{

	class App;

	class SidebarWidget : public Widget, private cinolib::SideBarItem
	{

	private:

		friend class App;

		void draw() override final;

	protected:

		SidebarWidget(const std::string& _title);

		virtual void drawSidebar() = 0;

		bool open() const;

	};

}