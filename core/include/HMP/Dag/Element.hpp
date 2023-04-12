#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Node.hpp>

namespace HMP::Dag
{

	class Element final: public Node
	{

	public:

		using Set = NodeSet<Operation>;

	private:

		using Node::isElement;
		using Node::isOperation;
		using Node::element;
		using Node::operation;

	public:

		Element();

		Set parents, children;
		HexVertIds vids;
		Id pid;

		Set& forward(bool _descending);
		const Set& forward(bool _descending) const;
		Set& back(bool _descending);
		const Set& back(bool _descending) const;

	};

}