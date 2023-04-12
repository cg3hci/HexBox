#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Node.hpp>
#include <vector>

namespace HMP::Dag
{

	class Operation: public Node
	{

	public:

		using Set = NodeSet<Element>;

		enum class EPrimitive
		{
			Refine, Extrude, Delete
		};

	private:

		using Node::isElement;
		using Node::isOperation;
		using Node::element;
		using Node::operation;

	protected:

		explicit Operation(EPrimitive _primitive);

	public:

		const EPrimitive primitive;
		Set parents, children;

		Set& forward(bool _descending);
		const Set& forward(bool _descending) const;
		Set& back(bool _descending);
		const Set& back(bool _descending) const;

	};

}