#include <HMP/Projection/Match.hpp>

#include <cinolib/octree.h>
#include <cinolib/parallel_for.h>
#include <cpputils/range/zip.hpp>

namespace HMP::Projection::Match
{

    struct TargetVidToSource final
    {
        Vec pos;
        Id sourceId;
    };

    cinolib::Octree surfaceOctree(const cinolib::AbstractPolygonMesh<>& _mesh)
    {
        cinolib::Octree octree{};
        octree.build_from_mesh_polys(_mesh);
        return octree;
    }

    cinolib::Octree pathOctree(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<Id>& _path)
    {
        cinolib::Octree octree{};
        octree.items.reserve(_path.size());
        for (const Id eid : _path)
        {
            octree.push_segment(eid, _mesh.edge_verts(eid));
        }
        octree.build();
        return octree;
    }

    static constexpr unsigned int c_minQueriesForParallelFor{ 32 };

    std::vector<TargetVidToSource> matchSurface(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target)
    {
        std::vector<TargetVidToSource> matches(toI(_target.num_verts()));
        const cinolib::Octree sourceOctree{ surfaceOctree(_source) };
        const auto func{ [&](Id _targetVid) {
            const Vec& targetVert{ _target.vert(_targetVid) };
            Vec sourcePos;
            Id sourceFid;
            Real dist;
            sourceOctree.closest_point(targetVert, sourceFid, sourcePos, dist);
            matches[toI(_targetVid)] = {
                .pos = sourcePos,
                .sourceId = sourceFid
            };
        } };
        cinolib::PARALLEL_FOR(0, _target.num_verts(), c_minQueriesForParallelFor, func);
        return matches;
    }

    std::vector<TargetVidToSource> matchPath(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEidsPath, const std::vector<Id>& _targetVidsPath)
    {
        std::vector<TargetVidToSource> matches(_targetVidsPath.size());
        const cinolib::Octree sourceOctree{ pathOctree(_source, _sourceEidsPath) };
        const auto func{ [&_target, &sourceOctree, &matches, &_targetVidsPath](Id _targetPathId) {
            const Id targetVid{ _targetVidsPath[toI(_targetPathId)] };
            const Vec& targetVert{ _target.vert(targetVid) };
            Vec sourcePos;
            Id sourceEid;
            Real dist;
            sourceOctree.closest_point(targetVert, sourceEid, sourcePos, dist);
            matches[toI(_targetPathId)] = {
                .pos = sourcePos,
                .sourceId = sourceEid
            };
        } };
        cinolib::PARALLEL_FOR(0, toId(_targetVidsPath.size()), c_minQueriesForParallelFor, func);
        return matches;
    }

    std::vector <std::vector<SourceToTargetVid>> invertSurfaceMatches(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<TargetVidToSource>& _matches)
    {
        std::vector<std::vector<SourceToTargetVid>> invMatches(toI(_source.num_polys()));
        for (Id targetVid{}; targetVid < _target.num_verts(); targetVid++)
        {
            const TargetVidToSource& match{ _matches[toI(targetVid)] };
            invMatches[match.sourceId].push_back({
                .pos = match.pos,
                .targetVid = targetVid
                });
        }
        return invMatches;
    }

    std::unordered_map<Id, std::vector<SourceToTargetVid>> invertPathMatches(const std::vector<Id>& _sourceEidsPath, const std::vector<Id>& _targetVidsPath, const std::vector<TargetVidToSource>& _matches)
    {
        std::unordered_map<Id, std::vector<SourceToTargetVid>> invMatches;
        invMatches.reserve(_sourceEidsPath.size());
        for (const Id eid : _sourceEidsPath)
        {
            invMatches.emplace(eid, std::vector<SourceToTargetVid>{});
        }
        for (const auto& [targetVid, match] : cpputils::range::zip(_targetVidsPath, _matches))
        {
            invMatches[match.sourceId].push_back({
                .pos = match.pos,
                .targetVid = targetVid
                });
        }
        return invMatches;
    }

    std::vector<std::vector<SourceToTargetVid>> matchSurfaceFid(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target)
    {
        return invertSurfaceMatches(_source, _target, matchSurface(_source, _target));
    }

    std::unordered_map<Id, std::vector<SourceToTargetVid>> matchPathEid(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEidsPath, const std::vector<Id>& _targetVidsPath)
    {
        return invertPathMatches(_sourceEidsPath, _targetVidsPath, matchPath(_source, _target, _sourceEidsPath, _targetVidsPath));
    }

}
