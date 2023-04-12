#pragma once

#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/concepts.hpp>
#include <HMP/Dag/NodeSet.hpp>
#include <HMP/Meshing/types.hpp>
#include <type_traits>
#include <queue>

namespace HMP::Dag
{

	template <cpputils::concepts::DerivedSimpleClass<Node>, bool TDescending = true>
	class NodeHandle;

	class Element;
	class Operation;

	class Node : public cpputils::mixins::ReferenceClass
	{

	private:

		static I s_allocatedNodeCount;

	public:

		static I allocatedNodeCount();

	public:

		using Set = NodeSet<Node>;

		enum class EType
		{
			Element, Operation
		};

	private:

		template <cpputils::concepts::DerivedSimpleClass<Node>, bool>
		friend class NodeHandle;

		Internal::NodeSetData m_parentsImpl, m_childrenImpl;

		I m_handles;

		static void deleteDangling(std::queue<Node*>& _dangling, bool _descending);
		bool onAttach(Node& _node, bool _descending);
		bool onDetach(Node& _node, bool _deleteDangling, bool _descending);
		bool onDetachAll(bool _deleteDangling, bool _descending);

		bool onParentAttach(Node& _parent);
		bool onParentDetach(Node& _parent, bool _deleteDangling);
		bool onParentsDetachAll(bool _deleteDangling);
		bool onChildAttach(Node& _child);
		bool onChildDetach(Node& _child, bool _deleteDangling);
		bool onChildrenDetachAll(bool _deleteDangling);

	protected:

		Node(EType _type);
		virtual ~Node();

		virtual void onParentAttaching(Node& _parent) const;
		virtual void onChildAttaching(Node& _child) const;

		Internal::NodeSetHandle& parentsHandle();
		Internal::NodeSetHandle& childrenHandle();

	public:

		const EType type;
		Set parents, children;

		bool isElement() const;
		bool isOperation() const;

		bool isRoot() const;
		bool isLeaf() const;

		Element& element();
		const Element& element() const;
		Operation& operation();
		const Operation& operation() const;

		template<cpputils::concepts::DerivedSimpleClass<Node> TNode>
		TNode& as();

		template<cpputils::concepts::DerivedSimpleClass<Node> TNode>
		const TNode& as() const;

		Set& forward(bool _descending);
		const Set& forward(bool _descending) const;
		Set& back(bool _descending);
		const Set& back(bool _descending) const;

	};

}

#define HMP_DAG_NODE_IMPL
#include <HMP/Dag/Node.tpp>
#undef HMP_DAG_NODE_IMPL

#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
