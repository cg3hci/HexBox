#include <HMP/Actions/Delete.hpp>

namespace HMP::Actions
{

	void Delete::apply()
	{
		assert(mesher().shown(m_element));
		m_operation->parents.attach(m_element);
		mesher().show(m_element, false);
		mesher().updateMesh();
	}

	void Delete::unapply()
	{
		m_operation->parents.detachAll(false);
		mesher().show(m_element, true);
		mesher().updateMesh();
	}

	Delete::Delete(Dag::Element& _element)
		: m_element{ _element }, m_operation{ *new Dag::Delete{} }
	{}

	const Dag::Element& Delete::element() const
	{
		return m_element;
	}

	const Dag::Delete& Delete::operation() const
	{
		return *m_operation;
	}

}