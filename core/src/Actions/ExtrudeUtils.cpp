#include <HMP/Actions/ExtrudeUtils.hpp>

#include <cpputils/unreachable.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/join.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <cassert>

namespace HMP::Actions::ExtrudeUtils
{

	Dag::Extrude& prepare(const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise)
	{
		Dag::Extrude& extrude{ *new Dag::Extrude{} };
		extrude.firstVi = _firstVi;
		extrude.clockwise = _clockwise;
		extrude.fis = _fis;
		extrude.source = Dag::Extrude::sourceByParentCount(_fis.size());
		extrude.children.attach(*new Dag::Element{});
		return extrude;
	}

	QuadVerts extrudeFace(const Meshing::Mesher::Mesh& _mesh, const QuadVertIds& _vids)
	{
		const QuadVerts inVerts{ Meshing::Utils::verts(_mesh, _vids) };
		const Real avgEdgeLength{ Meshing::Utils::avgEdgeLength(inVerts) };
		const Vec faceNormal = Meshing::Utils::normal(inVerts);
		QuadVerts outVerts;
		for (const auto& [in, out] : cpputils::range::zip(inVerts, outVerts))
		{
			out = in + faceNormal * avgEdgeLength;
		}
		return outVerts;
	}

	HexVertIds applyFaceExtrude(const Meshing::Mesher::Mesh& _mesh, const Dag::Element& _element, I _fi, Id _firstVid, std::vector<Vec>& _newVerts)
	{
		const QuadVertIds faceVids{ Meshing::Utils::align(Meshing::Utils::fiVids(_element.vids, _fi), _firstVid) };
		const QuadVerts newVerts{ extrudeFace(_mesh, faceVids) };
		HexVertIds vids;
		std::copy(faceVids.begin(), faceVids.end(), vids.begin());
		const Id firstNewVid{ _mesh.num_verts() + toId(_newVerts.size()) };
		_newVerts.insert(_newVerts.end(), newVerts.begin(), newVerts.end());
		for (I i{}; i < 4; i++)
		{
			vids[i + 4] = firstNewVid + toId(i);
		}
		return vids;
	}

	HexVertIds applyEdgeExtrude(const Meshing::Mesher::Mesh& _mesh, const std::array<const Dag::Element*, 2>& _elements, const std::array<I, 2>& _fis, Id _firstVid, bool _clockwise, std::vector<Vec>& _newVerts)
	{
		const std::array<QuadVertIds, 2> faceVids{
			cpputils::range::zip(_elements, _fis).map([&](const auto& _elAndFi) {
				const auto& [element, fi] {_elAndFi};
				return Meshing::Utils::align(Meshing::Utils::fiVids(element->vids, fi), _firstVid);
			}).toArray()
		};
		const std::array<QuadVerts, 2> newQuadVerts{
			cpputils::range::of(faceVids).map([&](const QuadVertIds& _vids) {
				return extrudeFace(_mesh, _vids);
			}).toArray()
		};
		const Id firstNewVid{ _mesh.num_verts() + toId(_newVerts.size()) };
		if (_clockwise)
		{
			_newVerts.push_back((newQuadVerts[0][1] + newQuadVerts[1][3]) / 2);
			_newVerts.push_back((newQuadVerts[0][2] + newQuadVerts[1][2]) / 2);
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[1][3], firstNewVid + 0, firstNewVid + 1, faceVids[1][2]
			};
		}
		else
		{
			_newVerts.push_back((newQuadVerts[0][2] + newQuadVerts[1][2]) / 2);
			_newVerts.push_back((newQuadVerts[0][3] + newQuadVerts[1][1]) / 2);
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[1][1], faceVids[1][2], firstNewVid + 0, firstNewVid + 1
			};
		}
	}

	HexVertIds applyVertexExtrude(const Meshing::Mesher::Mesh& _mesh, const std::array<const Dag::Element*, 3>& _elements, const std::array<I, 3>& _fis, Id _firstVid, bool _clockwise, std::vector<Vec>& _newVerts)
	{
		const std::array<QuadVertIds, 3> faceVids{
			cpputils::range::zip(_elements, _fis).map([&](const auto& _elAndFi) {
				const auto& [element, fi] {_elAndFi};
				return Meshing::Utils::align(Meshing::Utils::fiVids(element->vids, fi), _firstVid);
			}).toArray()
		};
		const std::array<QuadVerts, 3> newQuadVerts{
			cpputils::range::of(faceVids).map([&](const QuadVertIds& _vids) {
				return extrudeFace(_mesh, _vids);
			}).toArray()
		};
		const Id newVid{ _mesh.num_verts() + toId(_newVerts.size()) };
		_newVerts.push_back((newQuadVerts[0][2] + newQuadVerts[1][2] + newQuadVerts[2][2]) / 3);
		if (_clockwise)
		{
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[2][1], faceVids[2][2], newVid, faceVids[1][2],
			};
		}
		else
		{
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[1][1], faceVids[1][2], newVid, faceVids[2][2],
			};
		}
	}

	HexVertIds apply(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude, std::vector<Vec>& _newVerts)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id firstVid{ _extrude.parents.first().vids[_extrude.firstVi] };
		switch (_extrude.source)
		{
			case Dag::Extrude::ESource::Face:
				return applyFaceExtrude(mesh, _extrude.parents.first(), _extrude.fis.first(), firstVid, _newVerts);
			case Dag::Extrude::ESource::Edge:
				return applyEdgeExtrude(mesh, _extrude.parents.address().toArray<2>(), cpputils::range::of(_extrude.fis).toArray<2>(), firstVid, _extrude.clockwise, _newVerts);
			case Dag::Extrude::ESource::Vertex:
				return applyVertexExtrude(mesh, _extrude.parents.address().toArray<3>(), cpputils::range::of(_extrude.fis).toArray<3>(), firstVid, _extrude.clockwise, _newVerts);
			default:
				cpputils::unreachable();
		}
	}

}