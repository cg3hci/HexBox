#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Refinement/Schemes.hpp>
#include <HMP/Dag/Utils.hpp>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <optional>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Refinement/Utils.hpp>
#include <HMP/Refinement/Sub3x3AdapterCandidate.hpp>
#include <HMP/Refinement/Sub3x3AdapterCandidateSet.hpp>

namespace HMP::Actions
{

	void MakeConforming::installSub3x3Adapters()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		std::vector<Dag::Refine*> refines{};
		for (Dag::Node* node : Dag::Utils::descendants(*root()))
		{
			if (node->isOperation() && node->operation().primitive == Dag::Operation::EPrimitive::Refine)
			{
				Dag::Refine& refine{ node->as<Dag::Refine>() };
				if (refine.scheme == Refinement::EScheme::Subdivide3x3)
				{
					refines.push_back(&refine);
				}
			}
		}
		bool didSomething{ false };
		do
		{
			Refinement::Sub3x3AdapterCandidateSet set{};
			// build a set of candidates based on the source refinements
			for (Dag::Refine* refine : refines)
			{
				set.addAdjacency(mesher, *refine);
			}
			didSomething = !set.empty();
			// while there is a candidate
			while (!set.empty())
			{
				// prepare and apply its refinement
				const Refinement::Sub3x3AdapterCandidate candidate{ set.pop() };
				Dag::Refine& adapterRefine{ candidate.prepareAdapter() };
				adapterRefine.parents.attach(candidate.element());
				Refinement::Utils::apply(mesher, adapterRefine);
				m_operations.push_back({ &adapterRefine, &candidate.element() });
				// if the refinement is a new Subdivide3x3, then add it to the set
				if (candidate.scheme() == Refinement::EScheme::Subdivide3x3)
				{
					set.addAdjacency(mesher, adapterRefine);
					refines.push_back(&adapterRefine);
				}
			}
		}
		while (didSomething);
	}

	void MakeConforming::apply()
	{
		m_oldState = mesher().state();
		if (m_prepared)
		{
			for (auto [operation, element] : m_operations)
			{
				operation->parents.attach(*element);
				Refinement::Utils::apply(mesher(), *operation);
			}
		}
		else
		{
			m_prepared = true;
			installSub3x3Adapters();
		}
		mesher().updateMesh();
	}

	void MakeConforming::unapply()
	{
		for (auto& [op, el] : cpputils::range::of(m_operations).reverse())
		{
			mesher().show(*el, true);
			op->parents.detachAll(false);
		}
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	MakeConforming::MakeConforming(): m_operations{}, m_prepared{ false }
	{}

	std::pair<const Dag::Refine&, const Dag::Element&> MakeConforming::dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>& _pair)
	{
		const auto& [refine, element] {_pair};
		return { *refine, *element };
	}

	MakeConforming::Operations MakeConforming::operations() const
	{
		return cpputils::range::ofc(m_operations).map(&dereferenceOperation);
	}

}