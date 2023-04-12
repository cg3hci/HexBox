#pragma once

#include <cpputils/range/Ranged.hpp>
#include <cpputils/range/of.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <unordered_map>
#include <functional>
#include <list>
#include <type_traits>

namespace HMP::Dag
{

	class Node;

	template<typename>
	class NodeSet;

	namespace Internal
	{

		template<typename>
		class NodeSetBase;

		class NodeSetData final: public cpputils::mixins::ReferenceClass
		{

		private:

			template<typename TNode>
			friend class NodeSetBase;

			std::unordered_map<Node*, std::list<Node*>::iterator> m_map{};
			std::list<Node*> m_list{};

		public:

			bool add(Node& _node);
			bool remove(Node& _node);
			bool clear();

		};

		class NodeSetHandle final: public cpputils::mixins::ReferenceClass
		{

		private:

			template<typename>
			friend class NodeSetBase;

			template<typename>
			friend class Dag::NodeSet;

			NodeSetData& m_data;
			const std::function<bool(Node&)> m_onAttach;
			const std::function<bool(Node&, bool)> m_onDetach;
			const std::function<bool(bool)> m_onDetachAll;

			NodeSetHandle(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll);

		};

		template<typename TNode>
		constexpr TNode& nonConstNodeMapper(Node* _node) { return static_cast<TNode&>(*_node); }

		template<typename TNode>
		constexpr const TNode& constNodeMapper(Node* _node) { return static_cast<const TNode&>(*_node); }

		template<typename TNode>
		using NonConstNodeRange = decltype(cpputils::range::ofc(std::declval<std::list<Node*>>()).map(&nonConstNodeMapper<TNode>));

		template<typename TNode>
		using ConstNodeRange = decltype(cpputils::range::ofc(std::declval<const std::list<Node*>>()).map(&constNodeMapper<TNode>));

		template<typename TNode>
		class NodeSetBase: public cpputils::mixins::ReferenceClass, public cpputils::range::Ranged<typename ConstNodeRange<TNode>::Iterator, typename NonConstNodeRange<TNode>::Iterator>
		{

		private:

			template<typename>
			friend class Dag::NodeSet;

			NodeSetHandle& m_handle;
			const bool m_owner;

			NodeSetBase(NodeSetHandle& _NodeSetHandle, bool _owner);

			~NodeSetBase();

			NonConstNodeRange<TNode> range() override;

			ConstNodeRange<TNode> range() const override;

		public:

			bool attach(TNode& _node);
			bool detach(TNode& _node, bool _deleteDangling = false);
			bool detachAll(bool _deleteDangling = false);

			bool has(const TNode& _node) const;

		};

	}

	template<typename TNode>
	class NodeSet final: public Internal::NodeSetBase<TNode>
	{

	private:

		friend class Node;

		NodeSet(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll);

		Internal::NodeSetHandle& handle();
		Internal::NodeSetData& data();
		const Internal::NodeSetData& data() const;

	public:

		NodeSet(Internal::NodeSetHandle& _NodeSetHandle);

	};

}

#define HMP_DAG_NODESET_IMPL
#include <HMP/Dag/NodeSet.tpp>
#undef HMP_DAG_NODESET_IMPL