#include <HMP/Gui/SidebarWidget.hpp>

namespace HMP::Gui
{

	void SidebarWidget::draw()
	{
		drawSidebar();
	}

	SidebarWidget::SidebarWidget(const std::string& _title) : cinolib::SideBarItem{ _title }
	{}

	bool SidebarWidget::open() const
	{
		return show_open;
	}

}