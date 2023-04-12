#include <HMP/Actions/RefineSome.hpp>

#include <HMP/Refinement/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <cassert>
#include <queue>

namespace HMP::Actions
{

	std::vector<std::pair<const Dag::NodeHandle<Dag::Refine>, Dag::Element* const>> prepare(const std::vector<Dag::Element*>& _elements)
	{
		return cpputils::range::of(_elements)
			.map([](Dag::Element* _el)
				{
					Dag::Refine& refine{ Refinement::Utils::prepare(0, 0, HMP::Refinement::EScheme::Subdivide3x3) };
					return std::pair<const Dag::NodeHandle<Dag::Refine>, Dag::Element* const>{ Dag::NodeHandle<Dag::Refine>{ refine }, _el };
				})
			.toVector();
	}

	void RefineSome::apply()
	{
		m_oldState = mesher().state();
		for (auto [operation, element] : m_operations)
		{
			operation->parents.attach(*element);
			Refinement::Utils::apply(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	void RefineSome::unapply()
	{
		for (auto& [op, el] : cpputils::range::of(m_operations).reverse())
		{
			mesher().show(*el, true);
			op->parents.detachAll(false);
		}
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	RefineSome::RefineSome(const std::vector<Dag::Element*>& _elements)
		: m_operations{ prepare(_elements) }
	{}

	std::pair<const Dag::Refine&, const Dag::Element&> RefineSome::dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>& _pair)
	{
		const auto& [refine, element] { _pair };
		return { *refine, *element };
	}

	RefineSome::Operations RefineSome::operations() const
	{
		return cpputils::range::ofc(m_operations).map(&dereferenceOperation);
	}

}