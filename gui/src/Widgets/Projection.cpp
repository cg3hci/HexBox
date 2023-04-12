#include <HMP/Gui/Widgets/Projection.hpp>

#include <cassert>
#include <HMP/Actions/Project.hpp>
#include <cinolib/export_surface.h>
#include <cinolib/deg_rad.h>
#include <cinolib/dijkstra.h>
#include <cinolib/feature_mapping.h>
#include <cinolib/feature_network.h>
#include <cinolib/meshes/polygonmesh.h>
#include <cinolib/meshes/drawable_polygonmesh.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <HMP/Projection/Utils.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/App.hpp>

namespace HMP::Gui::Widgets
{

	Projection::Projection():
		SidebarWidget{ "Projection" },
		m_options{}, m_paths(1), m_featureFinderOptions{}, m_usePathAsCrease{ false }, m_featureFinderCreaseAngle{ 60.0f },
		m_showPaths{ false }, m_showAllPaths{ false }, m_currentPath{}
	{}

	void Projection::attached()
	{
		app().targetWidget.onMeshChanged += [this]() {
			for (auto& path : m_paths)
			{
				path.targetEids.clear();
			}
		};
		app().mesher.onAdded += [this](const Meshing::Mesher::State&) {
			for (I i{}; i < m_paths.size(); i++)
			{
				for (const Id eid : m_paths[i].sourceEids)
				{
					if (!app().mesh.edge_is_visible(eid))
					{
						m_paths[i].sourceEids.clear();
						break;
					}
				}
			}
		};
		app().mesher.onElementVisibilityChanged += [this](const Dag::Element& _element, bool) {
			const std::vector<Id> removedEids{ app().mesh.poly_dangling_eids(_element.pid) };
			const std::unordered_set<Id> removedEidsSet{ removedEids.begin(), removedEids.end() };
			for (I i{}; i < m_paths.size(); i++)
			{
				for (Id& eid : m_paths[i].sourceEids)
				{
					if (removedEidsSet.contains(eid))
					{
						m_paths[i].sourceEids.clear();
						break;
					}
				}
			}
		};
		app().mesher.onRestored += [this](const Meshing::Mesher::State&) {
			const Id eidCount{ app().mesh.num_edges() };
			for (I i{}; i < m_paths.size(); i++)
			{
				for (const Id eid : m_paths[i].sourceEids)
				{
					if (eid >= eidCount)
					{
						m_paths[i].sourceEids.clear();
						break;
					}
				}
			}
		};
	}

	const HMP::Projection::Options& Projection::options() const
	{
		return m_options;
	}

	void Projection::requestProjection()
	{
		assert(app().targetWidget.hasMesh());
		std::vector<Point> points;
		points.reserve(m_paths.size() * 2);
		std::vector<EidsPath> paths;
		paths.reserve(m_paths.size());
		cinolib::Polygonmesh<> targetMesh{ std::move(app().targetWidget.meshForProjection()) };
		for (const EidsPath& path : m_paths)
		{
			if (path.sourceEids.empty() || path.targetEids.empty())
			{
				continue;
			}
			paths.push_back(path);
			for (const Point& point : HMP::Projection::Utils::endPoints(path, app().mesh, targetMesh))
			{
				points.push_back(point);
			}
		}
		if (m_lockSelectedVertices)
		{
			std::vector<bool> mask(toI(app().mesh.num_verts()), true);
			for (const Id vid : app().vertEditWidget.vids())
			{
				mask[toI(vid)] = false;
			}
			m_options.vertexMask = mask;
		}
		else
		{
			m_options.vertexMask = std::nullopt;
		}
		app().applyAction(*new HMP::Actions::Project{ std::move(targetMesh), points, paths, m_options });
	}

	bool Projection::canReproject() const
	{
		if (!app().commander.canUndo())
		{
			return false;
		}
		return dynamic_cast<const HMP::Actions::Project*>(&app().commander.applied().first());
	}

	void Projection::requestReprojection()
	{
		assert(canReproject());
		app().commander.undo();
		requestProjection();
	}

	void Projection::matchPaths(I _first, I _lastEx, bool _fromSource)
	{
		for (I i{ _first }; i < _lastEx; i++)
		{
			matchPath(i, _fromSource);
		}
	}

	template<typename M, typename V, typename E, typename P>
	void joinPaths(std::vector<std::vector<Id>>& _vids, const cinolib::AbstractMesh<M, V, E, P>& _mesh, std::vector<bool>& _mask)
	{
		assert(_mask.size() == toI(_mesh.num_verts()));
		std::vector<Id> fullVids{}, tempVids{};
		for (const std::vector<Id>& vids : _vids)
		{
			assert(vids.size() > 1);
			for (const Id vid : vids)
			{
				_mask[toI(vid)] = true;
			}
			_mask[toI(vids.front())] = false;
			_mask[toI(vids.back())] = false;
		}
		for (I i{}; i < _vids.size(); i++)
		{
			tempVids.clear();
			if (!fullVids.empty() && _vids[i].front() == fullVids.back())
			{
				fullVids.pop_back();
			}
			fullVids.insert(fullVids.end(), _vids[i].begin(), _vids[i].end());
			cinolib::dijkstra(_mesh, _vids[i].back(), _vids[(i + 1) % _vids.size()].front(), _mask, tempVids);
			if (tempVids.size() > 2)
			{
				fullVids.insert(fullVids.end(), tempVids.begin() + 1, tempVids.end() - 1);
			}
		}
		_vids = { fullVids };
	}

	template<typename M, typename V, typename E, typename P>
	void joinPaths(std::vector<std::vector<Id>>& _vids, const cinolib::AbstractMesh<M, V, E, P>& _mesh)
	{
		std::vector<bool> mask(toI(_mesh.num_verts()), false);
		joinPaths(_vids, _mesh, mask);
	}

	void Projection::matchPath(I _i, bool _fromSource)
	{
		cinolib::Polygonmesh<> target{ app().targetWidget.meshForProjection() };
		const Meshing::Mesher::Mesh& source{ app().mesh };
		m_paths[_i].eids(!_fromSource).clear();
		std::vector<std::vector<Id>> fromVids{
			_fromSource
				? HMP::Projection::Utils::eidsToVidsPath(source, m_paths[_i].sourceEids)
				: HMP::Projection::Utils::eidsToVidsPath(target, m_paths[_i].targetEids)
		}, toVids;
		if (HMP::Projection::Utils::isVidsPathClosed(fromVids[0]))
		{
			const I midI{ fromVids[0].size() / 2 };
			const std::vector<Id> fullVids{fromVids[0]};
			fromVids = {
				std::vector<Id>(fullVids.begin(), fullVids.begin() + midI),
				std::vector<Id>(fullVids.begin() + midI, fullVids.end())
			};
		}
		std::unordered_map<Id, Id> surf2vol, vol2surf;
		cinolib::Polygonmesh<> sourceSurf{};
		cinolib::export_surface(source, sourceSurf, vol2surf, surf2vol, false);
		if (_fromSource)
		{
			for (std::vector<Id>& vids : fromVids)
			{
				for (Id& vid : vids)
				{
					vid = vol2surf[vid];
				}
			}
			if (!cinolib::feature_mapping(sourceSurf, fromVids, target, toVids))
			{
				return;
			}
		}
		else
		{
			if (!cinolib::feature_mapping(target, fromVids, sourceSurf, toVids))
			{
				return;
			}
			for (std::vector<Id>& vids : toVids)
			{
				for (Id& vid : vids)
				{
					vid = surf2vol[vid];
				}
			}
		}
		if (toVids.size() > 1)
		{
			if (_fromSource)
			{
				joinPaths(toVids, target);
			}
			else
			{
				std::vector<bool> mask(source.num_verts(), false);
				for (Id vid{}; vid < source.num_verts(); vid++)
				{
					mask[toI(vid)] = !source.vert_is_visible(vid) || !source.vert_is_on_srf(vid);
				}
				joinPaths(toVids, source, mask);
			}
		}
		if (_fromSource)
		{
			m_paths[_i].targetEids = HMP::Projection::Utils::vidsToEidsPath(target, toVids[0]);
		}
		else
		{
			m_paths[_i].sourceEids = HMP::Projection::Utils::vidsToEidsPath(source, toVids[0]);
		}
	}

	void Projection::findPaths(bool _inSource)
	{
		std::vector<std::vector<Id>> network{};
		cinolib::Polygonmesh<> mesh;
		std::unordered_map<Id, Id> surf2vol;
		if (_inSource)
		{
			std::unordered_map<Id, Id> vol2surf;
			cinolib::export_surface(app().mesh, mesh, vol2surf, surf2vol, false);
			if (m_usePathAsCrease)
			{
				for (const EidsPath path : m_paths)
				{
					for (const Id eid : path.sourceEids)
					{
						const Id surfEid{ static_cast<Id>(mesh.edge_id(
							vol2surf.at(app().mesh.edge_vert_id(eid, 0)),
							vol2surf.at(app().mesh.edge_vert_id(eid, 1))
						)) };
						mesh.edge_data(surfEid).flags[cinolib::CREASE] = true;
					}
				}
			}
		}
		else
		{
			mesh = app().targetWidget.meshForDisplay();
			if (m_usePathAsCrease)
			{
				for (const EidsPath path : m_paths)
				{
					for (const Id eid : path.targetEids)
					{
						mesh.edge_data(eid).flags[cinolib::CREASE] = true;
					}
				}
			}
		}
		if (!m_usePathAsCrease)
		{
			mesh.edge_mark_sharp_creases(static_cast<float>(cinolib::to_rad(static_cast<double>(m_featureFinderCreaseAngle))));
		}
		cinolib::feature_network(mesh, network, m_featureFinderOptions);
		m_paths.resize(std::max(m_paths.size(), network.size()));
		network.resize(m_paths.size());
		for (const auto& [path, featVids] : cpputils::range::zip(m_paths, network))
		{
			if (_inSource)
			{
				for (Id& vid : featVids)
				{
					vid = surf2vol.at(vid);
				}
				path.sourceEids = HMP::Projection::Utils::vidsToEidsPath(app().mesh, featVids);
			}
			else
			{
				path.targetEids = HMP::Projection::Utils::vidsToEidsPath(mesh, featVids);
			}
		}
	}

	void Projection::setSourcePathEdgeAtPoint(const Vec& _point, bool _add)
	{
		setPathEdgeAtPoint(_point, _add, app().mesh, true);
	}

	void Projection::setTargetPathEdgeAtPoint(const Vec& _point, bool _add)
	{
		setPathEdgeAtPoint(app().targetWidget.meshForDisplay().transform.inverse() * _point, _add, app().targetWidget.meshForDisplay(), false);
	}

	ImVec4 Projection::pathColor(I _path) const
	{
		ImVec4 color;
		ImGui::ColorConvertHSVtoRGB(static_cast<float>(_path) / static_cast<float>(m_paths.size()), themer->ovPathSat, themer->ovPathVal, color.x, color.y, color.z);
		color.w = 1.0f;
		return color;
	}

	void Projection::removePath(I _path)
	{
		if (_path + 1 != m_paths.size())
		{
			std::swap(m_paths[_path], m_paths.back());
		}
		m_paths.pop_back();
		if (_path == m_currentPath)
		{
			m_currentPath = 0;
		}
	}

	void Projection::drawSidebar()
	{
#ifdef HMP_ENABLE_ALT_PROJ
		ImGui::Checkbox("Alternative method", &m_options.alternativeMethod);
		ImGui::Spacing();
#endif
		static constexpr auto tweak{ [](HMP::Projection::Utils::Tweak& _tweak, const char* _label) {
			ImGui::PushID(&_tweak);
			ImGui::Text("%s", _label);
			Real min{ _tweak.min() }, power{ _tweak.power() };
			Utils::Controls::sliderReal("Min", min, -2.0, 1.0);
			Utils::Controls::sliderReal("Power", power, 0.0, 4.0, true);
			_tweak = { min, power };
			ImGui::PopID();
		} };
		if (!m_options.alternativeMethod)
		{
			if (ImGui::TreeNode("Weights"))
			{
				ImGui::Spacing();
				Utils::Controls::combo("Displace mode", m_options.displaceMode, { "NormDirAvgAndDirNormAvg", "NormDirAvgAndDirAvg", "DirAvg", "VertAvg" });
				ImGui::Spacing();
				tweak(m_options.baseWeightTweak, "Base weight factor");
				Utils::Controls::combo("Mode", m_options.baseWeightMode, { "Distance", "Barycentric coords" });
				ImGui::Spacing();
				tweak(m_options.normalDotTweak, "Normal dot factor");
				ImGui::Spacing();
				ImGui::Text("Distance weight factor");
				Utils::Controls::sliderReal("Weight", m_options.distanceWeight, 0.0, 10.0);
				Utils::Controls::sliderReal("Power", m_options.distanceWeightPower, 0.0, 4.0, true);
				ImGui::Spacing();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Fill and advance"))
			{
				ImGui::Spacing();
				tweak(m_options.unsetVertsDistWeightTweak, "Fill distance factor");
				ImGui::Spacing();
				ImGui::Text("Advance");
				Utils::Controls::sliderPercentage("Advance percentile", m_options.advancePercentile);
				ImGui::Spacing();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Smoothing"))
			{
				ImGui::Spacing();
				Utils::Controls::sliderI("Surface iterations", m_options.smoothSurfaceIterations, 0, 20);
				Utils::Controls::sliderI("Internal iterations", m_options.smoothInternalIterations, 0, 20);
				Utils::Controls::sliderPercentage("Internal done weight", m_options.smoothInternalDoneWeight, 0.25, 4.0);
				ImGui::Spacing();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Jacobian check"))
			{
				ImGui::Spacing();
				Utils::Controls::combo("Target", m_options.jacobianCheckMode, { "None", "Surface only", "All" });
				Utils::Controls::combo("Advance mode", m_options.jacobianAdvanceMode, { "Length", "Lerp" });
				Utils::Controls::sliderI("Max tests", m_options.jacobianAdvanceMaxTests, 2, 20);
				Utils::Controls::sliderPercentage("Stop threshold", m_options.jacobianAdvanceStopThreshold, 0.01, 0.4);
				ImGui::Spacing();
				ImGui::TreePop();
			}
		}
		ImGui::SetNextItemOpen(m_showPaths, ImGuiCond_Always);
		m_showPaths = ImGui::TreeNode("Paths");
		if (m_showPaths)
		{
			ImGui::Spacing();
			if (ImGui::Button("Add"))
			{
				m_currentPath = m_paths.size();
				m_paths.push_back({});
			}
			ImGui::SameLine();
			if (Utils::Controls::disabledButton("Clear", !m_paths.empty()))
			{
				m_currentPath = 0;
				m_paths.clear();
			}
			ImGui::SameLine();
			if (Utils::Controls::disabledButton("Clear empty", !cpputils::range::of(m_paths).filter([](const EidsPath& _path) { return _path.empty(); }).empty()))
			{
				for (I i{}; i < m_paths.size();)
				{
					if (m_paths[i].empty())
					{
						removePath(i);
					}
					else
					{
						++i;
					}
				}
			}
			ImGui::SameLine();
			ImGui::Checkbox("Show all", &m_showAllPaths);
			ImGui::SameLine();
			ImGui::TextColored(m_paths.empty() ? themer->sbWarn : themer->sbOk, "%u paths", static_cast<unsigned int>(m_paths.size()));
			ImGui::Spacing();
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 4.0f, 4.0f });
			ImGui::BeginTable(
				"List",
				8,
				ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter,
				{ ImGui::GetContentRegionAvail().x, 170.0f * themer->sbScale }
			);
			for (I i{}; i < m_paths.size(); i++)
			{
				ImGui::PushID(static_cast<int>(i));
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (m_showAllPaths) { ImGui::BeginDisabled(); }
				int currentPath = m_showAllPaths ? -1 : static_cast<int>(m_currentPath);
				if (ImGui::RadioButton("", &currentPath, static_cast<int>(i)))
				{
					m_currentPath = static_cast<I>(currentPath);
				}
				if (m_showAllPaths) { ImGui::EndDisabled(); }
				ImGui::TableNextColumn();
				ImGui::ColorButton("##color", pathColor(i), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip);
				ImGui::TableNextColumn();
				ImGui::TextColored(m_paths[i].empty() ? themer->sbWarn : themer->sbOk, "[%u/%u]", static_cast<unsigned int>(m_paths[i].sourceEids.size()), static_cast<unsigned int>(m_paths[i].targetEids.size()));
				ImGui::TableNextColumn();
				bool removed{ false };
				if (ImGui::Button("Remove"))
				{
					removePath(i);
					removed = true;
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Clear source", !removed && !m_paths[i].sourceEids.empty()))
				{
					m_paths[i].sourceEids.clear();
				}
				if (!app().targetWidget.hasMesh()) { ImGui::BeginDisabled(); }
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Clear target", !removed && !m_paths[i].targetEids.empty()))
				{
					m_paths[i].targetEids.clear();
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Match source", !removed && !m_paths[i].targetEids.empty()))
				{
					matchPath(i, false);
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Match target", !removed && !m_paths[i].sourceEids.empty()))
				{
					matchPath(i, true);
				}
				if (!app().targetWidget.hasMesh()) { ImGui::EndDisabled(); }
				ImGui::PopID();
			}
			ImGui::EndTable();
			ImGui::PopStyleVar();
			ImGui::Spacing();
			if (m_paths.empty()) { ImGui::BeginDisabled(); }
			if (ImGui::SmallButton("Sort by source length"))
			{
				std::sort(m_paths.begin(), m_paths.end(), [](const EidsPath& _a, const EidsPath& _b) {
					return _a.sourceEids.size() < _b.sourceEids.size();
				});
			}
			if (app().targetWidget.hasMesh())
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Sort by target length"))
				{
					std::sort(m_paths.begin(), m_paths.end(), [](const EidsPath& _a, const EidsPath& _b) {
						return _a.targetEids.size() < _b.targetEids.size();
					});
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Match all source"))
				{
					matchPaths(0, m_paths.size(), false);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Match all target"))
				{
					matchPaths(0, m_paths.size(), true);
				}
			}
			if (m_paths.empty()) { ImGui::EndDisabled(); }
			ImGui::Spacing();
			if (ImGui::TreeNode("Auto finder"))
			{
				ImGui::Spacing();
				ImGui::Checkbox("Use path as crease", &m_usePathAsCrease);
				if (!m_usePathAsCrease)
				{
					ImGui::SliderFloat("Crease angle threshold", &m_featureFinderCreaseAngle, 0, 90, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
				}
				ImGui::Spacing();
				ImGui::Checkbox("Split", &m_featureFinderOptions.split_lines_at_high_curvature_points);
				if (m_featureFinderOptions.split_lines_at_high_curvature_points)
				{
					ImGui::SliderFloat("Split angle threshold", &m_featureFinderOptions.ang_thresh_deg, 0.0f, 180.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
				}
				ImGui::Spacing();
				if (ImGui::Button("Find in source"))
				{
					findPaths(true);
				}
				if (app().targetWidget.hasMesh())
				{
					ImGui::SameLine();
					if (ImGui::Button("Find in target"))
					{
						findPaths(false);
					}
				}
				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::Spacing();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Project"))
		{
			ImGui::Spacing();
			if (app().vertEditWidget.empty())
			{
				ImGui::BeginDisabled();
			}
			ImGui::Checkbox("Lock selected vertices", &m_lockSelectedVertices);
			if (app().vertEditWidget.empty())
			{
				ImGui::EndDisabled();
			}
			Utils::Controls::sliderI("Iterations", m_options.iterations, 1, 100);
			if (app().targetWidget.hasMesh())
			{
				if (ImGui::Button("Project"))
				{
					requestProjection();
				}
				if (canReproject())
				{
					ImGui::SameLine();
					if (ImGui::Button("Reproject"))
					{
						requestReprojection();
					}
				}
			}
			ImGui::Spacing();
			ImGui::TreePop();
		}
	}

	void Projection::drawCanvas()
	{
		const Meshing::Mesher::Mesh& mesh{ app().mesh };
		const cinolib::DrawablePolygonmesh<>& targetMesh{ app().targetWidget.meshForDisplay() };
		const float lineThickness{ this->lineThickness * themer->ovScale };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		const auto drawPath{ [&](const I _pathI) {
			const EidsPath& path { m_paths[_pathI] };
			const ImU32 color{ ImGui::ColorConvertFloat4ToU32(pathColor(_pathI)) };
			for (const Id eid : path.sourceEids)
			{
				const auto eid2d{ Utils::Drawing::project(app().canvas, Meshing::Utils::verts(mesh, Meshing::Utils::eidVids(mesh, eid))) };
				Utils::Drawing::line(drawList, eid2d, color, lineThickness);
			}
			for (const Id eid : path.targetEids)
			{
				const std::vector<Id>& vids{ targetMesh.adj_e2v(eid) };
				const EdgeVerts verts{ cpputils::range::of(vids).map([&](const Id _vid) {
					return targetMesh.transform * targetMesh.vert(_vid);
				}).toArray<2>() };
				const auto eid2d{ Utils::Drawing::project(app().canvas, verts) };
				Utils::Drawing::line(drawList, eid2d, color, lineThickness);
			}
		} };
		if (m_showPaths)
		{
			if (m_paths.empty())
			{
				ImGui::TextColored(Utils::Drawing::toImVec4(themer->ovMut), "No feature path to show");
			}
			else if (m_showAllPaths)
			{
				ImGui::TextColored(Utils::Drawing::toImVec4(themer->ovMut), "Showing all feature paths (%d)", static_cast<int>(m_paths.size()));
				for (I i{}; i < m_paths.size(); i++)
				{
					drawPath(i);
				}
			}
			else
			{
				const EidsPath& path{ m_paths[m_currentPath] };
				ImGui::TextColored(Utils::Drawing::toImVec4(themer->ovMut),
					"Showing feature path %d (%d,%d edges; %c,%c)",
					static_cast<int>(m_currentPath),
					static_cast<int>(path.sourceEids.size()),
					static_cast<int>(path.targetEids.size()),
					HMP::Projection::Utils::isEidsPathClosed(app().mesh, path.sourceEids) ? 'C' : 'O',
					HMP::Projection::Utils::isEidsPathClosed(app().targetWidget.meshForDisplay(), path.targetEids) ? 'C' : 'O'
				);
				drawPath(m_currentPath);
			}
		}
	}

	void Projection::serialize(HMP::Utils::Serialization::Serializer& _serializer) const
	{
		_serializer << m_paths.size();
		for (const EidsPath& path : m_paths)
		{
			{
				const std::vector<Id> vids{ HMP::Projection::Utils::eidsToVidsPath(app().mesh, path.sourceEids) };
				_serializer << vids.size();
				for (const Id vid : vids)
				{
					_serializer << vid;
				}
			}
			{
				const std::vector<Id> vids{ HMP::Projection::Utils::eidsToVidsPath(app().targetWidget.meshForDisplay(), path.targetEids) };
				_serializer << vids.size();
				for (const Id vid : vids)
				{
					_serializer << vid;
				}
			}
		}
	}

	void Projection::deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
	{
		m_paths.clear();
		m_paths.resize(_deserializer.get<I>());
		for (EidsPath& path : m_paths)
		{
			{
				std::vector<Id> vids(_deserializer.get<I>());
				for (Id& vid : vids)
				{
					_deserializer >> vid;
				}
				path.sourceEids = HMP::Projection::Utils::vidsToEidsPath(app().mesh, vids);
			}
			{
				std::vector<Id> vids(_deserializer.get<I>());
				for (Id& vid : vids)
				{
					_deserializer >> vid;
				}
				if (app().targetWidget.hasMesh())
				{
					path.targetEids = HMP::Projection::Utils::vidsToEidsPath(app().targetWidget.meshForDisplay(), vids);
				}
				else
				{
					path.targetEids.clear();
				}
			}
		}
	}

	void Projection::printUsage() const
	{
		cinolib::print_binding(c_kbAddPathEdge.name(), "add path edge");
		cinolib::print_binding(c_kbRemovePathEdge.name(), "remove path edge");
		cinolib::print_binding(c_kbClosePath.name(), "close path");
	}

	void Projection::closePath()
	{
		const bool target{ app().targetWidget.hasMesh() && app().targetWidget.visible };
		if (m_showPaths && !m_showAllPaths && !m_paths.empty())
		{
			std::vector<Id>& eids{ m_paths[m_currentPath].eids(!target) };
			if (!eids.empty())
			{
				std::vector<Id> addedEids{};
				if (target)
				{
					const cinolib::Polygonmesh<>& mesh{app().targetWidget.meshForDisplay()};
					const std::vector<Id> endVids{HMP::Projection::Utils::eidsPathEndVids(mesh, eids)};
					std::vector<bool> mask(toI(mesh.num_edges()), false);
					for (const Id eid : eids)
					{
						mask[toI(eid)] = true;
					}
					std::vector<Id> addedVids{};
					cinolib::dijkstra_mask_on_edges(
						mesh,
						endVids[1],
						endVids[0],
						mask,
						addedVids
					);
					addedEids = HMP::Projection::Utils::vidsToEidsPath(mesh, addedVids);
				}
				else
				{
					const Meshing::Mesher::Mesh& mesh{app().mesh};
					const std::vector<Id> endVids{HMP::Projection::Utils::eidsPathEndVids(mesh, eids)};
					std::vector<bool> mask(toI(mesh.num_edges()), false);
					for (Id eid{}; eid < mesh.num_edges(); eid++)
					{
						mask[toI(eid)] = !mesh.edge_is_visible(eid) || !mesh.edge_is_on_srf(eid);
					}
					for (const Id eid : eids)
					{
						mask[toI(eid)] = true;
					}
					std::vector<Id> addedVids{};
					cinolib::dijkstra_mask_on_edges(
						mesh,
						endVids[1],
						endVids[0],
						mask,
						addedVids
					);
					addedEids = HMP::Projection::Utils::vidsToEidsPath(mesh, addedVids);
				}
				eids.insert(eids.end(), addedEids.begin(), addedEids.end());
			}
		}
	}

	void Projection::addOrRemovePathEdge(bool _add)
	{
		Vec point;
		if (app().canvas.unproject(app().mouse, point))
		{
			if (app().targetWidget.hasMesh() && app().targetWidget.visible)
			{
				setTargetPathEdgeAtPoint(point, _add);
			}
			else
			{
				setSourcePathEdgeAtPoint(point, _add);
			}
		}
	}

	bool Projection::keyPressed(const cinolib::KeyBinding& _key)
	{
		if (_key == c_kbAddPathEdge)
		{
			addOrRemovePathEdge(true);
		}
		else if (_key == c_kbRemovePathEdge)
		{
			addOrRemovePathEdge(false);
		}
		else if (_key == c_kbClosePath)
		{
			closePath();
		}
		else
		{
			return false;
		}
		return true;
	}

}
