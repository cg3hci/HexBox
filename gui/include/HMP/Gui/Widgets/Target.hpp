#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/drawable_polygonmesh.h>
#include <cinolib/meshes/polygonmesh.h>
#include <cinolib/color.h>
#include <cinolib/gl/key_bindings.hpp>
#include <string>
#include <cpputils/collections/Event.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <vector>

namespace HMP::Gui::Widgets
{

	class Target final: public SidebarWidget
	{

	private:

		static constexpr cinolib::KeyBinding c_kbLoad{ GLFW_KEY_L, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbToggleVisibility{ GLFW_KEY_U };

		cinolib::DrawablePolygonmesh<> m_mesh;
		std::string m_filename;
		bool m_missingMeshFile;

		void drawSidebar() override;

		void serialize(HMP::Utils::Serialization::Serializer& _serializer) const override;
		void deserialize(HMP::Utils::Serialization::Deserializer& _deserializer) override;

		bool keyPressed(const cinolib::KeyBinding& _key) override;

		void printUsage() const override;

		std::vector<const cinolib::DrawableObject*> additionalDrawables() const override;

	public:

		cinolib::Color faceColor, edgeColor;
		Utils::Transform transform;
		bool visible;
		float edgeThickness{ 2.0f };

		Target();

		mutable cpputils::collections::Event<Target> onMeshChanged;

		bool hasMesh() const;
		const cinolib::DrawablePolygonmesh<>& meshForDisplay() const;

		cinolib::Polygonmesh<> meshForProjection() const;

		void identity(bool _origin = true, bool _translation = true, bool _rotation = true, bool _scale = true);
		void fit(bool _origin = true, bool _translation = true, bool _scale = true);

		void updateTransform();
		void updateVisibility();
		void updateColor(bool _face = true, bool _edge = true);
		void updateEdgeThickness();

		bool load(bool _keepTransform = false);
		void load(const std::string& _filename, bool _keepTransform = false);

		void clearMesh();
		void requestApplyTransformToSource();

	};

}