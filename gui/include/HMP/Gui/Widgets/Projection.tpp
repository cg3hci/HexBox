#ifndef HMP_GUI_WIDGETS_PROJECTION_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Gui/Widgets/Projection.hpp>

#include <HMP/Projection/Utils.hpp>
#include <HMP/Gui/App.hpp>
#include <algorithm>
#include <limits>
#include <vector>
#include <cinolib/meshes/abstract_mesh.h>
#include <unordered_set>

namespace HMP::Gui::Widgets
{

    template<class M, class V, class E, class P>
    void Projection::setPathEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source)
    {
        if (!m_showPaths || m_showAllPaths || m_paths.empty())
        {
            return;
        }
        EidsPath& pathPair{ m_paths[m_currentPath] };
        std::vector<Id>& path{ pathPair.eids(_source) };
        const bool has1{ path.size() > 0 }, has2{ path.size() > 1 }, has3{ path.size() > 2 };
        const I lastI{ path.size() - 1 };
        const std::vector<Id> endEids{ has1 ? has2 ? std::vector<Id>{path[0], path[lastI]} : std::vector<Id>{ path[0] } : std::vector<Id>{} };
        const bool closed{ HMP::Projection::Utils::isEidsPathClosed(_mesh, path) };
        if (_add)
        {
            if (closed)
            {
                return;
            }
            Id closestEid{ noId };
            Real closestDist{ std::numeric_limits<Real>::infinity() };
            if (!has1)
            {
                for (Id eid{}; eid < _mesh.num_edges(); eid++)
                {
                    if (has2 && (eid == path[1] || eid == path[lastI - 1]))
                    {
                        continue;
                    }
                    if (_source && !app().mesh.edge_is_visible(eid))
                    {
                        continue;
                    }
                    const Real dist{ _point.dist_sqrd(_mesh.edge_sample_at(eid, 0.5)) };
                    if (dist < closestDist)
                    {
                        closestDist = dist;
                        closestEid = eid;
                    }
                }
            }
            else
            {
                for (const Id endEid : endEids)
                {
                    for (const Id eid : _mesh.adj_e2e(endEid))
                    {
                        if (_source && !app().mesh.edge_is_visible(eid))
                        {
                            continue;
                        }
                        if (has2 && (eid == path[1] || eid == path[lastI - 1]))
                        {
                            continue;
                        }
                        const Real dist{ _point.dist_sqrd(_mesh.edge_sample_at(eid, 0.5)) };
                        if (dist < closestDist)
                        {
                            closestDist = dist;
                            closestEid = eid;
                        }
                    }
                }
            }
            if (closestEid == noId)
            {
                return;
            }
            if (has2)
            {
                std::unordered_set<Id> invalidVids;
                if (has3)
                {
                    for (I i{ 1 }; i < lastI; i++)
                    {
                        const Id eid{ path[i] };
                        invalidVids.insert(_mesh.edge_vert_id(eid, 0));
                        invalidVids.insert(_mesh.edge_vert_id(eid, 1));
                    }
                }
                else
                {
                    invalidVids.insert(_mesh.vert_shared(endEids[0], endEids[1]));
                }
                if (invalidVids.contains(_mesh.edge_vert_id(closestEid, 0)) || invalidVids.contains(_mesh.edge_vert_id(closestEid, 1)))
                {
                    return;
                }
            }
            if (!has1 || !_mesh.edges_are_adjacent(path[0], closestEid))
            {
                path.push_back(closestEid);
            }
            else
            {
                path.insert(path.begin(), closestEid);
            }
        }
        else
        {
            const std::vector<Id>& candidates{ closed ? path : endEids };
            Real closestDist{ std::numeric_limits<Real>::infinity() };
            Id closestEid{ noId };
            for (const Id eid : candidates)
            {
                if (_source && !app().mesh.edge_is_visible(eid))
                {
                    continue;
                }
                const Real dist{ _point.dist_sqrd(_mesh.edge_sample_at(eid, 0.5)) };
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestEid = eid;
                }
            }
            if (closestEid == noId)
            {
                return;
            }
            const I i{ static_cast<I>(std::find(path.begin(), path.end(), closestEid) - path.begin()) };
            const I nextI{ (i + 1) % path.size() };
            std::rotate(path.begin(), path.begin() + nextI, path.end());
            path.pop_back();
        }
    }


}
