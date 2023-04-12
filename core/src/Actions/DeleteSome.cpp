#include <HMP/Actions/DeleteSome.hpp>

#include <cpputils/range/of.hpp>
#include <cassert>
#include <queue>

namespace HMP::Actions
{

	std::vector<std::pair<const Dag::NodeHandle<Dag::Delete>, Dag::Element* const>> prepare(const std::vector<Dag::Element*>& _elements)
	{
		return cpputils::range::of(_elements)
			.map([](Dag::Element* _el)
				{
					return std::pair<const Dag::NodeHandle<Dag::Delete>, Dag::Element* const>{ Dag::NodeHandle<Dag::Delete>{ *new Dag::Delete() }, _el };
				})
			.toVector();
	}

	void DeleteSome::apply()
	{
		for (auto [operation, element] : m_operations)
		{
			operation->parents.attach(*element);
			mesher().show(*element, false);
		}
		mesher().updateMesh();
	}

	void DeleteSome::unapply()
	{
		for (auto& [op, el] : cpputils::range::of(m_operations).reverse())
		{
			mesher().show(*el, true);
			op->parents.detachAll(false);
		}
		mesher().updateMesh();
	}

	DeleteSome::DeleteSome(const std::vector<Dag::Element*>& _elements)
		: m_operations{ prepare(_elements) }
	{}

	std::pair<const Dag::Delete&, const Dag::Element&> DeleteSome::dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Delete>, Dag::Element*>& _pair)
	{
		const auto& [del, element] { _pair };
		return { *del, *element };
	}

	DeleteSome::Operations DeleteSome::operations() const
	{
		return cpputils::range::ofc(m_operations).map(&dereferenceOperation);
	}

}