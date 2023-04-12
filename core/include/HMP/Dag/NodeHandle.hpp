#pragma once

#include <HMP/Dag/Node.hpp>
#include <cpputils/concepts.hpp>

namespace HMP::Dag
{

	template <cpputils::concepts::DerivedSimpleClass<Node> TNode, bool TDescending>
	class NodeHandle final
	{

	private:

		TNode* m_node;

		void attach(TNode* _node);
		void detach();

	public:

		NodeHandle(TNode* _node = nullptr);
		NodeHandle(TNode& _node);
		NodeHandle(const NodeHandle& _copy);
		NodeHandle(NodeHandle&& _moved);

		~NodeHandle();

		NodeHandle& operator=(TNode* _node);
		NodeHandle& operator=(TNode& _node);
		NodeHandle& operator=(const NodeHandle& _copy);
		NodeHandle& operator=(NodeHandle&& _moved);

		template <cpputils::concepts::DerivedSimpleClass<Node> TOtherNode, bool TOtherDescending>
		bool operator==(const NodeHandle<TOtherNode, TOtherDescending>& _other) const;

		TNode& operator*() const;
		TNode* operator->() const;
		operator TNode* () const;

		void free();

	};

}

#define HMP_DAG_NODEHANDLE_IMPL
#include <HMP/Dag/NodeHandle.tpp>
#undef HMP_DAG_NODEHANDLE_IMPL