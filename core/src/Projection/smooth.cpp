#include <HMP/Projection/smooth.hpp>

#include <HMP/Projection/Utils.hpp>
#include <cinolib/parallel_for.h>

namespace HMP::Projection
{

    static constexpr unsigned int c_minVertsForParallelFor{ 256 };

    void smooth(const cinolib::AbstractPolygonMesh<>& _mesh, std::vector<Vec>& _out)
    {
        _out.resize(toI(_mesh.num_verts()));
        const auto func{ [&](Id _vid) {
            Vec vertSum{};
            I vertCount{};
            for (const Id adjVid : _mesh.adj_v2v(_vid))
            {
                vertSum += _mesh.vert(adjVid);
                vertCount++;
            }
            _out[toI(_vid)] = vertCount != 0
                ? vertSum / static_cast<Real>(vertCount)
                : _mesh.vert(_vid);
        } };
        cinolib::PARALLEL_FOR(0, _mesh.num_verts(), c_minVertsForParallelFor, func);
    }

    void smoothPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<Id>& _vids, std::vector<Vec>& _out)
    {
        _out.resize(_vids.size());
        const auto func{ [&](Id _vidsI) {
            const Id vid{ _vids[toI(_vidsI)] };
            Vec vertSum{};
            I vertCount{};
            for (const Id adjVid : Utils::vidsPathAdjVids(_vids, toI(_vidsI)))
            {
                vertSum += _mesh.vert(adjVid);
                vertCount++;
            }
            _out[toI(_vidsI)] = vertCount != 0
                ? vertSum / static_cast<Real>(vertCount)
                : _mesh.vert(vid);
        } };
        cinolib::PARALLEL_FOR(0, toId(_vids.size()), c_minVertsForParallelFor, func);
    }

    void smoothInternal(const Meshing::Mesher::Mesh& _mesh, Real _doneWeight, std::vector<Vec>& _out)
    {
        smoothInternal(_mesh, Utils::SurfaceExporter::onSurfVids(_mesh), _doneWeight, _out);
    }

    bool isVidHidden(const Meshing::Mesher::Mesh& _mesh, Id _vid)
    {
        for (const Id adjPid : _mesh.adj_v2p(_vid))
        {
            if (!_mesh.poly_data(adjPid).flags[cinolib::HIDDEN])
            {
                return false;
            }
        }
        return true;
    }

    void smoothInternal(const Meshing::Mesher::Mesh& _mesh, const std::vector<Id>& _surfaceVids, Real _doneWeight, std::vector<Vec>& _out)
    {
        _out = _mesh.vector_verts();
        std::vector<bool> doneVids(toI(_mesh.num_verts()), false);
        std::vector<Id> currentVids{}, nextVids{ _surfaceVids };
        const auto func{ [&](Id _vidsI) {
            const Id vid{ currentVids[toI(_vidsI)] };
            Vec vertSum{};
            Real weightSum{};
            for (const Id adjVid : _mesh.adj_v2v(vid))
            {
                if (!isVidHidden(_mesh, adjVid))
                {
                    vertSum += doneVids[adjVid]
                        ? _out[toI(adjVid)] * _doneWeight
                        : _mesh.vert(adjVid);
                    weightSum += doneVids[adjVid]
                        ? _doneWeight
                        : 1.0;
                }
            }
            if (weightSum != 0.0)
            {
                _out[toI(vid)] = vertSum / weightSum;
            }
        } };
        while (!nextVids.empty())
        {
            currentVids = nextVids;
            nextVids.clear();
            cinolib::PARALLEL_FOR(0, toId(currentVids.size()), c_minVertsForParallelFor, func);
            for (const Id vid : currentVids)
            {
                doneVids[toI(vid)] = true;
            }
            for (const Id vid : currentVids)
            {
                for (const Id adjVid : _mesh.adj_v2v(vid))
                {
                    if (!doneVids[toI(adjVid)] && !isVidHidden(_mesh, adjVid))
                    {
                        nextVids.push_back(adjVid);
                    }
                }
            }
        }
        for (const Id vid : _surfaceVids)
        {
            _out[toI(vid)] = _mesh.vert(vid);
        }
    }

    std::vector<Vec> smooth(const cinolib::AbstractPolygonMesh<>& _mesh)
    {
        std::vector<Vec> out;
        smooth(_mesh, out);
        return out;
    }

    std::vector<Vec> smoothPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<Id>& _vids)
    {
        std::vector<Vec> out;
        smoothPath(_mesh, _vids, out);
        return out;
    }

    std::vector<Vec> smoothInternal(const Meshing::Mesher::Mesh& _mesh, Real _doneWeight)
    {
        std::vector<Vec> out;
        smoothInternal(_mesh, _doneWeight, out);
        return out;
    }

    std::vector<Vec> smoothInternal(const Meshing::Mesher::Mesh& _mesh, const std::vector<Id>& _surfaceVids, Real _doneWeight)
    {
        std::vector<Vec> out;
        smoothInternal(_mesh, _surfaceVids, _doneWeight, out);
        return out;
    }

}
