#include <HMP/Actions/SplitPlane.hpp>

#include <HMP/Refinement/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <cassert>
#include <queue>
#include <unordered_set>
#include <queue>

namespace HMP::Actions
{

	void SplitPlane::apply()
	{
		m_oldState = mesher().state();
		if (!m_prepared)
		{
			m_prepared = true;
			const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
			std::unordered_set<Id> visitedEids;
			std::unordered_set<Id> visitedPids;
			std::queue<Id> toVisitEids;
			toVisitEids.push(m_eid);
			visitedEids.insert(m_eid);
			while (!toVisitEids.empty())
			{
				const Id currEid{ toVisitEids.front() };
				toVisitEids.pop();
				for (const Id adjPid : mesh.adj_e2p(currEid))
				{
					if (mesher().shown(adjPid) && !visitedPids.contains(adjPid))
					{
						visitedPids.insert(adjPid);
						Id fid;
						for (const Id adjFid : mesh.adj_p2f(adjPid))
						{
							if (mesh.face_contains_edge(adjFid, currEid))
							{
								fid = adjFid;
								break;
							}
						}
						const I fi{ Meshing::Utils::fi(mesher().element(adjPid).vids, Meshing::Utils::fidVids(mesh, fid)) };
						const I ei{ Meshing::Utils::ei(mesher().element(adjPid).vids, Meshing::Utils::eidVids(mesh, currEid)) };
						const I vi = Meshing::Utils::firstFiVi(fi, ei);
						m_operations.emplace_back(
							Dag::NodeHandle<Dag::Refine>{ Refinement::Utils::prepare(fi, vi, HMP::Refinement::EScheme::PlaneSplit) },
							& mesher().element(adjPid)
						);
					}
				}
				for (const Id adjFid : mesh.adj_e2f(currEid))
				{
					for (const Id fidEid : mesh.adj_f2e(adjFid))
					{
						if (fidEid != currEid && !mesh.edges_are_adjacent(currEid, fidEid))
						{
							if (!visitedEids.contains(fidEid))
							{
								visitedEids.insert(fidEid);
								toVisitEids.push(fidEid);
							}
							break;
						}
					}
				}
			}
		}
		for (auto [operation, element] : m_operations)
		{
			operation->parents.attach(*element);
			Refinement::Utils::apply(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	void SplitPlane::unapply()
	{
		for (auto& [op, el] : cpputils::range::of(m_operations).reverse())
		{
			mesher().show(*el, true);
			op->parents.detachAll(false);
		}
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	SplitPlane::SplitPlane(Id _eid)
		: m_prepared{ false }, m_eid{ _eid }
	{}

	Id SplitPlane::eid() const
	{
		return m_eid;
	}

	std::pair<const Dag::Refine&, const Dag::Element&> SplitPlane::dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>& _pair)
	{
		const auto& [refine, element] { _pair };
		return { *refine, *element };
	}

	SplitPlane::Operations SplitPlane::operations() const
	{
		return cpputils::range::ofc(m_operations).map(&dereferenceOperation);
	}

}