#include <HMP/Dag/Element.hpp>

namespace HMP::Dag
{

	Element::Element()
		: Node{ EType::Element }, parents{ parentsHandle() }, children{ childrenHandle() }, vids{}, pid{ noId }
	{
		vids.fill(noId);
	}

	Element::Set& Element::forward(bool _descending)
	{
		return _descending ? children : parents;
	}

	const Element::Set& Element::forward(bool _descending) const
	{
		return const_cast<Element*>(this)->forward(_descending);
	}

	Element::Set& Element::back(bool _descending)
	{
		return forward(!_descending);
	}

	const Element::Set& Element::back(bool _descending) const
	{
		return const_cast<Element*>(this)->back(_descending);
	}

}