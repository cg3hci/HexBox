#include <HMP/Gui/Widgets/Axes.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/color.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <cmath>
#include <array>
#include <utility>
#include <algorithm>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>

namespace HMP::Gui::Widgets
{

	void Axes::drawCanvas()
	{
		using Utils::Controls::toImVec2;
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		Vec origin;
		Real radius;
		const cinolib::FreeCamera<Real>& camera{ app().canvas.camera };
		if (camera.projection.perspective)
		{
			origin = camera.view.centerAt(3);
			radius = std::tan(cinolib::to_rad(camera.projection.verticalFieldOfView / 2));
		}
		else
		{
			origin = camera.view.centerAt(2);
			radius = camera.projection.verticalFieldOfView / 2 * 0.75;
		}
		const Vec right(origin + cinolib::GLcanvas::world_right() * radius);
		const Vec up(origin + cinolib::GLcanvas::world_up() * radius);
		const Vec forward(origin - cinolib::GLcanvas::world_forward() * radius);
		const Real maxSize{ static_cast<Real>(std::min(app().canvas.canvas_width(), app().canvas.height())) };
		const Real size{ std::min((maxSize * 0.1 + 100 * static_cast<Real>(themer->ovScale)) / 2, maxSize / 3) };
		const auto project{ [&](const Vec& _point) -> Vec {
			Vec proj(camera.projectionViewMatrix() * _point);
			proj.x() *= camera.projection.aspectRatio;
			const ImVec2 windowOrigin{ ImGui::GetWindowPos() };
			const ImVec2 windowSize{ ImGui::GetWindowSize() };
			proj.x() = proj.x() * size + windowOrigin.x + windowSize.x - size;
			proj.y() = -proj.y() * size + windowOrigin.y + windowSize.y - size;
			return proj;
		} };
		origin = project(origin);
		std::array<std::pair<Vec, ImColor>, 3> tips{
			std::pair<Vec, ImColor>{project(right), Utils::Drawing::toImU32(cinolib::Color::hsv2rgb(0.0f / 3.0f, themer->ovAxesSat, themer->ovAxesVal)) },
			std::pair<Vec, ImColor>{project(up), Utils::Drawing::toImU32(cinolib::Color::hsv2rgb(1.0f / 3.0f, themer->ovAxesSat, themer->ovAxesVal)) },
			std::pair<Vec, ImColor>{project(forward), Utils::Drawing::toImU32(cinolib::Color::hsv2rgb(2.0f / 3.0f, themer->ovAxesSat, themer->ovAxesVal)) }
		};
		std::sort(tips.begin(), tips.end(), [](const std::pair<Vec, ImColor>& _a, const std::pair<Vec, ImColor>& _b) { return _a.first.z() > _b.first.z(); });
		for (const auto& [tip, color] : tips)
		{
			Utils::Drawing::line(drawList, { toImVec2(origin.rem_coord()), toImVec2(tip.rem_coord()) }, color, 3.0f);
			Utils::Drawing::circleFilled(drawList, toImVec2(tip.rem_coord()), 5.0f, color);
		}
	}

}