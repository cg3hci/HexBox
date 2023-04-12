#pragma once

#include <HMP/Projection/project.hpp>

namespace HMP::Projection
{

    std::vector<Vec> altProject(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options = {});

}
