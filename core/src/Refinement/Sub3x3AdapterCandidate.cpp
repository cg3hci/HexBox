#include <HMP/Refinement/Sub3x3AdapterCandidate.hpp>

#include <cpputils/unreachable.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Refinement/Utils.hpp>
#include <cassert>
#include <vector>
#include <algorithm>

namespace HMP::Refinement
{

	void Sub3x3AdapterCandidate::setup3x3Subdivide(const Meshing::Mesher& _mesher)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ m_element->pid };
		m_scheme = Refinement::EScheme::Subdivide3x3;
		m_forwardFi = 0;
		m_firstVi = 0;
	}

	void Sub3x3AdapterCandidate::findRightAdapter(const Meshing::Mesher& _mesher)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ m_element->pid };
		// process adjacent faces ignoring adjacent edges first
		switch (m_adjacentFids.size())
		{
			case 0:
			{
				// no adjacent faces, skipping (the adapter will be chosen later, when considering edge adjacencies)
				m_scheme = std::nullopt;
			}
			break;
			case 1:
			{
				// single adjacent face -> AdapterFaceSubdivide3x3
				m_scheme = Refinement::EScheme::AdapterFaceSubdivide3x3;
				m_forwardFi = Meshing::Utils::fi(m_element->vids, Meshing::Utils::fidVids(mesh, m_adjacentFids[0]));
				m_firstVi = Meshing::Utils::hexFiVis[m_forwardFi][0];
			}
			break;
			case 2:
			{
				// two adjacent faces
				if (mesh.faces_are_adjacent(m_adjacentFids[0], m_adjacentFids[1]))
				{
					// if the two faces are adjacent -> Adapter2FacesSubdivide3x3
					m_scheme = Refinement::EScheme::Adapter2FacesSubdivide3x3;
					m_forwardFi = Meshing::Utils::fi(m_element->vids, Meshing::Utils::fidVids(mesh, m_adjacentFids[0]));
					const Id sharedEid{ static_cast<Id>(mesh.face_shared_edge(m_adjacentFids[0], m_adjacentFids[1])) };
					const I sharedEi{ Meshing::Utils::ei(m_element->vids, Meshing::Utils::eidVids(mesh, sharedEid)) };
					m_firstVi = Meshing::Utils::firstFiVi(m_forwardFi, sharedEi);
				}
				else
				{
					// otherwise -> Subdivide3x3
					setup3x3Subdivide(_mesher);
				}
			}
			break;
			case 3:
			case 4:
			case 5:
			case 6:
			{
				// 3 or more adjacent faces -> Subdivide3x3
				setup3x3Subdivide(_mesher);
				break;
			}
			default:
				cpputils::unreachable();
		}
		// now consider adjacent edges too
		// if the scheme is Subdivide3x3, leave it as it is (no edge adjacency can change this)
		if (!m_adjacentEids.empty() && m_scheme != Refinement::EScheme::Subdivide3x3)
		{
			// collect unprocessed edges (edges that are not part of an adjacent face)
			std::vector<Id> unprocessedEids{};
			for (const Id eid : m_adjacentEids)
			{
				bool processed{ false };
				for (const Id fid : m_adjacentFids)
				{
					if (mesh.face_contains_edge(fid, eid))
					{
						processed = true;
						break;
					}
				}
				if (!processed)
				{
					unprocessedEids.push_back(eid);
				}
			}
			if (!unprocessedEids.empty())
			{
				if (unprocessedEids.size() == 1 && !m_scheme)
				{
					// no adapter has already been applied and there is only a single unprocessed edge -> AdapterEdgeSubdivide3x3
					m_scheme = Refinement::EScheme::AdapterEdgeSubdivide3x3;
					const Id targetForwardFid{ Meshing::Utils::anyAdjFidInPidByEid(mesh, pid, unprocessedEids[0]) };
					m_forwardFi = Meshing::Utils::fi(m_element->vids, Meshing::Utils::fidVids(mesh, targetForwardFid));
					const I ei{ Meshing::Utils::ei(m_element->vids, Meshing::Utils::eidVids(mesh, unprocessedEids[0])) };
					m_firstVi = Meshing::Utils::firstFiVi(m_forwardFi, ei);
				}
				else
				{
					// otherwise the only choice is to do Subdivide3x3
					setup3x3Subdivide(_mesher);
				}
			}
		}
	}

	Sub3x3AdapterCandidate::Sub3x3AdapterCandidate(Dag::Element& _element): m_element{ &_element }, m_scheme{} {}

	Dag::Element& Sub3x3AdapterCandidate::element() const
	{
		return *m_element;
	}

	EScheme Sub3x3AdapterCandidate::scheme() const
	{
		return *m_scheme;
	}

	void Sub3x3AdapterCandidate::addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ m_element->pid };
		const Id refPid{ _refined.pid };
		const Id sharedFid{ static_cast<Id>(mesh.poly_shared_face(pid, refPid)) };
		if (_edge)
		{
			if (sharedFid == noId) // skip if already processed as a face adjacency
			{
				// add the shared edge to the adjacency list
				const Id sharedEid{ Meshing::Utils::sharedEid(mesh, pid, refPid) };
				m_adjacentEids.push_back(sharedEid);
			}
		}
		else
		{
			// add the shared face to the adjacency list
			m_adjacentFids.push_back(sharedFid);
		}
		findRightAdapter(_mesher);
	}

	Dag::Refine& Sub3x3AdapterCandidate::prepareAdapter() const
	{
		return Utils::prepare(m_forwardFi, m_firstVi, *m_scheme);
	}

}