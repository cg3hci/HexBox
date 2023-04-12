#ifndef HMP_ENABLE_ALT_PROJ
#error alternative projection method not enabled
#endif

#include <HMP/Projection/altProject.hpp>

#include <lib/vertex_based_smoothing.h>
#include <utils/meshStructures.h>
#include <cassert>
#include <utility>
#include <cpputils/range/of.hpp>
#include <cpputils/range/index.hpp>
#include <HMP/Projection/Utils.hpp>

namespace HMP::Projection
{

    // ---------- CONVERSION UTILS ----------

    utilities::vec3 toAltVec(const Vec& _vec)
    {
        return { _vec.x(), _vec.y(), _vec.z() };
    }

    Vec toMainVec(const utilities::vec3& _vec)
    {
        return { _vec.x, _vec.y, _vec.z };
    }

    utilities::HexahedralMesh toAltHexMesh(const Meshing::Mesher::Mesh& _source)
    {
        return {
            ._pts{cpputils::range::of(_source.vector_verts()).map(&toAltVec).toVector()},
            ._hexes{cpputils::range::count(_source.num_polys())
                .filter([&](const Id _pid) {
                    return !_source.poly_data(_pid).flags[cinolib::HIDDEN];
                })
                .map([&](const Id _pid) {
                    const std::vector<Id>& _poly{_source.adj_p2v(_pid)};
                    return cpputils::range::of(std::array<Id,8>{
                        _poly[0 + 0],
                        _poly[1 + 0],
                        _poly[3 + 0],
                        _poly[2 + 0],
                        _poly[0 + 4],
                        _poly[1 + 4],
                        _poly[3 + 4],
                        _poly[2 + 4],
                    })
                    .cast<int>()
                    .toArray();
                })
                .toVector()}
        };
    }

    utilities::TriangleMesh toAltTriMesh(const cinolib::AbstractPolygonMesh<>& _target)
    {
        std::vector<std::array<int, 3>> tris;
        tris.reserve(toI(_target.num_polys()) * 3);
        for (Id pid{}; pid < _target.num_polys(); pid++)
        {
            const std::vector<Id>& vids{_target.poly_tessellation(pid)};
            for (Id t{}; t < vids.size() / 3; t++)
            {
                tris.push_back({
                    static_cast<int>(vids[3 * t + 0]),
                    static_cast<int>(vids[3 * t + 1]),
                    static_cast<int>(vids[3 * t + 2])
                    });
            }
        }
        return {
            ._pts{cpputils::range::of(_target.vector_verts()).map(&toAltVec).toVector()},
            ._tris{std::move(tris)}
        };
    }

    utilities::CurveMesh toAltCurveMesh(const cinolib::AbstractPolygonMesh<>& _target)
    {
        std::vector<std::array<int, 3>> tris;
        tris.reserve(toI(_target.num_polys()) * 3);
        for (Id pid{}; pid < _target.num_polys(); pid++)
        {
            const std::vector<Id>& vids{_target.poly_tessellation(pid)};
            for (I t{}; t < vids.size() / 3; t++)
            {
                tris.push_back({
                    static_cast<int>(vids[3 * t + 0]),
                    static_cast<int>(vids[3 * t + 1]),
                    static_cast<int>(vids[3 * t + 2])
                    });
            }
        }
        return {
            ._pts{cpputils::range::of(_target.vector_verts()).map(&toAltVec).toVector()},
            ._edges{cpputils::range::count(_target.num_edges()).map([&](const Id _eid) {
                return std::array<int, 2>{
                    static_cast<int>(_target.edge_vert_id(_eid, 0)),
                    static_cast<int>(_target.edge_vert_id(_eid, 1)),
                };
            }).toVector()}
        };
    }

    // --------------------------------------

    std::vector<Vec> altProject(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
        assert(_options.alternativeMethod);

        // source
        utilities::HexahedralMesh altSource{toAltHexMesh(_source)};
        vertex_smoother smoother(altSource);

        // no locks
        const std::vector<bool> locks(altSource._pts.size(), false);
        smoother.set_locked_vertices(locks);

        // target surface
        const utilities::TriangleMesh altTargetSurf{toAltTriMesh(_target)};
        smoother.set_bnd_triangles(altTargetSurf);

        // target edges
        const utilities::CurveMesh altTargetEdges{toAltCurveMesh(_target)};
        smoother.set_features_segment(altTargetEdges);

        // vert to surf mapping
        const std::vector<int> allTris{cpputils::range::count(static_cast<int>(altTargetSurf._tris.size())).toVector()};
        for (Id vid{}; vid < _source.num_verts(); vid++)
        {
            if (_source.vert_is_on_srf(vid) && _source.vert_is_visible(vid))
            {
                smoother.set_vertex_triangles(static_cast<int>(vid), allTris);
            }
        }

        // vert to edge chain mapping (feature paths)
        for (const Utils::EidsPath& path : _pathFeats)
        {
            const std::vector<int> edges{ cpputils::range::of(path.targetEids).cast<int>().toVector() };
            for (const Id vid : Utils::eidsToVidsPath(_source, path.sourceEids))
            {
                smoother.set_vertex_segments(static_cast<int>(vid), edges);
            }
        }

        // vert to vert mapping (feature points)
        for (const Utils::Point& point : _pointFeats)
        {
            smoother.set_vertex_point(static_cast<int>(point.sourceVid), toAltVec(_target.vert(point.targetVid)));
        }

        smoother.execute(static_cast<unsigned int>(_options.iterations));

        return cpputils::range::of(altSource._pts).map(&toMainVec).toVector();
    }

}
