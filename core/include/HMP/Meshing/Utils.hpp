#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/range/of.hpp>
#include <utility>

namespace HMP::Meshing::Utils
{

	struct VertComparer final
	{

		Real eps{ 1e-9 };

		bool operator()(const Vec& _a, const Vec& _b) const;

	};

	Id anyAdjFidInPidByEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);
	Id adjFidInPidByFidAndEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid);
	Id adjFidInPidByVidAndFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _vid, Id _fid1, Id _fid2);
	Id anyAdjFidInPidByFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid1, Id _fid2);
	Id anyAdjFidInPidByFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid);

	Id sharedEid(const Meshing::Mesher::Mesh& _mesh, Id _pid1, Id _pid2);
	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, Id _eid);
	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVertOffsets, Id _pid);
	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, Id _eid, Id _pid);
	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVids, Id _pid);

	bool isEdgeCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, Id _eid);
	bool areVidsCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _vid0, Id _vid1);
	bool isEdgeForward(const QuadVertIds& _vids, Id _vid0, Id _vid1);
	Id nextVidInFid(const QuadVertIds& _vids, Id _vid, bool _backwards = false);

	QuadVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _cw = false);
	QuadVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _cw = false);
	QuadVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _cw = false);
	HexVertIds pidVidsByForwardFidAndFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid);
	HexVertIds pidVidsByForwardFidAndFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _firstVid);

	inline constexpr HexFaceData<QuadVertIs> hexFiVis{ {
		{ 0, 3, 2, 1 },
		{ 4, 5, 6, 7 },
		{ 1, 2, 6, 5 },
		{ 0, 4, 7, 3 },
		{ 0, 1, 5, 4 },
		{ 3, 7, 6, 2 }
	} };

	inline constexpr HexEdgeData<EdgeVertIs> hexEiVis{ {
		{ 0, 1 },
		{ 1, 2 },
		{ 2, 3 },
		{ 3, 0 },
		{ 4, 5 },
		{ 5, 6 },
		{ 6, 7 },
		{ 7, 4 },
		{ 0, 4 },
		{ 1, 5 },
		{ 2, 6 },
		{ 3, 7 }
	} };

	inline constexpr QuadEdgeData<EdgeVertIs> quadEiVis{ {
		{ 0, 1 },
		{ 1, 2 },
		{ 2, 3 },
		{ 3, 0 }
	} };

	inline constexpr HexFaceIs hexFiOppFis{ 1,0,3,2,5,4 };

	template<I TOutSize, I TInSize, typename TValue>
	std::array<TValue, TOutSize> index(const std::array<TValue, TInSize>& _source, const std::array<I, TOutSize>& _is);

	template<I TSize>
	I indexOf(const std::array<I, TSize>& _source, I _i);

	template<I TSize>
	std::array<Vec, TSize> verts(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, TSize>& _vids, const std::vector<Vec>& _newVerts = {});

	template<I TSize>
	Vec centroid(const std::array<Vec, TSize>& _verts);

	I firstFiVi(I _fi, I _ei);
	bool isIForward(const QuadVertIs& _fiVis, I _vi0, I _vi1);
	Id closestEidVid(const Meshing::Mesher::Mesh& _mesh, Id _eid, const Vec& _position);
	Id closestFidEidByVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, Id _vid, const Vec& _midpoint);
	Id closestFidEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint);
	Id closestFidVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position);
	QuadVertIds fiVids(const HexVertIds& _hexVids, I _fi);
	EdgeVertIds eiVids(const HexVertIds& _hexVids, I _ei);
	I vi(const HexVertIds& _hexVids, Id _vid);
	I fi(const HexVertIds& _hexVids, const QuadVertIds& _vids);
	I ei(const HexVertIds& _hexVids, const EdgeVertIds& _vids);
	QuadVertIds align(const QuadVertIds& _vids, Id _firstVid, bool _reverse = false);
	HexVertIds align(const HexVertIds& _vids, Id _firstVid, bool _reverse = false);
	EdgeVertIds align(const EdgeVertIds& _vids, Id _firstVid, bool _reverse = false);
	HexVertIds rotate(const HexVertIds& _vids, I _forwardFi);
	QuadVertIds reverse(const QuadVertIds& _vids);
	HexVertIds reverse(const HexVertIds& _vids);
	EdgeVertIds reverse(const EdgeVertIds& _vids);
	Id eid(const Mesher::Mesh& _mesh, const HexVertIds& _hexVids, I _ei);
	Id fid(const Mesher::Mesh& _mesh, const HexVertIds& _hexVids, I _fi);
	Vec normal(const QuadVerts& _verts);
	Real avgEdgeLength(const QuadVerts& _verts);
	Real avgEdgeLength(const HexVerts& _verts);
	bool isShown(const Dag::Node& _node);
	EdgeVertIds eidVids(const Mesher::Mesh& _mesh, Id _eid);
	QuadVertIds fidVids(const Mesher::Mesh& _mesh, Id _fid);
	HexVertIds pidVids(const Mesher::Mesh& _mesh, Id _pid);
	void addTree(Mesher& _mesher, Dag::Node& _root, const std::vector<Vec>& _newVerts = {});

}

#define HMP_MESHING_UTILS_IMPL
#include <HMP/Meshing/Utils.tpp>
#undef HMP_MESHING_UTILS_IMPL