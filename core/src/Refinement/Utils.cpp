#include <HMP/Refinement/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <cinolib/geometry/lerp.hpp>
#include <cassert>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <limits>

namespace HMP::Refinement::Utils
{

	Real weldEpsFactor{ 1e-1 };
	Real weldEps{ 1e-6 };
	bool absWeldEps{ false };

	Dag::Refine& prepare(I _forwardFi, I _firstVi, Refinement::EScheme _scheme)
	{
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme = _scheme;
		refine.forwardFi = _forwardFi;
		refine.firstVi = _firstVi;
		const Refinement::Scheme& scheme{ Refinement::schemes.at(_scheme) };
		for (I i{ 0 }; i < scheme.polys.size(); i++)
		{
			Dag::Element& child{ *new Dag::Element{} };
			refine.children.attach(child);
		}
		return refine;
	}

	void weldAdjacencies_TEMP_NAIVE(Meshing::Mesher& _mesher, Dag::Refine& _refine, std::vector<Id>& _schemeVids)
	{
		const Scheme& scheme{ schemes.at(_refine.scheme) };
		Dag::Element& parent{ _refine.parents.single() };
		const HexVertIds parentVids{ Meshing::Utils::align(Meshing::Utils::rotate(parent.vids, _refine.forwardFi), parent.vids[_refine.firstVi]) };
		const HexVerts parentVerts{ Meshing::Utils::verts(_mesher.mesh(), parentVids) };
		const HexVerts lerpVerts{
			parentVerts[0],
			parentVerts[1],
			parentVerts[3],
			parentVerts[2],
			parentVerts[4],
			parentVerts[5],
			parentVerts[7],
			parentVerts[6]
		};
		const Real eps{ absWeldEps ? weldEps : Meshing::Utils::avgEdgeLength(parentVerts) / static_cast<Real>(scheme.gridSize) * weldEpsFactor };
		std::vector<Id> vertMap;
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const auto processAdjPid{ [&](const Id _pid)
		{
			const Dag::Refine* adjRefine{
				_mesher
				.element(_pid)
				.children
				.filter([&](const Dag::Operation& _op) { return _op.primitive == Dag::Operation::EPrimitive::Refine; })
				.address()
				.cast<const Dag::Refine*>()
				.single(nullptr)
			};
			if (adjRefine)
			{
				const Scheme& adjScheme{ schemes.at(adjRefine->scheme) };
				if (adjScheme.gridSize == scheme.gridSize)
				{
					for (const Id vid : adjRefine->surfVids)
					{
						vertMap.push_back(vid);
					}
				}
			}
		} };
		for (const Id adjPid : _mesher.mesh().adj_p2p(parent.pid))
		{
			processAdjPid(adjPid);
		}
		for (const Id adjEid : _mesher.mesh().adj_p2e(parent.pid))
		{
			for (const Id adjPid : mesh.adj_e2p(adjEid))
			{
				if (adjPid != parent.pid && static_cast<Id>(mesh.poly_shared_face(adjPid, parent.pid)) == noId)
				{
					processAdjPid(adjPid);
				}
			}
		}
		for (const auto& [vid, ivert] : cpputils::range::zip(_schemeVids, scheme.verts))
		{
			if (vid != noId)
			{
				continue;
			}
			const Vec progress{ ivert.cast<Real>() / static_cast<Real>(scheme.gridSize) };
			const Vec vert{ cinolib::lerp3(lerpVerts, progress) };
			Real minDist{ std::numeric_limits<Real>::infinity() };
			Id minVid{ noId };
			for (const Id candVid : vertMap)
			{
				const Vec& candVert{ mesh.vert(candVid) };
				const Real dist{ vert.dist(candVert) };
				if (dist < minDist)
				{
					minDist = dist;
					minVid = candVid;
				}
			}
			if (minVid != noId && minDist <= eps)
			{
				vid = minVid;
			}
		}
	}

	void apply(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Scheme& scheme{ schemes.at(_refine.scheme) };
		std::vector<Id> schemeVids(scheme.verts.size(), noId);
		Dag::Element& parent{ _refine.parents.single() };
		const HexVertIds parentVids{ Meshing::Utils::align(Meshing::Utils::rotate(parent.vids, _refine.forwardFi), parent.vids[_refine.firstVi]) };
		// weld adjacencies
		{
			weldAdjacencies_TEMP_NAIVE(_mesher, _refine, schemeVids);
		}
		// weld corners
		for (const I cornerVi : scheme.cornerVis)
		{
			const IVec& corner{ scheme.verts[cornerVi] };
			schemeVids[cornerVi] = parentVids[scheme.cornerVi(corner)];
		}
		// create missing verts
		std::vector<Vec> newVerts;
		{
			const Id numVerts{ _mesher.mesh().num_verts() };
			const HexVerts parentVerts{ Meshing::Utils::verts(_mesher.mesh(), parentVids) };
			const HexVerts lerpVerts{
				parentVerts[0],
				parentVerts[1],
				parentVerts[3],
				parentVerts[2],
				parentVerts[4],
				parentVerts[5],
				parentVerts[7],
				parentVerts[6]
			};
			for (const auto& [vid, ivert] : cpputils::range::zip(schemeVids, scheme.verts))
			{
				if (vid == noId)
				{
					vid = numVerts + toId(newVerts.size());
					const Vec progress{ ivert.cast<Real>() / static_cast<Real>(scheme.gridSize) };
					newVerts.push_back(cinolib::lerp3(lerpVerts, progress));
				}
			}
		}
		// create elements
		{
			const std::vector<Dag::Element*> elements{
				_refine.children.zip(scheme.polys).map([&](const auto& _elAndVis)
			{
				const auto& [el, vis] { _elAndVis };
				for (const auto& [vid, vi] : cpputils::range::zip(el.vids, vis))
				{
					vid = schemeVids[vi];
				}
				return &el;
			}).toVector() };
			_refine.surfVids = cpputils::range::of(scheme.surfVis).map([&](const I _vi) { return schemeVids[_vi]; }).toVector();
			_mesher.show(parent, false);
			_mesher.add(elements, newVerts);
		}
	}

}