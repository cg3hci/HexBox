#ifndef HMP_PROJECTION_UTILS_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Projection/Utils.hpp>

#include <cpputils/range/zip.hpp>
#include <cassert>
#include <utility>

namespace HMP::Projection::Utils
{

    template<typename M, typename V, typename E, typename P>
    bool isEidsPathClosed(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids)
    {
        return _eids.size() >= 3 && _mesh.edges_are_adjacent(_eids.front(), _eids.back());
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<I> eidsPathAdjEidsI(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids, I _i)
    {
        std::vector<I> adjEidsI;
        if (_i == 0)
        {
            if (isEidsPathClosed(_mesh, _eids))
            {
                adjEidsI.push_back(_eids.size() - 1);
            }
        }
        else
        {
            adjEidsI.push_back(_i - 1);
        }
        if (_i == _eids.size() - 1)
        {
            if (isEidsPathClosed(_mesh, _eids))
            {
                adjEidsI.push_back(0);
            }
        }
        else
        {
            adjEidsI.push_back(_i + 1);
        }
        return adjEidsI;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> eidsPathAdjEids(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids, I _i)
    {
        const std::vector<I> adjEidsI{ eidsPathAdjEidsI(_mesh, _eids, _i) };
        std::vector<Id> adjEids(adjEidsI.size());
        for (const auto& [eid, i] : cpputils::range::zip(adjEids, adjEidsI))
        {
            eid = _eids[i];
        }
        return adjEids;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> vidsToEidsPath(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _vids)
    {
        assert(_vids.size() != 1);
        std::vector<Id> eids;
        if (!_vids.empty())
        {
            eids.reserve(_vids.size() - 1);
            for (I i{ 1 }; i < _vids.size(); i++)
            {
                eids.push_back(static_cast<Id>(_mesh.edge_id(_vids[i], _vids[i - 1])));
            }
        }
        return eids;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> eidsToVidsPath(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids)
    {
        std::vector<Id> vids;
        vids.reserve(vids.size() + 1);
        if (!_eids.empty())
        {
            vids.push_back(_mesh.edge_vert_id(_eids.front(), 0));
            vids.push_back(_mesh.edge_vert_id(_eids.front(), 1));
            if (_eids.size() > 1 && !_mesh.edge_contains_vert(_eids[1], vids.back()))
            {
                std::swap(vids[0], vids[1]);
            }
            for (I i{ 1 }; i < _eids.size(); i++)
            {
                vids.push_back(_mesh.vert_opposite_to(_eids[i], vids.back()));
            }
        }
        return vids;
    }

    template<typename MS, typename VS, typename ES, typename PS, typename MT, typename VT, typename ET, typename PT>
    std::vector<VidsPath> eidsToVidsPaths(const std::vector<EidsPath>& _paths, const cinolib::AbstractMesh <MS, VS, ES, PS>& _source, const cinolib::AbstractMesh<MT, VT, ET, PT>& _target)
    {
        std::vector<VidsPath> out(_paths.size());
        for (const auto& [out, in] : cpputils::range::zip(out, _paths))
        {
            out.sourceVids = eidsToVidsPath(_source, in.sourceEids);
            out.targetVids = eidsToVidsPath(_target, in.targetEids);
        }
        return out;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> eidsPathEndVids(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids)
    {
        if (isEidsPathClosed(_mesh, _eids) || _eids.empty())
        {
            return {};
        }
        else if (_eids.size() == 1)
        {
            return _mesh.edge_vert_ids(_eids.front());
        }
        else
        {
            return {
                _mesh.vert_opposite_to(_eids[0], _mesh.vert_shared(_eids[0], _eids[1])),
                _mesh.vert_opposite_to(_eids[_eids.size() - 1], _mesh.vert_shared(_eids[_eids.size() - 1], _eids[_eids.size() - 2]))
            };
        }
    }

    template<typename MS, typename VS, typename ES, typename PS, typename MT, typename VT, typename ET, typename PT>
    std::vector<Point> endPoints(const EidsPath& _path, const cinolib::AbstractMesh <MS, VS, ES, PS>& _source, const cinolib::AbstractMesh<MT, VT, ET, PT>& _target)
    {
        const std::vector<Id> sourceVids{ eidsPathEndVids(_source, _path.sourceEids) };
        std::vector<Id> targetVids{ eidsPathEndVids(_target, _path.targetEids) };
        if (sourceVids.size() != targetVids.size() || sourceVids.empty())
        {
            return {};
        }
        const auto dist{ [&_source, &_target](const Id _sourceVid, const Id _targetVid) {
            return _source.vert(_sourceVid).dist(_target.vert(_targetVid));
        } };
        if (std::pow(dist(sourceVids[0], targetVids[0]) + dist(sourceVids[1], targetVids[1]), 2) > std::pow(dist(sourceVids[1], targetVids[0]) + dist(sourceVids[0], targetVids[1]), 2))
        {
            std::swap(targetVids[0], targetVids[1]);
        }
        return {
            Point{sourceVids[0], targetVids[0]},
            Point{sourceVids[1], targetVids[1]}
        };
    }

}