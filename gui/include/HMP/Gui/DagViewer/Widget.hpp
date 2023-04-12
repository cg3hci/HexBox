#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Gui/DagViewer/Layout.hpp>
#include <HMP/Dag/Node.hpp>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::DagViewer
{

	class Widget final: public SidebarWidget
	{

	private:

		Layout m_layout;
		bool m_needsLayoutUpdate{ true };
		bool m_tooManyNodes{ false };
		Vec2 m_center_nl{ 0.5, 0.5 };
		Real m_windowHeight_n{ 1.0 };
		bool m_showLayoutPerformanceWarning{ false };

		void zoom(Real _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

		void drawTooltip(const Dag::Node& _node);

		const Dag::Node* m_hovered{};

		void drawSidebar() override;

		void drawCanvas() override;

		void resetView();

		void updateLayout();

		void actionApplied() override;

		std::vector<cinolib::GLcanvas::Font> additionalFonts(const std::vector<cinolib::GLcanvas::Font>& _fonts) const override;

	public:

		Widget();

		const Dag::Node* hovered() const;

	};

}