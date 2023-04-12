#include <HMP/Dag/Extrude.hpp>

#include <cpputils/unreachable.hpp>

namespace HMP::Dag
{

	Extrude::ESource Extrude::sourceByParentCount(I _parentCount)
	{
		switch (_parentCount)
		{
			case 1:
				return ESource::Face;
			case 2:
				return ESource::Edge;
			case 3:
				return ESource::Vertex;
			default:
				cpputils::unreachable();
		}
	}


	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }
	{}

}