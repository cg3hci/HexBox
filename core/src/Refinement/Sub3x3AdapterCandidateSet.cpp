#include <HMP/Refinement/Sub3x3AdapterCandidateSet.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <algorithm>

namespace HMP::Refinement
{

	void Sub3x3AdapterCandidateSet::addAdjacency(const Meshing::Mesher& _mesher, Dag::Element& _candidate, const Dag::Element& _refined, bool _edge)
	{
		// find the map that contains the candidate (if any) and the iterator
		Map* map{ &m_sub3x3Map };
		auto it{ m_sub3x3Map.find(&_candidate) };
		if (it == m_sub3x3Map.end())
		{
			it = m_nonSub3x3Map.find(&_candidate);
			map = &m_nonSub3x3Map;
		}
		// get the existing candidate or create a new one
		Sub3x3AdapterCandidate candidate{ it == map->end() ? Sub3x3AdapterCandidate{_candidate} : it->second };
		candidate.addAdjacency(_mesher, _refined, _edge);
		// remove it from the previous map (if any) and add it again since it changed
		if (it != map->end())
		{
			map->erase(it);
		}
		Map& newMap{ (candidate.scheme() == Refinement::EScheme::Subdivide3x3) ? m_sub3x3Map : m_nonSub3x3Map };
		newMap.insert({ &_candidate, candidate });
	}

	void Sub3x3AdapterCandidateSet::addAdjacency(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		Dag::Element& refEl = _refine.parents.single();
		const Id refPid{ refEl.pid };
		// add face to face adjacent elements
		for (const Id candPid : mesh.adj_p2p(refPid))
		{
			if (_mesher.shown(candPid)) // skip if deleted or already refined
			{
				addAdjacency(_mesher, _mesher.element(candPid), refEl, false);
			}
		}
		// add face to edge adjacent elements
		for (const Id sharedEid : mesh.adj_p2e(refPid)) // for each adjacent edge sharedEid
		{
			for (const Id candPid : mesh.adj_e2p(sharedEid)) // for each adjacent element candPid to sharedEid
			{
				// if candPid is not the refined element, nor is adjacent face to face to it
				if (candPid != refPid && static_cast<Id>(mesh.poly_shared_face(candPid, refPid)) == noId)
				{
					if (_mesher.shown(candPid)) // skip if deleted or already refined
					{
						addAdjacency(_mesher, _mesher.element(candPid), refEl, true);
					}
				}
			}
		}
	}

	Sub3x3AdapterCandidate Sub3x3AdapterCandidateSet::pop()
	{
		// get the Subdivide3x3 candidates first
		Map& map{ m_sub3x3Map.empty() ? m_nonSub3x3Map : m_sub3x3Map };
		const auto it{ map.begin() };
		const Sub3x3AdapterCandidate candidate{ it->second };
		map.erase(it);
		return candidate;
	}

	bool Sub3x3AdapterCandidateSet::empty() const
	{
		return m_sub3x3Map.empty() && m_nonSub3x3Map.empty();
	}

}