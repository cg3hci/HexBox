#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/drawable_hexmesh.h>
#include <unordered_map>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <utility>
#include <functional>
#include <unordered_set>
#include <vector>
#include <cinolib/octree.h>

namespace HMP::Meshing
{

	class Mesher final : public cpputils::mixins::ReferenceClass
	{

	public:

		class PolyAttributes final : public cinolib::Polyhedron_std_attributes
		{

		private:

			friend class Mesher;

			Dag::Element* m_element{};

		};

		class State final
		{

		private:

			friend class Mesher;

			Id m_pids, m_fids, m_eids, m_vids;

			State(Id _pids, Id _fids, Id _eids, Id _vids);

		public:

			State();

			Id pidsCount() const;
			Id fidsCount() const;
			Id eidsCount() const;
			Id vidsCount() const;
			Id lastPid() const;
			Id lastFid() const;
			Id lastEid() const;
			Id lastVid() const;

			bool operator==(const State& _other) const = default;

		};

		using Mesh = cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, PolyAttributes>;

	private:

		static constexpr Real c_maxVertDistance{ 1e-3 };

		Mesh m_mesh;
		cinolib::Octree m_octree;
		std::vector<Id> m_fidVisibleTriId, m_eidVisibleEid;
		bool m_dirty;

		void updateOctree();

	public:

		cinolib::Color faceColor, edgeColor;

		Mesher();

		mutable cpputils::collections::Event<Mesher, State> onAdded;
		mutable cpputils::collections::Event<Mesher, State> onRestored;
		mutable cpputils::collections::Event<Mesher, const Dag::Element&, bool> onElementVisibilityChanged;
		mutable cpputils::collections::Event<Mesher> onUpdated;

		const Mesh& mesh() const;

		bool has(const Dag::Element& _element) const;
		Dag::Element& element(Id _pid);
		const Dag::Element& element(Id _pid) const;
		void moveVert(Id _vid, const Vec& _position);
		void add(const std::vector<Dag::Element*> _elements, const std::vector<Vec>& _verts = {});
		void restore(const State& _state);
		State state() const;
		void show(Id _pid, bool _visible);
		void show(Dag::Element& _element, bool _visible);
		bool shown(Id _pid) const;
		bool vidShown(Id _vid) const;
		bool shown(const Dag::Element& _element) const;

		void updateColors(bool _poly = true, bool _edge = true);
		void setEdgeThickness(float _edgeThickness);
		float edgeThickness() const;

		void updateMesh();
		void updateMeshTemp(const std::unordered_set<Id>& _changedVids);

		bool pick(const Vec& _from, const Vec& _dir, Id& _pid, Id& _fid, Id& _eid, Id& _vid, bool _allowBehind = false) const;

	};

}