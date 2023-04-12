#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Scheme.hpp>
#include <unordered_map>

namespace HMP::Refinement
{

	enum class EScheme
	{
		Subdivide3x3, AdapterFaceSubdivide3x3, Adapter2FacesSubdivide3x3, AdapterEdgeSubdivide3x3, Inset, Subdivide2x2, Test, PlaneSplit
	};

	extern const std::unordered_map<EScheme, const Scheme&> schemes;

	namespace Schemes
	{

		extern const Scheme subdivide3x3;
		extern const Scheme adapterFaceSubdivide3x3;
		extern const Scheme adapter2FacesSubdivide3x3;
		extern const Scheme adapterEdgeSubdivide3x3;
		extern const Scheme inset;
		extern const Scheme subdivide2x2;
		extern const Scheme test;
		extern const Scheme planeSplit;

	}

}