#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <vector>

namespace HMP::Projection
{

    enum class EJacobianAdvanceMode
    {
        Length, Lerp
    };

    void jacobianAdvance(Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _from, EJacobianAdvanceMode _mode = EJacobianAdvanceMode::Length, I _maxTests = 7, Real _stopThreshold = 0.1);

    void jacobianAdvance(Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _from, const std::vector<Id>& _vids, EJacobianAdvanceMode _mode = EJacobianAdvanceMode::Length, I _maxTests = 7, Real _stopThreshold = 0.1);

}
