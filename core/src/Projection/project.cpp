#include <HMP/Projection/project.hpp>

#include <cpputils/unreachable.hpp>
#include <cinolib/geometry/quad_utils.h>
#include <cinolib/meshes/polygonmesh.h>
#include <optional>
#include <algorithm>
#include <HMP/Projection/percentileAdvance.hpp>
#include <HMP/Projection/jacobianAdvance.hpp>
#include <cpputils/range/zip.hpp>
#include <HMP/Projection/fill.hpp>
#include <HMP/Projection/Match.hpp>
#include <HMP/Projection/smooth.hpp>
#include <array>
#include <cinolib/parallel_for.h>
#include <cassert>
#ifdef HMP_ENABLE_ALT_PROJ
#include <HMP/Projection/altProject.hpp>
#endif

#define HMP_PROJECTION_PROJECT_DEBUG_LOG

namespace HMP::Projection
{

    static constexpr unsigned int c_minVertsForParallelFor{ 256 };

    // surface

    std::vector<Real> surfaceVertBaseWeights(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches, const EBaseWeightMode _baseWeightMode)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        std::vector<Real> weights;
        {
            I weightCount{};
            for (const Id adjFid : adjFids)
            {
                weightCount += _matches[toI(adjFid)].size();
            }
            weights.reserve(weightCount);
        }
        switch (_baseWeightMode)
        {
            case EBaseWeightMode::Distance:
            {
                const Vec& sourceVert{ _source.vert(_vid) };
                for (const Id adjFid : adjFids)
                {
                    for (const Match::SourceToTargetVid& match : _matches[toI(adjFid)])
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                Utils::invertAndNormalizeDistances(weights);
            }
            break;
            case EBaseWeightMode::BarycentricCoords:
            {
                for (const Id adjFid : adjFids)
                {
                    const Id adjFO{ _source.poly_vert_offset(adjFid, _vid) };
                    const std::vector<Vec>& sourceQuadVerts{ _source.poly_verts(adjFid) };
                    for (const Match::SourceToTargetVid& match : _matches[toI(adjFid)])
                    {
                        cinolib::vec4<Real> sourceFaceWeights;
                        cinolib::quad_barycentric_coords(
                            sourceQuadVerts[0],
                            sourceQuadVerts[1],
                            sourceQuadVerts[2],
                            sourceQuadVerts[3],
                            match.pos,
                            sourceFaceWeights
                        );
                        weights.push_back(sourceFaceWeights[adjFO]);
                    }
                }
                Utils::normalizeWeights(weights);
            }
            break;
        }
        return weights;
    }

    std::vector<Real> surfaceVertNormzedDists(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        std::vector<Real> weights;
        {
            I weightCount{};
            for (const Id adjFid : adjFids)
            {
                if (!_matches[toI(adjFid)].empty())
                {
                    weightCount++;
                }
            }
            weights.reserve(weightCount);
        }
        const Vec& sourceVert{ _source.vert(_vid) };
        for (const Id adjFid : adjFids)
        {
            for (const Match::SourceToTargetVid& match : _matches[toI(adjFid)])
            {
                const Vec& targetVert{ _target.vert(match.targetVid) };
                weights.push_back(targetVert.dist(sourceVert));
            }
        }
        Utils::normalizeWeights(weights);
        return weights;
    }

    std::optional<Vec> projectSurfaceVert(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches, const Options& _options)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        const Vec& sourceVert{ _source.vert(_vid) };
        const std::vector<Real>& baseWeights{ surfaceVertBaseWeights(_source, _vid, _matches, _options.baseWeightMode) };
        const std::vector<Real>& normzedDists{ surfaceVertNormzedDists(_source, _target, _vid, _matches) };
        const Vec sourceNormal{ _source.vert_data(_vid).normal };
        Vec targetVertSum{};
        Vec dirSum{};
        Vec normzedDirSum{};
        Real dirLengthSum{};
        Real weightSum{};
        std::vector<Real>::const_iterator baseWeightIt{ baseWeights.begin() };
        std::vector<Real>::const_iterator normzedDistIt{ normzedDists.begin() };
        for (I i{}; i < adjFids.size(); i++)
        {
            for (const Match::SourceToTargetVid& match : _matches[toI(adjFids[i])])
            {
                const Real baseWeight{ *baseWeightIt++ };
                if (_options.baseWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Vec targetNormal{ _target.vert_data(match.targetVid).normal };
                const Real normalDot{ sourceNormal.dot(targetNormal) };
                if (_options.normalDotTweak.shouldSkip(normalDot))
                {
                    continue;
                }
                const Real normzedDist{ *normzedDistIt++ };
                const Real distanceWeight{ std::pow(normzedDist, _options.distanceWeightPower) * _options.distanceWeight + 1.0 };
                const Real weight{ _options.baseWeightTweak.apply(baseWeight) * _options.normalDotTweak.apply(normalDot) * distanceWeight };
                const Vec targetVert{ _target.vert(match.targetVid) };
                const Vec dir{ targetVert - sourceVert };
                weightSum += weight;
                targetVertSum += targetVert * weight;
                dirSum += dir * weight;
                normzedDirSum += dir.is_null() ? Vec{} : (dir.normalized() * weight);
                dirLengthSum += dir.norm() * weight;
            }
        }
        if (weightSum == 0)
        {
            return std::nullopt;
        }
        else
        {
            switch (_options.displaceMode)
            {
                case EDisplaceMode::VertAvg:
                    return targetVertSum / weightSum;
                    break;
                case EDisplaceMode::DirAvg:
                    return sourceVert + dirSum / weightSum;
                case EDisplaceMode::NormDirAvgAndDirAvg:
                    return sourceVert + normzedDirSum * ((dirSum / weightSum).norm() / weightSum);
                case EDisplaceMode::NormDirAvgAndDirNormAvg:
                    return sourceVert + normzedDirSum * dirLengthSum / weightSum / weightSum;
                default:
                    cpputils::unreachable();
            }
        }
    }

    std::vector<Vec> projectSurface(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Options& _options)
    {
        const std::vector<std::vector<Match::SourceToTargetVid>>& matches{ Match::matchSurfaceFid(_source, _target) };
        std::vector<std::optional<Vec>> projected(toI(_source.num_verts()));
        const auto func{ [&](const Id _vid) {
            projected[toI(_vid)] = projectSurfaceVert(_source, _target, _vid, matches, _options);
        } };
        cinolib::PARALLEL_FOR(0, _source.num_verts(), c_minVertsForParallelFor, func);
        return fill(_source, projected, _options.unsetVertsDistWeightTweak);
    }

    // path

    std::vector<Real> pathVertBaseWeights(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const std::vector<Id>& _adjEids, const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& _matches, const EBaseWeightMode _baseWeightMode)
    {
        std::vector<Real> weights;
        {
            I weightCount{};
            for (const Id adjEid : _adjEids)
            {
                weightCount += _matches.at(adjEid).size();
            }
            weights.reserve(weightCount);
        }
        switch (_baseWeightMode)
        {
            case EBaseWeightMode::Distance:
            {
                const Vec& sourceVert{ _source.vert(_vid) };
                for (const Id adjEid : _adjEids)
                {
                    for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                Utils::invertAndNormalizeDistances(weights);
            }
            break;
            case EBaseWeightMode::BarycentricCoords:
            {
                for (const Id adjEid : _adjEids)
                {
                    for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
                    {
                        const Id otherVid{ _source.vert_opposite_to(adjEid, _vid) };
                        const Vec vert{ _source.vert(_vid) }, otherVert{ _source.vert(otherVid) };
                        const Vec adjEdgeDir{ otherVert - vert }, progressDir{ match.pos - vert };
                        const Real progress{ progressDir.dot(adjEdgeDir) / adjEdgeDir.dot(adjEdgeDir) };
                        weights.push_back(1.0 - progress);
                    }
                }
                Utils::normalizeWeights(weights);
            }
            break;
        }
        return weights;
    }

    std::optional<Vec> projectPathVert(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<Id>& _adjEids, const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& _matches, const Options& _options)
    {
        const Vec& sourceVert{ _source.vert(_vid) };
        const std::vector<Real>& baseWeights{ pathVertBaseWeights(_source, _vid, _adjEids, _matches, _options.baseWeightMode) };
        Vec targetVertSum{};
        Vec dirSum{};
        Vec normDirSum{};
        Real dirLengthSum{};
        Real weightSum{};
        std::vector<Real>::const_iterator baseWeightIt{ baseWeights.begin() };
        for (const Id adjEid : _adjEids)
        {
            for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
            {
                const Real baseWeight{ *baseWeightIt++ };
                if (_options.baseWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Real weight{ _options.baseWeightTweak.apply(baseWeight) };
                const Vec targetVert{ _target.vert(match.targetVid) };
                const Vec dir{ targetVert - sourceVert };
                weightSum += weight;
                targetVertSum += targetVert * weight;
                dirSum += dir * weight;
                normDirSum += dir.is_null() ? Vec{} : (dir.normalized() * weight);
                dirLengthSum += dir.norm() * weight;
            }
        }
        if (weightSum == 0)
        {
            return std::nullopt;
        }
        else
        {
            switch (_options.displaceMode)
            {
                case EDisplaceMode::VertAvg:
                    return targetVertSum / weightSum;
                    break;
                case EDisplaceMode::DirAvg:
                    return sourceVert + dirSum / weightSum;
                case EDisplaceMode::NormDirAvgAndDirAvg:
                    return sourceVert + normDirSum * ((dirSum / weightSum).norm() / weightSum);
                case EDisplaceMode::NormDirAvgAndDirNormAvg:
                    return sourceVert + normDirSum * dirLengthSum / weightSum / weightSum;
                default:
                    cpputils::unreachable();
            }
        }
    }

    std::vector<Vec> projectPath(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEidsPath, const std::vector<Id>& _sourceVidsPath, const std::vector<Id>& _targetVidsPath, const Options& _options)
    {
        const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>> matches{ Match::matchPathEid(_source, _target, _sourceEidsPath, _targetVidsPath) };
        std::vector<std::optional<Vec>> projected(_sourceVidsPath.size());
        const auto func{ [&](const Id _id) {
            const I i{ toI(_id) };
            const Id vid{ _sourceVidsPath[i] };
            const std::vector<Id> adjVids{ Utils::vidsPathAdjVids(_sourceVidsPath, i) };
            std::vector<Id> adjEids(adjVids.size());
            for (const auto& [adjVid, adjEid] : cpputils::range::zip(adjVids, adjEids))
            {
                adjEid = static_cast<Id>(_source.edge_id(vid, adjVid));
            }
            projected[i] = projectPathVert(_source, _target, vid, adjEids, matches, _options);
        } };
        cinolib::PARALLEL_FOR(0, toId(_sourceVidsPath.size()), c_minVertsForParallelFor, func);
        return fillPath(_source, projected, _options.unsetVertsDistWeightTweak, _sourceVidsPath);
    }

    // final projection

    std::vector<Vec> mainProject(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
        Utils::SurfaceExporter exporter{ _source };
        const std::vector<Id> onSurfVolVids{ exporter.onSurfVolVids() };
        const std::vector<Utils::Point> surfPointFeats{ Utils::toSurfFeats(_pointFeats, exporter) };
        const std::vector<Utils::EidsPath> surfEidsPathFeats{ Utils::toSurfFeats(_pathFeats, exporter) };
        const std::vector<Utils::VidsPath> surfVidsPathFeats{ Utils::eidsToVidsPaths(surfEidsPathFeats, exporter.surf, _target) };
        std::vector<std::vector<Vec>> pathTempVerts(surfEidsPathFeats.size());
        std::vector<Vec> oldVolVerts, oldSurfVerts;
        for (I i{}; i < _options.iterations; i++)
        {
            const bool lastIteration{ i + 1 == _options.iterations };
            if (i > 0 && _options.normalDotTweak.power() != 0.0)
            {
                exporter.surf.update_normals();
            }
            oldVolVerts = exporter.vol.vector_verts();
            oldSurfVerts = exporter.surf.vector_verts();
            // points
            Utils::setVerts(exporter.surf.vector_verts(), _target.vector_verts(), surfPointFeats);
            // paths
            for (const auto& [newVerts, eidsPath, vidsPath] : cpputils::range::zip(pathTempVerts, surfEidsPathFeats, surfVidsPathFeats))
            {
                newVerts = projectPath(exporter.surf, _target, eidsPath.sourceEids, vidsPath.sourceVids, vidsPath.targetVids, _options);
            }
            Utils::setSourceVerts(pathTempVerts, exporter.surf.vector_verts(), surfVidsPathFeats);
            Utils::setVerts(exporter.surf.vector_verts(), _target.vector_verts(), surfPointFeats);
            // surface
            exporter.surf.vector_verts() = projectSurface(exporter.surf, _target, _options);
            Utils::setSourceVerts(pathTempVerts, exporter.surf.vector_verts(), surfVidsPathFeats);
            Utils::setVerts(exporter.surf.vector_verts(), _target.vector_verts(), surfPointFeats);
            // smooth
            if (!lastIteration)
            {
                for (I si{}; si < _options.smoothSurfaceIterations; si++)
                {
                    exporter.surf.vector_verts() = smooth(exporter.surf);
                    Utils::setSourceVerts(pathTempVerts, exporter.surf.vector_verts(), surfVidsPathFeats);
                    Utils::setVerts(exporter.surf.vector_verts(), _target.vector_verts(), surfPointFeats);
                    for (const auto& [newVerts, eidsPath, vidsPath] : cpputils::range::zip(pathTempVerts, surfEidsPathFeats, surfVidsPathFeats))
                    {
                        Utils::setVerts(smoothPath(exporter.surf, vidsPath.sourceVids), exporter.surf.vector_verts(), vidsPath.sourceVids);
                    }
                    Utils::setVerts(exporter.surf.vector_verts(), _target.vector_verts(), surfPointFeats);
                }
            }
            // median advance
            if (_options.advancePercentile < 1.0 && !lastIteration)
            {
                percentileAdvance(oldSurfVerts, exporter.surf.vector_verts(), exporter.surf.vector_verts(), _options.advancePercentile);
            }
            exporter.applySurfToVol();
            for (I si{}; si < _options.smoothInternalIterations; si++)
            {
                exporter.vol.vector_verts() = smoothInternal(exporter.vol, onSurfVolVids, _options.smoothInternalDoneWeight);
            }
            exporter.applySurfToVol();
            // jacobian advance
            switch (_options.jacobianCheckMode)
            {
                case EJacobianCheckMode::Surface:
                    jacobianAdvance(exporter.vol, oldSurfVerts, onSurfVolVids, _options.jacobianAdvanceMode, _options.jacobianAdvanceMaxTests, _options.jacobianAdvanceStopThreshold);
                    break;
                case EJacobianCheckMode::All:
                    jacobianAdvance(exporter.vol, oldVolVerts, _options.jacobianAdvanceMode, _options.jacobianAdvanceMaxTests, _options.jacobianAdvanceStopThreshold);
                    break;
                default:
                    break;
            }
        }
#ifdef HMP_PROJECTION_PROJECT_DEBUG_LOG
        {
            I notOkCount{};
            for (Id pid{}; pid < exporter.vol.num_polys(); pid++)
            {
                const std::vector<Vec>& verts{ exporter.vol.poly_verts(pid) };
                if (cinolib::hex_scaled_jacobian(verts[0], verts[1], verts[2], verts[3], verts[4], verts[5], verts[6], verts[7]) < 0.0)
                {
                    notOkCount++;
                }
            }
            std::cout << notOkCount << " polys with negative scaled jacobian" << std::endl;
        }
#endif
        return exporter.vol.vector_verts();
    }

    std::vector<Vec> project(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
#ifdef HMP_ENABLE_ALT_PROJ
        if (_options.alternativeMethod)
        {
            return altProject(_source, _target, _pointFeats, _pathFeats, _options);
        }
#endif
        return mainProject(_source, _target, _pointFeats, _pathFeats, _options);
    }

}
