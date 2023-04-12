#include <HMP/Meshing/Mesher.hpp>

#include <cpputils/ensure.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cinolib/Moller_Trumbore_intersection.h>
#include <limits>
#include <cinolib/octree.h>
#include <queue>
#include <cassert>

namespace HMP::Meshing
{

	// Mesher

	Mesher::Mesher()
		: m_mesh(), m_octree{},
		m_fidVisibleTriId{}, m_eidVisibleEid{},
		m_dirty{ false },
		faceColor{ cinolib::Color::WHITE() }, edgeColor{ cinolib::Color::BLACK() }
	{
		m_mesh.draw_back_faces = false;
		m_mesh.show_mesh(true);
		m_mesh.show_mesh_flat();
		m_mesh.show_marked_face(true);
		setEdgeThickness(2.0f);
		updateColors();
	}

	const Mesher::Mesh& Mesher::mesh() const
	{
		return m_mesh;
	}

	Dag::Element& Mesher::element(Id _pid)
	{
		return *m_mesh.poly_data(_pid).m_element;
	}

	const Dag::Element& Mesher::element(Id _pid) const
	{
		return const_cast<Mesher*>(this)->element(_pid);
	}

	void Mesher::moveVert(Id _vid, const Vec& _position)
	{
		if (m_mesh.vert(_vid) != _position)
		{
			m_mesh.vert(_vid) = _position;
			m_dirty = true;
		}
	}

	void Mesher::updateColors(bool _poly, bool _edge)
	{
		if (_poly)
		{
			m_mesh.poly_set_color(faceColor);
		}
		if (_edge)
		{
			m_mesh.edge_set_color(edgeColor);
		}
	}

	void Mesher::updateMesh()
	{
		if (m_dirty)
		{
			updateOctree();
			m_fidVisibleTriId.resize(toI(m_mesh.num_faces()));
			m_eidVisibleEid.resize(toI(m_mesh.num_edges()));
			std::fill(m_fidVisibleTriId.begin(), m_fidVisibleTriId.end(), noId);
			std::fill(m_eidVisibleEid.begin(), m_eidVisibleEid.end(), noId);
			m_mesh.update_normals();
			m_mesh.update_bbox();
			m_mesh.updateGL_out(m_fidVisibleTriId, m_eidVisibleEid);
			m_mesh.updateGL_in(m_fidVisibleTriId, m_eidVisibleEid);
			m_dirty = false;
			onUpdated();
		}
	}

	void Mesher::updateMeshTemp(const std::unordered_set<Id>& _changedVids)
	{
		if (m_dirty)
		{
			static constexpr double c_threshold{ 0.5 };
			if (_changedVids.size() >= static_cast<I>(static_cast<double>(m_mesh.num_polys()) * c_threshold))
			{
				m_mesh.update_normals();
				m_mesh.updateGL_out(m_fidVisibleTriId, m_eidVisibleEid);
				m_mesh.updateGL_in(m_fidVisibleTriId, m_eidVisibleEid);
			}
			else
			{
				std::unordered_set<Id> changedFids{}, changedEids{};
				for (const Id vid : _changedVids)
				{
					const std::vector<Id>& fids{ m_mesh.adj_v2f(vid) };
					changedFids.insert(fids.begin(), fids.end());
					const std::vector<Id>& eids{ m_mesh.adj_v2e(vid) };
					changedEids.insert(eids.begin(), eids.end());
				}
				for (const Id fid : changedFids)
				{
					m_mesh.update_f_normal(fid);
				}
				for (const Id vid : _changedVids)
				{
					m_mesh.update_v_normal(vid);
				}
				for (const Id fid : changedFids)
				{
					const Id visibleTriId{ m_fidVisibleTriId[toI(fid)] };
					if (visibleTriId != noId)
					{
						if (m_mesh.face_is_on_srf(fid))
						{
							m_mesh.updateGL_out_f(fid, visibleTriId);
						}
						else
						{
							m_mesh.updateGL_in_f(fid, visibleTriId);
						}
					}
				}
				for (const Id eid : changedEids)
				{
					const Id visibleEid{ m_eidVisibleEid[toI(eid)] };
					if (visibleEid != noId)
					{
						if (m_mesh.edge_is_on_srf(eid))
						{
							m_mesh.updateGL_out_e(eid, visibleEid);
						}
						else
						{
							m_mesh.updateGL_in_e(eid, visibleEid);
						}
					}
				}
			}
		}
	}

	Mesher::State::State(Id _pids, Id _fids, Id _eids, Id _vids)
		: m_pids{ _pids }, m_fids{ _fids }, m_eids{ _eids }, m_vids{ _vids }
	{}

	Mesher::State::State() : State{ 0,0,0,0 } {}

	Id Mesher::State::pidsCount() const
	{
		return m_pids;
	}

	Id Mesher::State::fidsCount() const
	{
		return m_fids;
	}

	Id Mesher::State::eidsCount() const
	{
		return m_eids;
	}

	Id Mesher::State::vidsCount() const
	{
		return m_vids;
	}

	Id Mesher::State::lastPid() const
	{
		return m_pids - 1;
	}

	Id Mesher::State::lastFid() const
	{
		return m_fids - 1;
	}

	Id Mesher::State::lastEid() const
	{
		return m_eids - 1;
	}

	Id Mesher::State::lastVid() const
	{
		return m_vids - 1;
	}

	void Mesher::add(const std::vector<Dag::Element*> _elements, const std::vector<Vec>& _verts)
	{
		if (!_elements.empty() || !_verts.empty())
		{
			const State oldState{ state() };
			for (const Vec& vert : _verts)
			{
				m_mesh.vert_add(vert);
			}
			for (Dag::Element* element : _elements)
			{
				assert(element->pid == noId || element->pid == m_mesh.num_polys());
				element->pid = m_mesh.num_polys();
				const Id pid{ m_mesh.poly_add(cpputils::range::of(element->vids).toVector()) };
				m_mesh.poly_data(pid).m_element = element;
				m_mesh.poly_data(pid).color = faceColor;
				for (const Id eid : m_mesh.adj_p2e(pid))
				{
					m_mesh.edge_data(eid).color = edgeColor;
				}
				m_dirty = true;
			}
			m_dirty = true;
			onAdded(oldState);
		}
	}

	void Mesher::restore(const State& _state)
	{
		const State oldState{ state() };
		if (_state != oldState)
		{
			if (_state == State{})
			{
				m_mesh.clear();
			}
			else
			{
				for (Id pidsCount{ m_mesh.num_polys() }; pidsCount > _state.pidsCount(); --pidsCount)
				{
					m_mesh.poly_remove(pidsCount - 1, false);
				}
				for (Id fidsCount{ m_mesh.num_faces() }; fidsCount > _state.fidsCount(); --fidsCount)
				{
					m_mesh.face_remove_unreferenced(fidsCount - 1);
				}
				for (Id eidsCount{ m_mesh.num_edges() }; eidsCount > _state.eidsCount(); --eidsCount)
				{
					m_mesh.edge_remove_unreferenced(eidsCount - 1);
				}
				for (Id vidsCount{ m_mesh.num_verts() }; vidsCount > _state.vidsCount(); --vidsCount)
				{
					m_mesh.vert_remove_unreferenced(vidsCount - 1);
				}
			}
			assert(state() == _state);
			m_dirty = true;
			onRestored(oldState);
		}
	}

	Mesher::State Mesher::state() const
	{
		return State{ m_mesh.num_polys(), m_mesh.num_faces(), m_mesh.num_edges(), m_mesh.num_verts() };
	}

	void Mesher::show(Id _pid, bool _visible)
	{
		auto data{ m_mesh.poly_data(_pid).flags[cinolib::HIDDEN] };
		if (data != !_visible)
		{
			data = !_visible;
			m_dirty = true;
			onElementVisibilityChanged(element(_pid), _visible);
		}
	}

	void Mesher::show(Dag::Element& _element, bool _visible)
	{
		show(_element.pid, _visible);
	}

	bool Mesher::shown(Id _pid) const
	{
		return !m_mesh.poly_data(_pid).flags[cinolib::HIDDEN];
	}

	bool Mesher::vidShown(Id _vid) const
	{
		for (const Id pid : m_mesh.adj_v2p(_vid))
		{
			if (shown(pid))
			{
				return true;
			}
		}
		return false;
	}

	bool Mesher::shown(const Dag::Element& _element) const
	{
		return shown(_element.pid);
	}

	bool Mesher::pick(const Vec& _from, const Vec& _normDir, Id& _pid, Id& _fid, Id& _eid, Id& _vid, bool _allowBehind) const
	{
		double minT{ std::numeric_limits<double>::infinity() };
		_pid = _fid = _eid = _vid = noId;
		if (_allowBehind ? m_octree.intersects_line(_from, _normDir, minT, _fid) : m_octree.intersects_ray(_from, _normDir, minT, _fid))
		{
			const Vec point{ _from + _normDir * minT };
			ensure(m_mesh.face_is_visible(_fid, _pid));
			_eid = Meshing::Utils::closestFidEid(m_mesh, _fid, point);
			_vid = Meshing::Utils::closestFidVid(m_mesh, _fid, point);
			if (!m_mesh.edge_contains_vert(_eid, _vid))
			{
				const Vec eidCentroid{ Utils::centroid(Utils::verts(m_mesh, Utils::eidVids(m_mesh, _eid))) };
				const Vec vert{ m_mesh.vert(_vid) };
				if (point.dist(vert) < point.dist(eidCentroid))
				{
					_eid = Meshing::Utils::closestFidEidByVid(m_mesh, _fid, _vid, point);
				}
				else
				{
					_vid = Meshing::Utils::closestEidVid(m_mesh, _eid, point);
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	void Mesher::setEdgeThickness(float _thickness)
	{
		m_mesh.show_out_wireframe_width(_thickness);
		m_mesh.show_in_wireframe_width(_thickness);
	}

	float Mesher::edgeThickness() const
	{
		return static_cast<float>(m_mesh.drawlist_out.seg_width);
	}

	void Mesher::updateOctree()
	{
		m_octree.clear();
		m_octree.items.reserve(m_mesh.num_faces());
		for (Id fid{}; fid < m_mesh.num_faces(); fid++)
		{
			Id facePid;
			if (m_mesh.face_is_visible(fid, facePid))
			{
				const bool cw{ m_mesh.poly_face_is_CW(facePid, fid) };
				for (I ti{}; ti < 2; ti++)
				{
					m_octree.push_triangle(fid, {
							m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + (cw ? 2 : 0)]),
							m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + 1]),
							m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + (cw ? 0 : 2)]),
						}
					);
				}
			}
		}
		m_octree.build();
	}

}