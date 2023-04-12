#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <vector>
#include <unordered_set>
#include <optional>
#include <HMP/Projection/Utils.hpp>

namespace HMP::Projection
{

    void fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak, std::vector<Vec>& _out);

    void fillPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newPathVerts, const Utils::Tweak& _distWeightTweak, const std::vector<Id>& _vidsPath, std::vector<Vec>& _out);

    std::vector<Vec> fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak);

    std::vector<Vec> fillPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newPathVerts, const Utils::Tweak& _distWeightTweak, const std::vector<Id>& _vidsPath);

}
