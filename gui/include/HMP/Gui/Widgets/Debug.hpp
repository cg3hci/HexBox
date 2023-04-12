#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/SidebarWidget.hpp>
#include <cinolib/meshes/polygonmesh.h>
#include <cinolib/drawable_segment_soup.h>
#include <HMP/Refinement/Schemes.hpp>
#include <vector>

namespace HMP::Gui::Widgets
{

	class Debug final : public SidebarWidget
	{

	private:

		cinolib::DrawableSegmentSoup m_sectionSoup;

		unsigned int m_negJacTestRes{}, m_closeVertsTestRes{};

		Real sectionValue() const;

		bool sectionExports{ false };
		bool sectionExportsInv{ false };
		Real testEps{ 1e-9 };
		float fontBuildSize{ 13.0f };
		float namesFontScale{ 1.0f };
		Real sectionFactor{ 0.0 };
		int sectionDim{ 1 };
		float sectionLineThickness{ 1.0f };
		bool exportColors{ false };
		I fi{}, fiVi{};
		Refinement::EScheme refineSingleScheme{ Refinement::EScheme::Test };
		Real weldAbsEpsExp{ -6 }, weldEpsFactorExp{ -1 };
		bool showElements{ false }, showVids{ false }, showEids{ false }, showFids{ false }, showPids{ false };

		void selectCloseVerts();

		void selectNegJacobianHexes();

		void refineSingle();

		void exportTarget() const;

		void exportSource(bool _includeInterior) const;

		void updateSection();

		void drawCanvas() override;

		void drawSidebar() override;

		std::vector<const cinolib::DrawableObject*> additionalDrawables() const override;

		void attached() override;

		static void exportWithColors(const cinolib::Polygonmesh<>& _mesh, const std::vector<Real>& _fidQuality, const char* _file);

		Real pidQuality(Id _pid) const;

		void exportQualityList() const;

	public:

		float themeHue{ 32.0f };
		bool themeDark{ true };
		float themeScale{ 1.0f };

		Debug();

		void updateTheme() const;

	};

}