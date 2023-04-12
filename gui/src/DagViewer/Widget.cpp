#include <HMP/Gui/DagViewer/Widget.hpp>

#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/clamp.h>
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <string>
#include <HMP/Gui/Utils/HrDescriptions.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <limits>
#include <chrono>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/unreachable.hpp>
#include <cpputils/range/zip.hpp>
#include <sstream>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Gui/DagViewer/createLayout.hpp>

namespace HMP::Gui::DagViewer
{

	Widget::Widget(): SidebarWidget{ "Dag" }
	{}

	void Widget::actionApplied()
	{
		m_needsLayoutUpdate = true;
	}

	const Dag::Node* Widget::hovered() const 
	{
		return m_hovered;
	}

	std::vector<cinolib::GLcanvas::Font> Widget::additionalFonts(const std::vector<cinolib::GLcanvas::Font>& _fonts) const
	{
		constexpr unsigned int c_minSize{ 128 };
		for (const cinolib::GLcanvas::Font& font : _fonts)
		{
			if (font.size >= c_minSize)
			{
				return {};
			}
		}
		// FIXME
		//std::unordered_set<char> alphabet{ app().dagNamer.alphabetChars.begin(), app().dagNamer.alphabetChars.end() };
		//alphabet.insert(app().dagNamer.padChar);
		//alphabet.insert(app().dagNamer.unknownChar);
		return { cinolib::GLcanvas::Font{ c_minSize, /*alphabet*/ {}}};
	}

	void Widget::drawCanvas()
	{
		const float
			semiBoldLineThickness{ 2.5f * themer->ovScale },
			lineSpacing{ (10.0f * themer->ovScale + 10.0f) / 2.0f };
		if (app().canvas.show_sidebar() && open())
		{
			ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
			if (m_hovered && m_hovered->isElement())
			{
				const Id pid{ m_hovered->element().pid };
				for (const Id fid : app().mesh.adj_p2f(pid))
				{
					const QuadVerts fidVerts{ Meshing::Utils::verts(app().mesh, Meshing::Utils::fidVids(app().mesh, fid)) };
					const auto fidVerts2d{ Utils::Drawing::project(app().canvas, fidVerts) };
					Utils::Drawing::quadFilled(drawList, fidVerts2d, themer->ovPolyHi);
				}
				for (const Id eid : app().mesh.adj_p2e(pid))
				{
					const EdgeVerts eidVerts{ Meshing::Utils::verts(app().mesh, Meshing::Utils::eidVids(app().mesh, eid)) };
					const auto eidVerts2d{ Utils::Drawing::project(app().canvas, eidVerts) };
					Utils::Drawing::dashedLine(drawList, eidVerts2d, themer->ovFaceHi, semiBoldLineThickness, lineSpacing);
				}
			}
		}
		else
		{
			m_showLayoutPerformanceWarning = false;
		}
	}

	void Widget::resetView()
	{
		m_center_nl = Vec2{ m_layout.aspectRatio(), 1.0 } / 2;
		m_windowHeight_n = std::numeric_limits<Real>::infinity();
	}

	void Widget::drawTooltip(const Dag::Node& _node)
	{
		ImGui::BeginTable("tooltip", 2, ImGuiTableFlags_RowBg);
		ImGui::TableNextColumn(); ImGui::Text("Name");
		ImGui::TableNextColumn(); ImGui::Text("%s", Utils::HrDescriptions::name(_node, app().dagNamer).c_str());
		ImGui::TableNextColumn(); ImGui::Text("Kind");
		switch (_node.type)
		{
			case Dag::Node::EType::Element:
			{
				const Dag::Element& node{ _node.element() };
				ImGui::TableNextColumn(); ImGui::Text("Element");
				ImGui::TableNextColumn(); ImGui::Text("Pid");
				ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(node.pid));
				ImGui::TableNextColumn(); ImGui::Text("Vids");
				ImGui::TableNextColumn(); ImGui::Text("%u,%u,%u,%u,%u,%u,%u,%u",
					static_cast<unsigned int>(node.vids[0]),
					static_cast<unsigned int>(node.vids[1]),
					static_cast<unsigned int>(node.vids[2]),
					static_cast<unsigned int>(node.vids[3]),
					static_cast<unsigned int>(node.vids[4]),
					static_cast<unsigned int>(node.vids[5]),
					static_cast<unsigned int>(node.vids[6]),
					static_cast<unsigned int>(node.vids[7])
				);
				ImGui::TableNextColumn(); ImGui::Text("Visibility");
				bool hidden{ false };
				for (const Dag::Operation& child : node.children)
				{
					switch (child.primitive)
					{
						case Dag::Operation::EPrimitive::Delete:
							hidden = true;
							ImGui::TableNextColumn(); ImGui::Text("Deleted");
							break;
						case Dag::Operation::EPrimitive::Refine:
							hidden = true;
							ImGui::TableNextColumn(); ImGui::Text("Refined");
							break;
						default:
							break;
					}
					if (hidden)
					{
						break;
					}
				}
				if (!hidden)
				{
					ImGui::TableNextColumn(); ImGui::Text("Visible");
				}
				if (&node == app().copiedElement)
				{
					ImGui::TableNextColumn(); ImGui::Text("Copied");
					ImGui::TableNextColumn(); ImGui::Text("True");
				}
			}
			break;
			case Dag::Node::EType::Operation:
				switch (_node.operation().primitive)
				{
					case Dag::Operation::EPrimitive::Delete:
					{
						const Dag::Delete& node{ _node.as<Dag::Delete>() };
						ImGui::TableNextColumn(); ImGui::Text("Delete Op.");
					}
					break;
					case Dag::Operation::EPrimitive::Refine:
					{
						const Dag::Refine& node{ _node.as<Dag::Refine>() };
						ImGui::TableNextColumn(); ImGui::Text("Refine Op.");
						ImGui::TableNextColumn(); ImGui::Text("Scheme");
						ImGui::TableNextColumn(); ImGui::Text("%s", Utils::HrDescriptions::describe(node.scheme).c_str());
						ImGui::TableNextColumn(); ImGui::Text("'%s' forward face", Utils::HrDescriptions::name(node.parents.first(), app().dagNamer).c_str());
						ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(node.forwardFi));
						ImGui::TableNextColumn(); ImGui::Text("'%s' vert", Utils::HrDescriptions::name(node.parents.first(), app().dagNamer).c_str());
						ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(node.firstVi));
						ImGui::TableNextColumn(); ImGui::Text("Surface verts count");
						ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(node.surfVids.size()));
						ImGui::TableNextColumn(); ImGui::Text("Surface verts");
						std::ostringstream ss;
						for (I i{}; i < node.surfVids.size(); i++)
						{
							if (i > 0)
							{
								ss << ",";
							}
							ss << node.surfVids[i];
							if (i > 16)
							{
								ss << "...";
								break;
							}
						}
						ImGui::TableNextColumn(); ImGui::Text("%s", ss.str().c_str());
					}
					break;
					case Dag::Operation::EPrimitive::Extrude:
					{
						const Dag::Extrude& node{ _node.as<Dag::Extrude>() };
						ImGui::TableNextColumn(); ImGui::Text("Extrude Op.");
						ImGui::TableNextColumn(); ImGui::Text("Source");
						ImGui::TableNextColumn(); ImGui::Text("%s", Utils::HrDescriptions::describe(node.source).c_str());
						for (const auto& [parent, parentFi] : cpputils::range::zip(node.parents, node.fis))
						{
							ImGui::TableNextColumn(); ImGui::Text("'%s' face", Utils::HrDescriptions::name(parent, app().dagNamer).c_str());
							ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(parentFi));
						}
						ImGui::TableNextColumn(); ImGui::Text("'%s' vert", Utils::HrDescriptions::name(node.parents.first(), app().dagNamer).c_str());
						ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(node.firstVi));
						ImGui::TableNextColumn(); ImGui::Text("Direction");
						ImGui::TableNextColumn(); ImGui::Text(node.clockwise ? "CW" : "CCW");
					}
					break;
					default:
						cpputils::unreachable();
				}
				break;
			default:
				cpputils::unreachable();
		}
		ImGui::EndTable();
	}

	void Widget::updateLayout()
	{
		if (m_needsLayoutUpdate)
		{
			m_needsLayoutUpdate = false;
			m_tooManyNodes = app().mesh.num_polys() > 10000;
			if (!m_tooManyNodes)
			{
				const auto t1{ std::chrono::high_resolution_clock::now() };
				m_layout = DagViewer::createLayout(app().root());
				const auto t2{ std::chrono::high_resolution_clock::now() };
				const int64_t elapsedMs{ std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() };
				static constexpr int64_t warningTimeThresholdMs{ 200 };
				m_showLayoutPerformanceWarning = elapsedMs >= warningTimeThresholdMs;
			}
			else
			{
				m_layout = {};
				m_showLayoutPerformanceWarning = false;
			}
			resetView();
		}
	}

	void Widget::drawSidebar()
	{
		m_hovered = nullptr;

		updateLayout();

		if (m_tooManyNodes)
		{
			ImGui::TextColored(themer->sbErr, "That's too many nodes! Sorry. :(");
			return;
		}

		if (m_showLayoutPerformanceWarning)
		{
			ImGui::TextColored(themer->sbWarn, "Leaving this widget open will affect meshing performance!");
		}

		// types

		static constexpr auto toVec{ [](const ImVec2& _vec) {
			return Vec2{_vec.x, _vec.y};
		} };

		static constexpr auto toImVec{ [](const Vec2& _vec) {
			return ImVec2{static_cast<float>(_vec.x()), static_cast<float>(_vec.y())};
		} };

		// window

		const ImGuiStyle& style{ ImGui::GetStyle() };

		const Vec2 availWindowSize_s{ toVec(ImGui::GetContentRegionAvail()) };
		const Vec2 windowSize_s{
			availWindowSize_s.x(),
			std::max(
				std::min(
					availWindowSize_s.x(),
					750.0 * static_cast<Real>(themer->sbScale)
				),
				std::max(
					250.0 * static_cast<Real>(themer->sbScale),
					availWindowSize_s.y() - static_cast<Real>(style.ItemSpacing.y)
				)
			)
		};
		if (windowSize_s.x() <= 0.0 || windowSize_s.y() <= 0.0)
		{
			return;
		}
		const Vec2 topLeft_sw{ toVec(ImGui::GetCursorScreenPos()) };
		const Vec2 bottomRight_sw{ topLeft_sw + windowSize_s };
		const Real windowAspectRatio{ windowSize_s.x() / windowSize_s.y() };

		if (m_layout.size().x() <= 0.0 || m_layout.size().y() <= 0.0)
		{
			return;
		}

		// trasformations

		const Real s2n{ 1.0 / windowSize_s.y() };
		const Real n2s{ 1.0 / s2n };
		const Real n2l{ m_layout.size().y() };
		const Real l2n{ 1.0 / n2l };

		const auto ss2sw{ [&](const Vec2& _point_ss) {
			return _point_ss - topLeft_sw;
		} };

		const auto sw2ss{ [&](const Vec2& _point_sw) {
			return _point_sw + topLeft_sw;
		} };

		const auto sw2nw{ [&](const Vec2& _point_sw) {
			return Vec2{ _point_sw.x() * s2n, 1.0 - _point_sw.y() * s2n };
		} };

		const auto nw2sw{ [&](const Vec2& _point_nw) {
			return Vec2{ _point_nw.x(), 1.0 - _point_nw.y() } *n2s;
		} };

		const auto nw2nl{ [&](const Vec2& _point_nw) {
			return (_point_nw - Vec2{windowAspectRatio, 1} / 2) * m_windowHeight_n + m_center_nl;
		} };

		const auto nl2nw{ [&](const Vec2& _point_nl) {
			return (_point_nl - m_center_nl) / m_windowHeight_n + Vec2{windowAspectRatio, 1} / 2;
		} };

		/*const auto nl2ll{ [&](const Vec2& _point_nl) {
			return _point_nl * n2l + m_layout.bottomLeft();
		} };*/

		const auto ll2nl{ [&](const Vec2& _point_ll) {
			return (_point_ll - m_layout.bottomLeft()) * l2n;
		} };

		// font

		ImGuiIO& io = ImGui::GetIO();

		ImFont** maxFontIt{ std::max_element(io.Fonts->Fonts.begin(), io.Fonts->Fonts.end(), [](ImFont* _a, ImFont* _b) { return _a->FontSize < _b->FontSize; }) };
		ImGui::PushFont(*maxFontIt);

		// input

		ImGui::InvisibleButton("canvas", toImVec(windowSize_s), ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

		const auto clampCenter{ [&]() {
			m_center_nl.x() = cinolib::clamp(m_center_nl.x(), 0.0, m_layout.aspectRatio());
			m_center_nl.y() = cinolib::clamp(m_center_nl.y(), 0.0, 1.0);
		} };

		const auto clampHeight{ [&]() {
			const Real fullHeight_n {(windowAspectRatio < m_layout.aspectRatio()) ? m_layout.aspectRatio() / windowAspectRatio : 1};
			const Real min{ m_layout.nodeRadius() * l2n * 3 };
			const Real max{ fullHeight_n * 1.1 + m_layout.nodeRadius() * l2n };
			const Real maxByNumberOfNodes{ m_layout.nodeRadius() * l2n * 100 };
			m_windowHeight_n = cinolib::clamp(m_windowHeight_n, min, std::min(max, maxByNumberOfNodes));
		} };

		clampHeight();

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
		{
			m_center_nl -= Vec2{ io.MouseDelta.x, -io.MouseDelta.y } *s2n * m_windowHeight_n;
		}

		clampCenter();

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
		{
			constexpr Real speed{ 2 };
			const Vec2 mouse_ss{ toVec(io.MouseClickedPos[ImGuiMouseButton_Middle]) };
			const Vec2 oldMouse_nl{ nw2nl(sw2nw(ss2sw(mouse_ss))) };
			m_windowHeight_n += io.MouseDelta.y * s2n * m_windowHeight_n * speed;
			clampHeight();
			const Vec2 newMouse_nl{ nw2nl(sw2nw(ss2sw(mouse_ss))) };
			m_center_nl += oldMouse_nl - newMouse_nl;
			clampCenter();
		}

		// drawing

		const Dag::Node* hoveredNode{ };

		{
			ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
			const ImU32 borderColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Border]) };
			const ImU32 gridColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_TableBorderLight]) };
			const ImU32 strokeColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_PlotLines]) };
			const ImU32 backgroundColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_PopupBg]) };
			const float gridThickness{ 1.0f * themer->sbScale };
			const float lineThickness{ 1.5f * themer->sbScale };

			Utils::Drawing::rectFilled(drawList, toImVec(topLeft_sw), toImVec(bottomRight_sw), backgroundColor);
			Utils::Drawing::rect(drawList, toImVec(topLeft_sw), toImVec(bottomRight_sw), borderColor, gridThickness);

			drawList.PushClipRect(toImVec(topLeft_sw + Vec2{ 1,1 }), toImVec(bottomRight_sw - Vec2{ 1,1 }), true);
			{
				const auto ll2ss{ [&](const Vec2& _point_ll) {
					return sw2ss(nw2sw(nl2nw(ll2nl(_point_ll))));
				} };

				const Real l2s{ l2n / m_windowHeight_n * n2s };

				// grid

				{
					const int gridLevel{ static_cast<int>(-std::log2(m_windowHeight_n / 2.0)) };
					const Real gridStep_s{ n2s / (std::pow(2, gridLevel) * 10) / m_windowHeight_n };
					const Vec2 origin_ss{ sw2ss(nw2sw(nl2nw(Vec2{0,1}))) };
					for (Real x_s{ std::fmod(origin_ss.x(), gridStep_s) }; x_s <= bottomRight_sw.x(); x_s += gridStep_s)
					{
						Utils::Drawing::line(drawList, { toImVec(Vec2{ x_s, topLeft_sw.y() }), toImVec(Vec2{ x_s, bottomRight_sw.y() }) }, gridColor, gridThickness);
					}
					for (Real y_s{ std::fmod(origin_ss.y(), gridStep_s) }; y_s <= bottomRight_sw.y(); y_s += gridStep_s)
					{
						Utils::Drawing::line(drawList, { toImVec(Vec2{ topLeft_sw.x(), y_s }), toImVec(Vec2{ bottomRight_sw.x(), y_s }) }, gridColor, gridThickness);
					}
				}

				// edges

				for (const auto& [lineA, lineB] : m_layout.lines())
				{
					Utils::Drawing::line(drawList, { toImVec(ll2ss(lineA)), toImVec(ll2ss(lineB)) }, strokeColor, lineThickness);
				}

				// nodes

				const float nodeRadius_s{ static_cast<float>(m_layout.nodeRadius() * l2s) };
				const float copiedNodeRadius_s{ static_cast<float>(m_layout.nodeRadius() * l2s) * 1.1f };
				const Vec2 nodeHalfDiag_s{ nodeRadius_s, nodeRadius_s };
				const Vec2 copiedNodeHalfDiag_s{ copiedNodeRadius_s, copiedNodeRadius_s };
				const Vec2 mouse{ toVec(ImGui::GetMousePos()) };
				constexpr int circleSegmentsNear{ 10 }, circleSegmentsFar{ 4 };
				const int circleSegments{ nodeRadius_s < 10.0 ? circleSegmentsFar : circleSegmentsNear };
				for (const Layout::Node& node : m_layout.nodes())
				{
					const Vec2 center{ ll2ss(node.center()) };
					bool hovered{ false };
					switch (node.node().type)
					{
						case Dag::Node::EType::Element:
						{
							const Dag::Element& element{ node.node().element() };
							const ImU32 color{ app().cursor.element == &node.node()
								? themer->dagNodeElHi
								: Meshing::Utils::isShown(element) ? themer->dagNodeEl : themer->dagNodeElMut };
							Utils::Drawing::rectFilled(drawList, toImVec(center - nodeHalfDiag_s), toImVec(center + nodeHalfDiag_s), color);
							Utils::Drawing::rect(drawList, toImVec(center - nodeHalfDiag_s), toImVec(center + nodeHalfDiag_s), strokeColor, lineThickness);
							if (&element == app().copiedElement)
							{
								Utils::Drawing::rect(drawList, toImVec(center - copiedNodeHalfDiag_s), toImVec(center + copiedNodeHalfDiag_s), strokeColor, lineThickness);
							}
							const Vec2 mouseCenterDiff{ mouse - center };
							hovered =
								mouseCenterDiff.x() <= nodeHalfDiag_s.x() &&
								mouseCenterDiff.x() >= -nodeHalfDiag_s.x() &&
								mouseCenterDiff.y() <= nodeHalfDiag_s.y() &&
								mouseCenterDiff.y() >= -nodeHalfDiag_s.y();
						}
						break;
						case Dag::Node::EType::Operation:
						{
							ImU32 operationColor{};
							switch (node.node().operation().primitive)
							{
								case Dag::Operation::EPrimitive::Extrude:
									operationColor = themer->dagNodeExtrude;
									break;
								case Dag::Operation::EPrimitive::Refine:
									operationColor = themer->dagNodeRefine;
									break;
								case Dag::Operation::EPrimitive::Delete:
									operationColor = themer->dagNodeDelete;
									break;
							}
							Utils::Drawing::circleFilled(drawList, toImVec(center), nodeRadius_s, operationColor, circleSegments);
							Utils::Drawing::circle(drawList, toImVec(center), nodeRadius_s, strokeColor, lineThickness, circleSegments);
							const Vec2 mouseCenterDiff{ mouse - center };
							hovered = mouseCenterDiff.norm() <= nodeRadius_s;
						}
						break;
					}
					if (nodeRadius_s > 10.0f)
					{
						const std::string text{ Utils::HrDescriptions::name(node.node(), app().dagNamer) };
						const Vec2 textSize{ toVec(ImGui::CalcTextSize(text.c_str())) / ImGui::GetFontSize() };
						const Real maxTextSize{ std::max(textSize.x(), textSize.y()) };
						const Real fontSize{ nodeRadius_s * 1.25 / maxTextSize };
						Utils::Drawing::text(drawList, text.c_str(), toImVec(center), static_cast<float>(fontSize), backgroundColor);
					}
					if (hovered)
					{
						hoveredNode = &node.node();
					}
				}
			}
			drawList.PopClipRect();
		}


		ImGui::PopFont();

		// tooltip

		if (hoveredNode && ImGui::IsItemHovered() && !ImGui::IsItemActive())
		{
			m_hovered = hoveredNode;
			ImGui::BeginTooltip();
			drawTooltip(*hoveredNode);
			ImGui::EndTooltip();
		}

	}

}