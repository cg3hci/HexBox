#include <HMP/Gui/Widgets/Target.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/FilePicking.hpp>
#include <vector>
#include <filesystem>
#include <string>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Actions/Transform.hpp>

namespace HMP::Gui::Widgets
{

	Target::Target():
		SidebarWidget{ "Target mesh" },
		m_mesh{},
		m_missingMeshFile{ false },
		faceColor{ themer->tgtFace }, edgeColor{ themer->tgtEdge },
		transform{},
		visible{ true }
	{
		themer.onThemeChange += [this]() {
			faceColor = themer->tgtFace;
			edgeColor = themer->tgtEdge;
			updateColor();
			updateEdgeThickness();
		};
	}

	bool Target::hasMesh() const
	{
		return m_mesh.num_verts();
	}

	const cinolib::DrawablePolygonmesh<>& Target::meshForDisplay() const
	{
		return m_mesh;
	}

	cinolib::Polygonmesh<> Target::meshForProjection() const
	{
		assert(hasMesh());
		cinolib::Polygonmesh<> mesh{ m_mesh };
		for (Id vid{}; vid < mesh.num_verts(); vid++)
		{
			Vec& vert{ mesh.vert(vid) };
			vert = m_mesh.transform * vert;
		}
		return mesh;
	}

	void Target::identity(bool _origin, bool _translation, bool _rotation, bool _scale)
	{
		if (_origin)
		{
			transform.origin = Vec{};
		}
		if (_translation)
		{
			transform.translation = Vec{};
		}
		if (_rotation)
		{
			transform.rotation = Vec{};
		}
		if (_scale)
		{
			transform.scale = { 1.0 };
		}
		if (hasMesh())
		{
			updateTransform();
		}
	}

	void Target::fit(bool _origin, bool _translation, bool _scale)
	{
		assert(hasMesh());
		if (_origin)
		{
			transform.origin = m_mesh.bbox().center();
		}
		if (_translation)
		{
			transform.translation = -transform.origin + app().mesh.bbox().center();
		}
		if (_scale)
		{
			transform.scale = { app().mesh.bbox().diag() / m_mesh.bbox().diag() };
		}
		updateTransform();
	}

	void Target::updateTransform()
	{
		assert(hasMesh());
		m_mesh.transform = transform.matrix();
		app().refitScene();
	}

	void Target::updateVisibility()
	{
		assert(hasMesh());
		m_mesh.show_mesh(visible);
	}

	void Target::updateEdgeThickness()
	{
		const float edgeThickness{ this->edgeThickness * themer->ovScale };
		m_mesh.show_wireframe_width(edgeThickness);
	}

	void Target::updateColor(bool _face, bool _edge)
	{
		if (!hasMesh())
		{
			return;
		}
		if (_face && m_mesh.poly_data(0).color != faceColor)
		{
			m_mesh.poly_set_color(faceColor);
		}
		if (_edge && m_mesh.edge_data(0).color != edgeColor)
		{
			m_mesh.edge_set_color(edgeColor);
		}
	}

	bool Target::load(bool _keepTransform)
	{
		const std::optional<std::string> filename{ Utils::FilePicking::open() };
		if (filename)
		{
			visible = true;
			load(*filename, _keepTransform);
			return true;
		}
		return false;
	}

	std::vector<const cinolib::DrawableObject*> Target::additionalDrawables() const
	{
		return { &m_mesh };
	}

	void Target::load(const std::string& _filename, bool _keepTransform)
	{
		clearMesh();
		m_filename = _filename;
		if (std::filesystem::exists(_filename))
		{
			visible = true;
			m_mesh = cinolib::DrawablePolygonmesh<>{ m_filename.c_str() };
			m_mesh.show_marked_edge(false);
			m_mesh.draw_back_faces = false;
			updateColor();
			updateEdgeThickness();
			updateVisibility();
			if (!_keepTransform)
			{
				transform = {};
				fit();
			}
			updateTransform();
			app().refitScene();
			onMeshChanged();
		}
		else
		{
			m_missingMeshFile = true;
		}
	}

	void Target::clearMesh()
	{
		m_missingMeshFile = false;
		m_filename = "";
		m_mesh.clear();
		m_mesh.updateGL();
		app().refitScene();
		onMeshChanged();
	}

	void Target::requestApplyTransformToSource()
	{
		assert(hasMesh());
		updateTransform();
		app().applyAction(*new HMP::Actions::Transform{ m_mesh.transform.inverse() });
		app().resetCamera();
		identity();
	}

	void Target::drawSidebar()
	{
		if (hasMesh())
		{
			{
				ImGui::TextColored(themer->sbOk, "%s", m_filename.c_str());
				ImGui::Spacing();
				if (ImGui::Button("Clear"))
				{
					clearMesh();
					return;
				}
				ImGui::SameLine();
				if (ImGui::Button("Replace"))
				{
					if (!load(true))
					{
						return;
					}
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Visible", &visible))
				{
					updateVisibility();
				}
			}
			ImGui::Spacing();
			{
				ImGui::BeginTable("Transform", 3, ImGuiTableFlags_RowBg);
				ImGui::TableSetupColumn("drag", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("identity", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("fit", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableNextColumn();
				ImGui::Text("Transform");
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##global"))
				{
					identity();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##global"))
				{
					fit();
				}
				ImGui::TableNextColumn();
				const float targetMeshSize{ static_cast<float>(m_mesh.bbox().diag()) * 2 };
				if (Utils::Controls::dragTranslationVec("Origin", transform.origin, targetMeshSize))
				{
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##origin"))
				{
					identity(true, false, false, false);
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##origin"))
				{
					fit(true, false, false);
				}
				ImGui::TableNextColumn();
				const float sourceMeshSize{ static_cast<float>(app().mesh.bbox().diag()) * 2 };
				if (Utils::Controls::dragTranslationVec("Translation", transform.translation, sourceMeshSize))
				{
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##translation"))
				{
					identity(false, true, false, false);
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##translation"))
				{
					fit(false, true, false);
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::dragRotation("Rotation", transform.rotation))
				{
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##rotation"))
				{
					identity(false, false, true, false);
				}
				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
				const Real sourceAndTargetMeshScaleRatio{ app().mesh.bbox().diag() / m_mesh.bbox().diag() * 3 };
				Real scale{ transform.avgScale() };
				if (Utils::Controls::dragScale("Scale##scale", scale, sourceAndTargetMeshScaleRatio))
				{
					transform.scale = { scale };
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##scale"))
				{
					identity(false, false, false, true);
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##scale"))
				{
					fit(false, false, true);
				}
				ImGui::EndTable();
			}
			ImGui::Spacing();
			{
				ImGui::BeginTable("Color", 2, ImGuiTableFlags_RowBg);
				ImGui::TableSetupColumn("pick", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("apply", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableNextColumn();
				Utils::Controls::colorButton("Face color", faceColor);
				ImGui::TableNextColumn();
				if (ImGui::Button("Apply##facecol"))
				{
					updateColor(true, false);
				}
				ImGui::TableNextColumn();
				Utils::Controls::colorButton("Edge color", edgeColor);
				ImGui::TableNextColumn();
				if (ImGui::Button("Apply##edgecol"))
				{
					updateColor(false, true);
				}
				ImGui::TableNextColumn();
				ImGui::SliderFloat("Edge thickness", &edgeThickness, 1.0f, 5.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::TableNextColumn();
				if (ImGui::Button("Apply##edgethickness"))
				{
					updateEdgeThickness();
				}
				ImGui::EndTable();
			}
			ImGui::Spacing();
			if (ImGui::Button("Apply transform to source"))
			{
				requestApplyTransformToSource();
			}
		}
		else
		{
			if (m_missingMeshFile)
			{
				ImGui::TextColored(themer->sbErr, "Missing mesh file");
				ImGui::TextColored(themer->sbWarn, "%s", m_filename.c_str());
				ImGui::Spacing();
				if (ImGui::Button("Clear"))
				{
					clearMesh();
				}
				ImGui::SameLine();
				if (ImGui::Button("Replace"))
				{
					load(true);
				}
			}
			if (ImGui::Button("Load"))
			{
				load();
			}
		}
	}

	void Target::serialize(HMP::Utils::Serialization::Serializer& _serializer) const
	{
		_serializer << hasMesh();
		if (hasMesh())
		{
			_serializer << m_filename;
			_serializer << transform.origin;
			_serializer << transform.translation;
			_serializer << transform.scale;
			_serializer << transform.rotation;
		}
	}

	void Target::deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
	{
		clearMesh();
		if (_deserializer.get<bool>())
		{
			const std::string filename{ _deserializer.get<std::string>() };
			_deserializer >> transform.origin;
			_deserializer >> transform.translation;
			_deserializer >> transform.scale;
			_deserializer >> transform.rotation;
			load(filename, true);
		}
	}

	bool Target::keyPressed(const cinolib::KeyBinding& _key)
	{
		if (_key == c_kbLoad)
		{
			load();
		}
		else if (_key == c_kbToggleVisibility)
		{
			visible ^= true;
			updateVisibility();
		}
		else
		{
			return false;
		}
		return true;
	}

	void Target::printUsage() const
	{
		cinolib::print_binding(c_kbLoad.name(), "load target mesh");
		cinolib::print_binding(c_kbToggleVisibility.name(), "toggle target visibility");
	}

}
