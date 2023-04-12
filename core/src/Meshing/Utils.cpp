#include <HMP/Meshing/Utils.hpp>

#include <cpputils/unreachable.hpp>
#include <HMP/Dag/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <cpputils/range/join.hpp>
#include <cinolib/geometry/polygon_utils.h>
#include <cassert>
#include <algorithm>

namespace HMP::Meshing::Utils
{

	bool VertComparer::operator()(const Vec& _a, const Vec& _b) const
	{
		const Real xDiff{ _a.x() - _b.x() };
		const Real absXDiff{ std::abs(xDiff) };
		if (xDiff < 0.0 && absXDiff > eps)
		{
			return true;
		}
		else if (absXDiff < eps)
		{
			const Real yDiff{ _a.y() - _b.y() };
			if (yDiff < 0.0 && std::abs(yDiff) > eps)
			{
				return true;
			}
			else if (std::abs(yDiff) < eps)
			{
				const Real zDiff{ _a.z() - _b.z() };
				if (zDiff < 0.0 && std::abs(zDiff) > eps)
				{
					return true;
				}
			}
		}
		return false;
	}

	Id anyAdjFidInPidByFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid)
	{
		return adjFidInPidByFidAndEid(_mesh, _pid, _fid, _mesh.adj_f2e(_fid)[0]);
	}

	Id adjFidInPidByVidAndFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _vid, Id _fid1, Id _fid2)
	{
		return cpputils::range::of(_mesh.poly_faces_id(_pid)).filter([&](const Id _fid) {
			return _fid != _fid1 && _fid != _fid2
			&& _mesh.faces_are_adjacent(_fid, _fid1)
			&& _mesh.faces_are_adjacent(_fid, _fid2)
			&& _mesh.face_contains_vert(_fid, _vid);
		}).single();
	}

	Id anyAdjFidInPidByFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid1, Id _fid2)
	{
		return adjFidInPidByVidAndFids(_mesh, _pid, _mesh.edge_vert_id(_mesh.face_shared_edge(_fid1, _fid2), 0), _fid1, _fid2);
	}

	Id anyAdjFidInPidByEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid)
	{
		assert(_mesh.poly_contains_edge(_pid, _eid));
		for (const Id fid : _mesh.adj_e2f(_eid))
		{
			if (_mesh.poly_contains_face(_pid, fid))
			{
				return fid;
			}
		}
		cpputils::unreachable();
	}

	Id adjFidInPidByFidAndEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid)
	{
		assert(_mesh.poly_contains_face(_pid, _fid));
		assert(_mesh.face_contains_edge(_fid, _eid));
		for (const Id fid : _mesh.poly_faces_id(_pid))
		{
			if (fid != _fid && _mesh.face_shared_edge(fid, _fid) == _eid)
			{
				return fid;
			}
		}
		cpputils::unreachable();
	}

	Id sharedEid(const Meshing::Mesher::Mesh& _mesh, Id _pid1, Id _pid2)
	{
		for (const Id sharedEid : _mesh.adj_p2e(_pid1))
		{
			for (const Id adjPid : _mesh.adj_e2p(sharedEid))
			{
				if (adjPid == _pid2)
				{
					return sharedEid;
				}
			}
		}
		cpputils::unreachable();
	}

	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVertOffsets, Id _pid)
	{
		return {
			_mesh.poly_vert_id(_pid, _edgeVertOffsets[0]),
			_mesh.poly_vert_id(_pid, _edgeVertOffsets[1])
		};
	}

	Id nextVidInFid(const QuadVertIds& _vids, Id _vid, bool _backwards)
	{
		const int index{ static_cast<int>(std::distance(_vids.begin(), std::find(_vids.begin(), _vids.end(), _vid))) };
		return _vids[static_cast<I>((index + (_backwards ? -1 : 1)) % 4)];
	}

	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, Id _eid)
	{
		return {
			_mesh.edge_vert_id(_eid, 0),
			_mesh.edge_vert_id(_eid, 1)
		};
	}

	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, Id _eid, Id _pid)
	{
		return edgeHexVertOffsets(_mesh, edgeVids(_mesh, _eid), _pid);
	}

	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVids, Id _pid)
	{
		return {
			_mesh.poly_vert_offset(_pid, _edgeVids[0]),
			_mesh.poly_vert_offset(_pid, _edgeVids[1])
		};
	}

	QuadVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _cw)
	{
		assert(_mesh.poly_contains_face(_pid, _fid));
		QuadVertIds vids{ fidVids(_mesh, _fid) };
		if (_cw == _mesh.poly_face_winding(_pid, _fid))
		{
			std::reverse(vids.begin(), vids.end());
		}
		return cpputils::range::of(vids).toArray<4>();
	}

	QuadVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _cw)
	{
		assert(_mesh.face_contains_edge(_fid, _firstEid));
		QuadVertIds vids{ pidFidVids(_mesh, _pid, _fid, _cw) };
		while (static_cast<Id>(_mesh.edge_id(vids[0], vids[1])) != _firstEid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		return vids;
	}

	QuadVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _cw)
	{
		assert(_mesh.face_contains_vert(_fid, _firstVid));
		QuadVertIds vids{ pidFidVids(_mesh, _pid, _fid, _cw) };
		while (vids[0] != _firstVid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		return vids;
	}

	bool isEdgeCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, Id _eid)
	{
		return areVidsCW(_mesh, _pid, _fid, _firstVid, _mesh.vert_opposite_to(_eid, _firstVid));
	}

	bool areVidsCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _vid0, Id _vid1)
	{
		return !isEdgeForward(pidFidVids(_mesh, _pid, _fid), _vid0, _vid1);
	}

	bool isEdgeForward(const QuadVertIds& _vids, Id _vid0, Id _vid1)
	{
		const I index0{ static_cast<I>(std::distance(_vids.begin(), std::find(_vids.begin(), _vids.end(), _vid0))) };
		return _vids[(index0 + 1) % 4] == _vid1;
	}

	HexVertIds pidVidsByForwardFidAndFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid)
	{
		const QuadVertIds forwardFaceVids{ pidFidVidsByFirstEid(_mesh, _pid, _forwardFid, _forwardUpEid, true) };
		QuadVertIds backFaceVids{ pidFidVids(_mesh, _pid, _mesh.poly_face_opposite_to(_pid, _forwardFid), false) };
		while (static_cast<Id>(_mesh.edge_id(forwardFaceVids[0], backFaceVids[0])) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		HexVertIds vids;
		std::copy(forwardFaceVids.begin(), forwardFaceVids.end(), vids.begin());
		std::copy(backFaceVids.begin(), backFaceVids.end(), vids.begin() + 4);
		return vids;
	}

	HexVertIds pidVidsByForwardFidAndFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _firstVid)
	{
		QuadVertIds forwardFaceVids{ pidFidVidsByFirstVid(_mesh, _pid, _forwardFid, _firstVid) };
		QuadVertIds backFaceVids{ pidFidVids(_mesh, _pid, _mesh.poly_face_opposite_to(_pid, _forwardFid), true) };
		while (static_cast<Id>(_mesh.edge_id(forwardFaceVids[0], backFaceVids[0])) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		HexVertIds vids;
		std::copy(forwardFaceVids.begin(), forwardFaceVids.end(), vids.begin());
		std::copy(backFaceVids.begin(), backFaceVids.end(), vids.begin() + 4);
		return vids;
	}

	Id closestEidVid(const Meshing::Mesher::Mesh& _mesh, Id _eid, const Vec& _position)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestVid{ noId };
		for (const Id vid : eidVids(_mesh, _eid))
		{
			const Real dist{ _position.dist(_mesh.vert(vid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestVid = vid;
			}
		}
		return closestVid;
	}

	Id closestFidEidByVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, Id _vid, const Vec& _midpoint)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestEid{ noId };
		for (Id edgeOffset{ 0 }; edgeOffset < 4; edgeOffset++)
		{
			const Id eid{ _mesh.face_edge_id(_fid, edgeOffset) };
			if (_mesh.edge_contains_vert(eid, _vid))
			{
				const Vec midpoint{ centroid(verts(_mesh, eidVids(_mesh, eid))) };
				const Real dist{ _midpoint.dist(midpoint) };
				if (dist < closestDist)
				{
					closestDist = dist;
					closestEid = eid;
				}
			}
		}
		return closestEid;
	}

	Id closestFidVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestVid{ noId };
		for (const Id vid : fidVids(_mesh, _fid))
		{
			const Real dist{ _position.dist(_mesh.vert(vid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestVid = vid;
			}
		}
		return closestVid;
	}

	Id closestFidEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestEid{ noId };
		for (Id edgeOffset{ 0 }; edgeOffset < 4; edgeOffset++)
		{
			const Id eid{ _mesh.face_edge_id(_fid, edgeOffset) };
			const Vec midpoint{ centroid(verts(_mesh, eidVids(_mesh, eid))) };
			const Real dist{ _midpoint.dist(midpoint) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestEid = eid;
			}
		}
		return closestEid;
	}

	QuadVertIds fiVids(const HexVertIds& _hexVids, I _fi)
	{
		return index(_hexVids, hexFiVis[_fi]);
	}

	QuadVertIds align(const QuadVertIds& _vids, Id _firstVid, bool _reverse)
	{
		QuadVertIds vids{ _vids };
		while (vids[0] != _firstVid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		if (_reverse)
		{
			vids = reverse(vids);
		}
		return vids;
	}

	HexVertIds align(const HexVertIds& _vids, Id _firstVid, bool _reverse)
	{
		HexVertIds vids{ _vids };
		while (vids[0] != _firstVid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.begin() + 4);
			std::rotate(vids.begin() + 4, vids.begin() + 4 + 1, vids.end());
		}
		if (_reverse)
		{
			vids = reverse(vids);
		}
		return vids;
	}

	EdgeVertIds align(const EdgeVertIds& _vids, Id _firstVid, bool _reverse)
	{
		return (_reverse != (_vids[0] != _firstVid)) ? reverse(_vids) : _vids;
	}

	EdgeVertIds reverse(const EdgeVertIds& _vids)
	{
		return { _vids[1], _vids[0] };
	}

	HexVertIds rotate(const HexVertIds& _vids, I _forwardFi)
	{
		return cpputils::range::join(
			fiVids(_vids, _forwardFi),
			reverse(fiVids(_vids, hexFiOppFis[_forwardFi]))
		).toArray();
	}

	QuadVertIds reverse(const QuadVertIds& _vids)
	{
		return {
			_vids[0],
			_vids[3],
			_vids[2],
			_vids[1]
		};
	}

	HexVertIds reverse(const HexVertIds& _vids)
	{
		return {
			_vids[0],
			_vids[3],
			_vids[2],
			_vids[1],
			_vids[4],
			_vids[7],
			_vids[6],
			_vids[5]
		};
	}

	I vi(const HexVertIds& _hexVids, Id _vid)
	{
		for (I vi{}; vi < 8; vi++)
		{
			if (_hexVids[vi] == _vid)
			{
				return vi;
			}
		}
		cpputils::unreachable();
	}

	I fi(const HexVertIds& _hexVids, const QuadVertIds& _vids)
	{
		QuadVertIds qVids{ _vids };
		std::sort(qVids.begin(), qVids.end());
		for (I fi{}; fi < 6; fi++)
		{
			QuadVertIds vids{ fiVids(_hexVids, fi) };
			std::sort(vids.begin(), vids.end());
			if (vids == qVids)
			{
				return fi;
			}
		}
		cpputils::unreachable();
	}

	template<I TSize>
	I indexOf(const std::array<I, TSize>& _source, I _i)
	{
		for (I i{}; i < TSize; ++i)
		{
			if (_source[i] == _i)
			{
				return i;
			}
		}
		cpputils::unreachable();
	}

	I firstFiVi(I _fi, I _ei)
	{
		const EdgeVertIs& eiVis{ hexEiVis[_ei] };
		return isIForward(hexFiVis[_fi], eiVis[0], eiVis[1]) ? eiVis[0] : eiVis[1];
	}

	bool isIForward(const QuadVertIs& _fiVis, I _vi0, I _vi1)
	{
		const I fiVi0{ indexOf(_fiVis, _vi0) };
		return _fiVis[(fiVi0 + 1) % 4] == _vi1;
	}

	EdgeVertIds eiVids(const HexVertIds& _hexVids, I _ei)
	{
		return index(_hexVids, hexEiVis[_ei]);
	}

	I ei(const HexVertIds& _hexVids, const EdgeVertIds& _vids)
	{
		EdgeVertIds qVids{ _vids };
		std::sort(qVids.begin(), qVids.end());
		for (I ei{}; ei < 12; ei++)
		{
			EdgeVertIds vids{ eiVids(_hexVids, ei) };
			std::sort(vids.begin(), vids.end());
			if (vids == qVids)
			{
				return ei;
			}
		}
		cpputils::unreachable();
	}

	Id eid(const Mesher::Mesh& _mesh, const HexVertIds& _hexVids, I _ei)
	{
		const int eid{ _mesh.edge_id(cpputils::range::of(eiVids(_hexVids, _ei)).toVector()) };
		assert(eid != -1);
		return static_cast<Id>(eid);
	}

	Id fid(const Mesher::Mesh& _mesh, const HexVertIds& _hexVids, I _fi)
	{
		const int fid{ _mesh.face_id(cpputils::range::of(fiVids(_hexVids, _fi)).toVector()) };
		assert(fid != -1);
		return static_cast<Id>(fid);
	}

	Vec normal(const QuadVerts& _verts)
	{
		return cinolib::polygon_normal(cpputils::range::of(_verts).toVector());
	}

	Real avgEdgeLength(const QuadVerts& _verts)
	{
		Real sum{};
		for (const EdgeVertIs is : quadEiVis)
		{
			sum += _verts[is[0]].dist(_verts[is[1]]);
		}
		return sum / 4.0;
	}

	Real avgEdgeLength(const HexVerts& _verts)
	{
		Real sum{};
		for (const EdgeVertIs is : hexEiVis)
		{
			sum += _verts[is[0]].dist(_verts[is[1]]);
		}
		return sum / 12.0;
	}

	bool isShown(const Dag::Node& _node)
	{
		return _node.isElement() && _node.element().children.filter([&](const Dag::Operation& _child) {
			return _child.primitive != Dag::Operation::EPrimitive::Extrude;
		}).empty();
	}

	EdgeVertIds eidVids(const Mesher::Mesh& _mesh, Id _eid)
	{
		return cpputils::range::of(_mesh.adj_e2v(_eid)).toArray<2>();
	}

	QuadVertIds fidVids(const Mesher::Mesh& _mesh, Id _fid)
	{
		return cpputils::range::of(_mesh.adj_f2v(_fid)).toArray<4>();
	}

	HexVertIds pidVids(const Mesher::Mesh& _mesh, Id _pid)
	{
		return cpputils::range::of(_mesh.adj_p2v(_pid)).toArray<8>();
	}

	void addTree(Mesher& _mesher, Dag::Node& _root, const std::vector<Vec>& _newVerts)
	{
		std::vector<Dag::Element*> elements{
			cpputils::range::of(Dag::Utils::descendants(_root))
			.filter([&](const Dag::Node* _node) { return _node->isElement(); })
			.cast<Dag::Element*>()
			.toVector()
		};
		std::sort(elements.begin(), elements.end(), [](const Dag::Element* _a, const Dag::Element* _b) { return _a->pid < _b->pid; });
		_mesher.add(elements, _newVerts);
		for (Dag::Element* element : elements)
		{
			if (!isShown(*element))
			{
				_mesher.show(*element, false);
			}
		}
	}

}