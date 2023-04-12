#include <HMP/Actions/Refine.hpp>

#include <HMP/Refinement/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <cassert>
#include <queue>

namespace HMP::Actions
{

	std::vector<std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>> prepareRecursive(Dag::Element& _element, I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
	{
		assert(_depth >= 1 && _depth <= 3);
		Dag::Refine& refine{ Refinement::Utils::prepare(_forwardFi, _firstVi, _scheme) };
		std::vector<std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>> refines{ {&_element, refine} };
		if (_depth > 1)
		{
			const I childDepth{ _depth - 1 };
			for (Dag::Element& child : refine.children)
			{
				std::vector<std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>> childRefines{ prepareRecursive(child, _forwardFi, _firstVi, _scheme, childDepth) };
				refines.reserve(refines.size() + childRefines.size());
				for (const auto& [el, op] : childRefines)
				{
					refines.emplace_back(el, op);
				}
			}
		}
		return refines;
	}

	void Refine::apply()
	{
		assert(mesher().shown(element()));
		m_oldState = mesher().state();
		for (const auto& [element, operation] : m_operations)
		{
			operation->parents.attach(*element);
			Refinement::Utils::apply(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		for (const auto& [element, operation] : cpputils::range::of(m_operations).reverse())
		{
			operation->parents.detachAll(false);
			mesher().show(*element, true);
		}
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
		: m_operations{ prepareRecursive(_element, _forwardFi, _firstVi, _scheme, _depth) }, m_depth{ _depth }
	{
		assert(_depth >= 1 && _depth <= 3);
		assert(_forwardFi < 6);
		assert(_firstVi < 8);
		assert(Refinement::schemes.contains(_scheme));
	}

	const Dag::Element& Refine::element() const
	{
		return *m_operations.front().first;
	}

	const Dag::Refine& Refine::operation() const
	{
		return *m_operations.front().second;
	}

	I Refine::depth() const
	{
		return m_depth;
	}

}