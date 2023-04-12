#ifndef HMP_DAG_NODEHANDLE_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Dag/NodeHandle.hpp>

namespace HMP::Dag
{

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	void NodeHandle<TNode, TDescending>::attach(TNode* _node)
	{
		detach();
		m_node = _node;
		if (m_node)
		{
			m_node->m_handles++;
		}
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	void NodeHandle<TNode, TDescending>::detach()
	{
		if (m_node)
		{
			m_node->m_handles--;
			if (!m_node->m_handles && m_node->back(TDescending).empty())
			{
				m_node->forward(TDescending).detachAll(true);
				delete m_node;
			}
			m_node = nullptr;
		}
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>::NodeHandle(TNode* _node)
		: m_node{}
	{
		attach(_node);
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>::NodeHandle(TNode& _node)
		: m_node{}
	{
		attach(&_node);
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>::NodeHandle(const NodeHandle& _copy)
		: m_node{}
	{
		attach(_copy.m_node);
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>::NodeHandle(NodeHandle&& _moved)
		: m_node{}
	{
		attach(_moved.m_node);
		_moved.detach();
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>::~NodeHandle()
	{
		detach();
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>& NodeHandle<TNode, TDescending>::operator=(TNode* _node)
	{
		attach(_node);
		return *this;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>& NodeHandle<TNode, TDescending>::operator=(TNode& _node)
	{
		attach(&_node);
		return *this;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>& NodeHandle<TNode, TDescending>::operator=(const NodeHandle& _copy)
	{
		attach(_copy.m_node);
		return *this;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>& NodeHandle<TNode, TDescending>::operator=(NodeHandle&& _moved)
	{
		attach(_moved.m_node);
		_moved.detach();
		return *this;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	template <cpputils::concepts::DerivedSimpleClass<Node> TOtherNode, bool TOtherDescending>
	bool NodeHandle<TNode, TDescending>::operator==(const NodeHandle<TOtherNode, TOtherDescending>& _other) const
	{
		return _other.m_node == m_node;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	TNode& NodeHandle<TNode, TDescending>::operator*() const
	{
		return *m_node;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	TNode* NodeHandle<TNode, TDescending>::operator->() const
	{
		return m_node;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	NodeHandle<TNode, TDescending>::operator TNode* () const
	{
		return m_node;
	}

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	void NodeHandle<TNode, TDescending>::free()
	{
		if (m_node)
		{
			m_node->m_handles--;
			m_node = nullptr;
		}
	}

}