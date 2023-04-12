#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <HMP/Meshing/Mesher.hpp>
#include <vector>
#include <unordered_set>

namespace HMP::Projection
{

    void smooth(const cinolib::AbstractPolygonMesh<>& _mesh, std::vector<Vec>& _out);

    void smoothPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<Id>& _vids, std::vector<Vec>& _out);

    void smoothInternal(const Meshing::Mesher::Mesh& _mesh, std::vector<Vec>& _out, Real _doneWeight = 2.0);

    void smoothInternal(const Meshing::Mesher::Mesh& _mesh, const std::vector<Id>& _surfaceVids, Real _doneWeight, std::vector<Vec>& _out);

    std::vector<Vec> smooth(const cinolib::AbstractPolygonMesh<>& _mesh);

    std::vector<Vec> smoothPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<Id>& _vids);

    std::vector<Vec> smoothInternal(const Meshing::Mesher::Mesh& _mesh, Real _doneWeight = 2.0);

    std::vector<Vec> smoothInternal(const Meshing::Mesher::Mesh& _mesh, const std::vector<Id>& _surfaceVids, Real _doneWeight = 2.0);

}