#include <HMP/Dag/Operation.hpp>

namespace HMP::Dag
{

	Operation::Operation(EPrimitive _primitive)
		: Node{ EType::Operation }, primitive{ _primitive }, parents{ parentsHandle() }, children{ childrenHandle() }
	{}

	Operation::Set& Operation::forward(bool _descending)
	{
		return _descending ? children : parents;
	}

	const Operation::Set& Operation::forward(bool _descending) const
	{
		return const_cast<Operation*>(this)->forward(_descending);
	}

	Operation::Set& Operation::back(bool _descending)
	{
		return forward(!_descending);
	}

	const Operation::Set& Operation::back(bool _descending) const
	{
		return const_cast<Operation*>(this)->back(_descending);
	}

}
