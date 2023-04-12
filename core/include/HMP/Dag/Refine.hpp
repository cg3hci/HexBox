#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Schemes.hpp>
#include <vector>

namespace HMP::Dag
{

	class Refine final: public Operation
	{

	public:

		Refine();

		Refinement::EScheme scheme;
		std::vector<Id> surfVids;
		I forwardFi;
		I firstVi;

	};

}