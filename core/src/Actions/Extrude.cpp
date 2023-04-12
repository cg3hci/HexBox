#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/ExtrudeUtils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/index.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cassert>

namespace HMP::Actions
{

	void Extrude::apply()
	{
		m_oldState = mesher().state();
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents.attach(*parent);
		}
		Dag::Element& child{ m_operation->children.single() };
		std::vector<Vec> newVerts;
		child.vids = ExtrudeUtils::apply(mesher(), *m_operation, newVerts);
		mesher().add({ &child }, newVerts);
		mesher().updateMesh();
	}

	void Extrude::unapply()
	{
		m_operation->parents.detachAll(false);
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	Extrude::Extrude(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise)
		: m_elements{ _elements }, m_operation{ ExtrudeUtils::prepare(_fis, _firstVi, _clockwise) }
	{
		assert(_elements.size() == _fis.size());
		assert(_firstVi < 8);
	}

	Extrude::Elements Extrude::elements() const
	{
		return cpputils::range::ofc(m_elements).dereference().immutable();
	}

	const Dag::Extrude& Extrude::operation() const
	{
		return *m_operation;
	}

}