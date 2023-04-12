#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/key_bindings.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <cpputils/collections/Event.hpp>
#include <cpputils/range/of.hpp>
#include <unordered_map>
#include <vector>
#include <optional>

namespace HMP::Gui::Widgets
{

	class VertEdit final : public SidebarWidget
	{

	private:

		static constexpr cinolib::KeyBinding c_kbSelectVertex{ GLFW_KEY_1 };
		static constexpr cinolib::KeyBinding c_kbSelectEdge{ GLFW_KEY_2 };
		static constexpr cinolib::KeyBinding c_kbSelectUpEdge{ GLFW_KEY_2, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectFace{ GLFW_KEY_3 };
		static constexpr cinolib::KeyBinding c_kbSelectUpFace{ GLFW_KEY_3, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectPoly{ GLFW_KEY_4 };
		static constexpr cinolib::KeyBinding c_kbDeselectAll{ GLFW_KEY_A, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbInvertSelection{ GLFW_KEY_A, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectAll{ GLFW_KEY_A, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbSelectBox{ GLFW_KEY_K };
		static constexpr int c_kmodSelectAdd{ GLFW_MOD_SHIFT };
		static constexpr int c_kmodSelectRemove{ GLFW_MOD_CONTROL };
		static constexpr int c_kmodSelectIntersect{ GLFW_MOD_ALT };

		enum class ESelectionSource
		{
			Vertex, Edge, Face, Poly, UpFace, UpEdge
		};

		enum class ESelectionMode
		{
			Add, Remove, Set, Intersect
		};

		static constexpr Id vertsToVidsConvert(const std::unordered_map<Id, Vec>::value_type& _entry)
		{
			return _entry.first;
		}

		std::unordered_map<Id, Vec> m_verts;
		bool m_pendingAction;
		Utils::Transform m_unappliedTransform, m_appliedTransform;
		Vec m_centroid;
		std::optional<Vec2> m_boxSelectionStart{ std::nullopt };

		bool addOrRemove(const Id* _vids, I _count, bool _add, bool _update);

		bool keyPressed(const cinolib::KeyBinding& _key) override;

		void printUsage() const override;

		void actionApplied() override;

		void actionPrepared() override;

		void onSelect(ESelectionSource _source, ESelectionMode _mode);

		void onSelectAll(bool _selected);

		void onBoxSelect(ESelectionMode _mode);

		void attached() override;

		bool mouseMoved(const Vec2&) override;

		void cameraChanged() override;

	public:

		float radius{ 3.0f }, lineThickness{ 1.5f };

		using Vids = decltype(cpputils::range::ofc(m_verts).map(&vertsToVidsConvert));

		cpputils::collections::Event<VertEdit> onVidsChanged{};

		VertEdit();

		bool add(Id _vid, bool _update = true);

		bool remove(Id _vid, bool _update = true);

		bool add(const std::vector<Id>& _vids, bool _update = true);

		bool remove(const std::vector<Id>& _vids, bool _update = true);

		bool intersect(const std::vector<Id>& _vids, bool _update = true);

		bool has(Id _vid) const;

		Vids vids() const;

		void clear();

		bool empty() const;

		const Vec& centroid() const;

		Utils::Transform& transform();

		const Utils::Transform& transform() const;

		void applyTransform();

		bool pendingAction() const;

		void cancel();

		void applyAction();

		void updateCentroid();

		void drawSidebar() override;

		void drawCanvas() override;

	};

}