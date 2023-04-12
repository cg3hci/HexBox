#ifndef HMP_DAG_NODESET_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Dag/NodeSet.hpp>

namespace HMP::Dag
{

	namespace Internal
	{

		// NodeSetBase

		template<typename TNode>
		NodeSetBase<TNode>::NodeSetBase(NodeSetHandle& _handle, bool _owner)
			: m_handle{ _handle }, m_owner{ _owner }
		{}

		template<typename TNode>
		NodeSetBase<TNode>::~NodeSetBase()
		{
			if (m_owner)
			{
				delete& m_handle;
			}
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::attach(TNode& _node)
		{
			return m_handle.m_onAttach(_node);
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::detach(TNode& _node, bool _deleteDangling)
		{
			return m_handle.m_onDetach(_node, _deleteDangling);
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::detachAll(bool _deleteDangling)
		{
			return m_handle.m_onDetachAll(_deleteDangling);
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::has(const TNode& _node) const
		{
			return m_handle.m_data.m_map.contains(const_cast<TNode*>(&_node));
		}

		template<typename TNode>
		NonConstNodeRange<TNode> NodeSetBase<TNode>::range()
		{
			return cpputils::range::ofc(m_handle.m_data.m_list).map(&nonConstNodeMapper<TNode>);
		}

		template<typename TNode>
		ConstNodeRange<TNode> NodeSetBase<TNode>::range() const
		{
			return cpputils::range::ofc(m_handle.m_data.m_list).map(&constNodeMapper<TNode>);
		}

	}

	// NodeSet

	template<typename TNode>
	NodeSet<TNode>::NodeSet(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll)
		: Internal::NodeSetBase<TNode>{ *new Internal::NodeSetHandle{ _data, _onAttach, _onDetach, _onDetachAll }, true }
	{}

	template<typename TNode>
	Internal::NodeSetHandle& NodeSet<TNode>::handle()
	{
		return Internal::NodeSetBase<TNode>::m_handle;
	}

	template<typename TNode>
	Internal::NodeSetData& NodeSet<TNode>::data()
	{
		return Internal::NodeSetBase<TNode>::m_handle.m_data;
	}

	template<typename TNode>
	const Internal::NodeSetData& NodeSet<TNode>::data() const
	{
		return Internal::NodeSetBase<TNode>::m_handle.m_data;
	}

	template<typename TNode>
	NodeSet<TNode>::NodeSet(Internal::NodeSetHandle& _handle)
		: Internal::NodeSetBase<TNode>{ _handle, false }
	{}

}