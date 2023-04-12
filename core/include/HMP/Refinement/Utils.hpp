#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Schemes.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <vector>
#include <optional>
#include <unordered_set>
#include <cstddef>

namespace HMP::Refinement::Utils
{

	extern Real weldEpsFactor;
	extern Real weldEps;
	extern bool absWeldEps;

	Dag::Refine& prepare(I _forwardFi, I _firstVi, Refinement::EScheme _scheme);
	void apply(Meshing::Mesher& _mesher, Dag::Refine& _refine);

}