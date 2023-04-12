#ifndef HMP_DAG_NODE_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Dag/Node.hpp>

namespace HMP::Dag
{

	template<cpputils::concepts::DerivedSimpleClass<Node> TNode>
	TNode& Node::as()
	{
		return static_cast<TNode&>(*this);
	}

	template<cpputils::concepts::DerivedSimpleClass<Node> TNode>
	const TNode& Node::as() const
	{
		return static_cast<const TNode&>(*this);
	}

}