#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>
#include <vector>
#include <cinolib/gl/FreeCamera.hpp>
#include <string>
#include <optional>

namespace HMP::Gui::Widgets
{

	class Ae3d2ShapeExporter final: public SidebarWidget
	{

	private:

		struct Polygon final
		{
			std::vector<Vec> vertices;
			Real quality;
		};

		struct Keyframe final
		{
			std::vector<Polygon> polygons{};
			cinolib::FreeCamera<Real> camera{};
			Real qualityFactor;
		};

		std::vector<Keyframe> m_keyframes;
		std::optional<std::string> m_sampleError;
		bool m_includeFrameSize{ false };
		Real m_qualityFactor{ 0.0 };

		bool exportKeyframes(const std::vector<Ae3d2ShapeExporter::Keyframe>& _keyframes) const;

		bool requestExport() const;

		bool requestTargetExport() const;

		bool requestSample();

		void clear();

		I keyframeCount() const;

		bool empty() const;

		void drawSidebar() override;

	public:

		Ae3d2ShapeExporter();

	};

}