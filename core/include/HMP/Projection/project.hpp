#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <vector>
#include <optional>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Projection/Utils.hpp>
#include <HMP/Projection/jacobianAdvance.hpp>

namespace HMP::Projection
{

    enum class EBaseWeightMode
    {
        Distance, BarycentricCoords
    };

    enum class EDisplaceMode
    {
        NormDirAvgAndDirNormAvg, NormDirAvgAndDirAvg, DirAvg, VertAvg
    };

    enum class EJacobianCheckMode
    {
        None, Surface, All
    };

    struct Options final
    {

        EBaseWeightMode baseWeightMode{ EBaseWeightMode::Distance };
        EDisplaceMode displaceMode{ EDisplaceMode::DirAvg };
        EJacobianCheckMode jacobianCheckMode{ EJacobianCheckMode::All };
        EJacobianAdvanceMode jacobianAdvanceMode{ EJacobianAdvanceMode::Length };
        I jacobianAdvanceMaxTests{ 7 };
        I smoothInternalIterations{ 1 };
        I smoothSurfaceIterations{ true };
        Real smoothInternalDoneWeight{ 1.0 };
        Real jacobianAdvanceStopThreshold{ 0.1 };
        Utils::Tweak baseWeightTweak{ 0.0, 1.0 };
        Utils::Tweak normalDotTweak{ -1.0, 0.0 };
        Utils::Tweak unsetVertsDistWeightTweak{ 0.0, 0.0 };
        double distanceWeight{ 0.0 };
        double distanceWeightPower{ 1.0 };
        double advancePercentile{ 0.5 };
        std::optional<std::vector<bool>> vertexMask{std::nullopt};
        I iterations{ 5 };
#ifdef HMP_ENABLE_ALT_PROJ
        bool alternativeMethod{ false };
#else
        static constexpr bool alternativeMethod{ false };
#endif

    };

    std::vector<Vec> project(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options = {});

}
