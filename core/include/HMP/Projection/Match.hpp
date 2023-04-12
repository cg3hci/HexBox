#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace HMP::Projection::Match
{

    struct SourceToTargetVid final
    {
        Vec pos;
        Id targetVid;
    };

    std::vector<std::vector<SourceToTargetVid>> matchSurfaceFid(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target);

    std::unordered_map<Id, std::vector<SourceToTargetVid>> matchPathEid(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEidsPath, const std::vector<Id>& _targetVidsPath);

}
