#pragma once

#include <HMP/Meshing/types.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <vector>
#include <utility>

namespace HMP::Actions::ExtrudeUtils
{

	HexVertIds apply(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude, std::vector<Vec>& _newVerts);

	Dag::Extrude& prepare(const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise);

}