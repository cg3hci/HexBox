#include <HMP/Gui/Widgets/VertEdit.hpp>

#include <imgui.h>
#include <cassert>
#include <algorithm>
#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Actions/Transform.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/index.hpp>

namespace HMP::Gui::Widgets
{

	bool VertEdit::addOrRemove(const Id* _vids, I _count, bool _add, bool _update)
	{
		bool changed{ false };
		for (const Id* vidp{ _vids }; vidp < _vids + _count; vidp++)
		{
			const Id vid{ *vidp };
			const auto it{ m_verts.find(vid) };
			if ((it == m_verts.end()) == _add)
			{
				if (!changed)
				{
					changed = true;
					applyAction();
				}
				if (_add)
				{
					const Vec pos{ app().mesh.vert(vid) };
					m_verts.insert(it, { vid, pos });
				}
				else
				{
					m_verts.erase(it);
				}
			}
		}
		if (changed && _update)
		{
			onVidsChanged();
			updateCentroid();
		}
		return changed;
	}

	VertEdit::VertEdit() :
		SidebarWidget{ "Vertex editor" },
		m_verts{}, m_pendingAction{ false },
		m_unappliedTransform{}, m_appliedTransform{}, m_centroid{}
	{}

	bool VertEdit::add(Id _vid, bool _update)
	{
		return addOrRemove(&_vid, 1, true, _update);
	}

	bool VertEdit::remove(Id _vid, bool _update)
	{
		return addOrRemove(&_vid, 1, false, _update);
	}

	bool VertEdit::add(const std::vector<Id>& _vids, bool _update)
	{
		if (!_vids.empty())
		{
			return addOrRemove(&_vids[0], _vids.size(), true, _update);
		}
		return false;
	}

	bool VertEdit::remove(const std::vector<Id>& _vids, bool _update)
	{
		if (!_vids.empty())
		{
			return addOrRemove(&_vids[0], _vids.size(), false, _update);
		}
		return false;
	}

	bool VertEdit::intersect(const std::vector<Id>& _vids, bool _update)
	{
		if (!_vids.empty())
		{
			std::vector<Id> vids;
			for (const Id vid : _vids)
			{
				if (has(vid))
				{
					vids.push_back(vid);
				}
			}
			clear();
			return addOrRemove(&vids[0], vids.size(), true, _update);
		}
		return false;
	}

	bool VertEdit::has(Id _vid) const
	{
		return m_verts.contains(_vid);
	}

	VertEdit::Vids VertEdit::vids() const
	{
		return cpputils::range::ofc(m_verts).map(&vertsToVidsConvert);
	}

	void VertEdit::clear()
	{
		applyAction();
		m_verts.clear();
		onVidsChanged();
	}

	bool VertEdit::empty() const
	{
		return m_verts.empty();
	}

	const Vec& VertEdit::centroid() const
	{
		assert(!empty());
		return m_centroid;
	}

	bool VertEdit::pendingAction() const
	{
		return m_pendingAction;
	}

	void VertEdit::applyAction()
	{
		if (m_pendingAction)
		{
			const Mat4 transform{ m_appliedTransform.matrix() };
			std::vector<Id> vids{};
			vids.reserve(m_verts.size());
			for (const auto& [vid, pos] : m_verts)
			{
				vids.push_back(vid);
			}
			cancel();
			app().applyAction(*new HMP::Actions::Transform{ transform, vids });
			for (auto& [vid, pos] : m_verts)
			{
				pos = app().mesh.vert(vid);
			}
			updateCentroid();
		}
	}

	Utils::Transform& VertEdit::transform()
	{
		return m_unappliedTransform;
	}

	const Utils::Transform& VertEdit::transform() const
	{
		return m_unappliedTransform;
	}

	void VertEdit::applyTransform()
	{
		const Mat4 transform{ m_unappliedTransform.matrix() };
		std::unordered_set<Id> vids{};
		vids.reserve(m_verts.size());
		for (const auto& [vid, pos] : m_verts)
		{
			app().mesher.moveVert(vid, transform * pos);
			vids.insert(vid);
		}
		m_appliedTransform = m_unappliedTransform;
		app().mesher.updateMeshTemp(vids);
		updateCentroid();
		const bool hadPendingAction{ m_pendingAction };
		m_pendingAction = !m_appliedTransform.isIdentity();
		if (m_pendingAction && !hadPendingAction)
		{
			app().commander.unapplied().clear();
		}
	}

	void VertEdit::cancel()
	{
		m_unappliedTransform = {};
		applyTransform();
	}

	void VertEdit::updateCentroid()
	{
		m_centroid = m_unappliedTransform.origin = Vec{};
		if (!empty())
		{
			for (const auto& [vid, pos] : m_verts)
			{
				m_unappliedTransform.origin += pos;
				m_centroid += app().mesh.vert(vid);
			}
			m_centroid /= static_cast<Real>(m_verts.size());
			m_unappliedTransform.origin /= static_cast<Real>(m_verts.size());
		}
	}

	void VertEdit::attached()
	{
		app().mesher.onRestored += [this](const Meshing::Mesher::State&)
		{
			remove(vids().filter([&](const Id _vid)
				{
					return _vid >= app().mesh.num_verts();
				}).toVector());
		};
		app().mesher.onElementVisibilityChanged += [this](const Dag::Element& _element, bool _visible)
		{
			if (!_visible)
			{
				remove(app().mesh.poly_dangling_vids(_element.pid));
			}
		};
	}

	void VertEdit::actionApplied()
	{
		updateCentroid();
		const std::vector<Id> vids{ this->vids().filter([&](const Id _vid) { return app().mesher.vidShown(_vid); }).toVector() };
		clear();
		add(vids);
	}

	void VertEdit::actionPrepared()
	{
		applyAction();
	}

	void VertEdit::drawSidebar()
	{
		ImGui::TextColored(m_verts.empty() ? themer->sbWarn : themer->sbOk, "%d vertices selected", static_cast<int>(m_verts.size()));
		if (empty())
		{
			return;
		}
		ImGui::Spacing();
		ImGui::BeginTable("Transform", 2, ImGuiTableFlags_RowBg);
		ImGui::TableSetupColumn("drag", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("button", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableNextColumn();
		ImGui::Text("Transform");
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##all"))
		{
			cancel();
		}
		// translation
		ImGui::TableNextColumn();
		if (Utils::Controls::dragTranslationVec("Translation", m_unappliedTransform.translation, app().mesh.scene_radius()))
		{
			applyTransform();
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##translation"))
		{
			m_unappliedTransform.translation = Vec{};
			applyTransform();
		}
		// scale
		ImGui::TableNextColumn();
		if (Utils::Controls::dragScaleVec("Scale", m_unappliedTransform.scale))
		{
			applyTransform();
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##scale"))
		{
			m_unappliedTransform.scale = { 1.0 };
			applyTransform();
		}
		// rotation
		ImGui::TableNextColumn();
		if (Utils::Controls::dragRotation("Rotation", m_unappliedTransform.rotation))
		{
			applyTransform();
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##rotation"))
		{
			m_unappliedTransform.rotation = Vec{};
			applyTransform();
		}
		ImGui::EndTable();
	}

	bool VertEdit::mouseMoved(const Vec2&)
	{
		return m_boxSelectionStart.has_value();
	}

	void VertEdit::cameraChanged()
	{
		m_boxSelectionStart = std::nullopt;
	}

	void VertEdit::drawCanvas()
	{
		const float
			radius{ this->radius * themer->ovScale },
			lineThickness{ this->lineThickness * themer->ovScale },
			lineSpacing{ (10.0f * themer->ovScale + 10.0f) / 2.0f };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		if (m_boxSelectionStart)
		{
			Utils::Drawing::dashedRect(drawList, Utils::Controls::toImVec2(*m_boxSelectionStart), Utils::Controls::toImVec2(app().mouse), themer->ovMut, lineThickness, lineSpacing);
		}
		for (const Id vid : vids())
		{
			const Vec vert{ app().mesh.vert(vid) };
			const auto pos{ Utils::Drawing::project(app().canvas, vert) };
			Utils::Drawing::circleFilled(drawList, pos, radius, themer->ovHi, 4);
		}
		if (!empty())
		{
			Utils::Drawing::cross(drawList, Utils::Drawing::project(app().canvas, m_centroid), radius * 2.0f, themer->ovHi, lineThickness);
			const char* verticesLit{ m_verts.size() == 1 ? "vertex" : "vertices" };
			const int vertexCount{ static_cast<int>(m_verts.size()) };
			ImGui::TextColored(Utils::Drawing::toImVec4(themer->ovMut), "%d %s selected", vertexCount, verticesLit);
		}
	}

	bool VertEdit::keyPressed(const cinolib::KeyBinding& _key)
	{
		if (_key == c_kbSelectVertex)
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectEdge)
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectUpEdge)
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectFace)
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectUpFace)
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectPoly)
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Set);
		}
		else if (_key == (c_kbSelectVertex | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectEdge | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectUpEdge | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectUpFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectPoly | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectVertex | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectEdge | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectUpEdge | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectUpFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectPoly | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectVertex | c_kmodSelectIntersect))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Intersect);
		}
		else if (_key == (c_kbSelectEdge | c_kmodSelectIntersect))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Intersect);
		}
		else if (_key == (c_kbSelectUpEdge | c_kmodSelectIntersect))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Intersect);
		}
		else if (_key == (c_kbSelectFace | c_kmodSelectIntersect))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Intersect);
		}
		else if (_key == (c_kbSelectUpFace | c_kmodSelectIntersect))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Intersect);
		}
		else if (_key == (c_kbSelectPoly | c_kmodSelectIntersect))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Intersect);
		}
		else if (_key == c_kbDeselectAll)
		{
			onSelectAll(false);
		}
		else if (_key == c_kbSelectAll)
		{
			onSelectAll(true);
		}
		else if (_key == c_kbSelectBox)
		{
			onBoxSelect(ESelectionMode::Set);
		}
		else if (_key == (c_kbSelectBox | c_kmodSelectAdd))
		{
			onBoxSelect(ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectBox | c_kmodSelectRemove))
		{
			onBoxSelect(ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectBox | c_kmodSelectIntersect))
		{
			onBoxSelect(ESelectionMode::Intersect);
		}
		else if (_key == c_kbInvertSelection)
		{
			std::vector<Id> vids;
			vids.reserve(toI(app().mesh.num_verts()));
			for (Id vid{}; vid < app().mesh.num_verts(); vid++)
			{
				if (app().mesher.vidShown(vid) && !has(vid))
				{
					vids.push_back(vid);
				}
			}
			app().vertEditWidget.clear();
			app().vertEditWidget.add(vids);
		}
		else
		{
			return false;
		}
		return true;
	}

	void VertEdit::onBoxSelect(ESelectionMode _mode)
	{
		if (!m_boxSelectionStart)
		{
			m_boxSelectionStart = app().mouse;
			return;
		}
		const Vec2
			min{
				std::min(app().mouse.x(), m_boxSelectionStart->x()),
				std::min(app().mouse.y(), m_boxSelectionStart->y())
		},
			max{
				std::max(app().mouse.x(), m_boxSelectionStart->x()),
				std::max(app().mouse.y(), m_boxSelectionStart->y())
		};
		m_boxSelectionStart = std::nullopt;
		std::vector<Id> vids{};
		for (Id vid{}; vid < app().mesh.num_verts(); ++vid)
		{
			if (app().mesher.vidShown(vid))
			{
				Vec2 vert2d;
				GLdouble depth;
				app().canvas.project(app().mesh.vert(vid), vert2d, depth);
				if (depth >= 0.0
					&& depth <= 1.0
					&& vert2d.x() >= min.x()
					&& vert2d.y() >= min.y()
					&& vert2d.x() <= max.x()
					&& vert2d.y() <= max.y())
				{
					vids.push_back(vid);
				}
			}
		}
		switch (_mode)
		{
			case ESelectionMode::Add:
				add(vids);
				break;
			case ESelectionMode::Set:
				clear();
				add(vids);
				break;
			case ESelectionMode::Remove:
				remove(vids);
				break;
			case ESelectionMode::Intersect:
				intersect(vids);
				break;
		}
	}

	void VertEdit::onSelect(ESelectionSource _source, ESelectionMode _mode)
	{
		const App::Cursor& curs{ app().cursor };
		if (curs.element)
		{
			std::vector<Id> vids{};
			switch (_source)
			{
				case ESelectionSource::Vertex:
					vids = { curs.vid };
					break;
				case ESelectionSource::Edge:
					vids = app().mesh.edge_vert_ids(curs.eid);
					break;
				case ESelectionSource::Face:
					vids = app().mesh.face_verts_id(curs.fid);
					break;
				case ESelectionSource::UpFace:
					vids = app().mesh.face_verts_id(Meshing::Utils::adjFidInPidByFidAndEid(app().mesh, curs.pid, curs.fid, curs.eid));
					break;
				case ESelectionSource::UpEdge:
					vids = { curs.vid, app().mesh.poly_vert_opposite_to(curs.pid, curs.fid, curs.vid) };
					break;
				case ESelectionSource::Poly:
					vids = app().mesh.poly_verts_id(curs.pid);
					break;
			}
			switch (_mode)
			{
				case ESelectionMode::Add:
					add(vids);
					break;
				case ESelectionMode::Set:
					clear();
					add(vids);
					break;
				case ESelectionMode::Remove:
					remove(vids);
					break;
				case ESelectionMode::Intersect:
					intersect(vids);
					break;
			}
		}
	}

	void VertEdit::onSelectAll(bool _selected)
	{
		if (_selected)
		{
			std::vector<Id> vids;
			vids.reserve(toI(app().mesh.num_verts()));
			for (Id vid{}; vid < app().mesh.num_verts(); vid++)
			{
				if (app().mesher.vidShown(vid))
				{
					vids.push_back(vid);
				}
			}
			add(vids);
		}
		else
		{
			clear();
		}
	}

	void VertEdit::printUsage() const
	{
		cinolib::print_binding(c_kbSelectVertex.name(), "select vertex");
		cinolib::print_binding(c_kbSelectEdge.name(), "select edge vertices");
		cinolib::print_binding(c_kbSelectUpEdge.name(), "select adjacent edge vertices");
		cinolib::print_binding(c_kbSelectFace.name(), "select face vertices");
		cinolib::print_binding(c_kbSelectUpFace.name(), "select adjacent face vertices");
		cinolib::print_binding(c_kbSelectPoly.name(), "select poly vertices");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectAdd), "remove from selection (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectRemove), "add to selection (hold down)");
		cinolib::print_binding(c_kbSelectAll.name(), "select all vertices");
		cinolib::print_binding(c_kbDeselectAll.name(), "deselect all vertices");
		cinolib::print_binding(c_kbInvertSelection.name(), "invert selection");
	}

}