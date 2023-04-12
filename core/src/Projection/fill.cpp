#include <HMP/Projection/fill.hpp>

#include <algorithm>
#include <limits>
#include <updatable_priority_queue.h>
#include <cpputils/range/zip.hpp>

namespace HMP::Projection
{

    std::vector<Vec> fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak)
    {
        std::vector<Vec> out;
        fill(_mesh, _newVerts, _distWeightTweak, out);
        return out;
    }

    std::vector<Vec> fillPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newPathVerts, const Utils::Tweak& _distWeightTweak, const std::vector<Id>& _vidsPath)
    {
        std::vector<Vec> out;
        fillPath(_mesh, _newPathVerts, _distWeightTweak, _vidsPath, out);
        return out;
    }

    void fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak, std::vector<Vec>& _out)
    {
        better_priority_queue::updatable_priority_queue<I, I> queue{};
        for (I vi{}; vi < _newVerts.size(); vi++)
        {
            if (!_newVerts[vi])
            {
                I count{};
                for (const Id adjVid : _mesh.adj_v2v(toId(vi)))
                {
                    if (_newVerts[toI(adjVid)])
                    {
                        count++;
                    }
                }
                queue.push(vi, count);
            }
        }
        std::vector<std::optional<Vec>> newVerts{ _newVerts };
        std::vector<Real> baseWeights;
        baseWeights.reserve(4);
        while (!queue.empty())
        {
            const I vi{ queue.pop_value(false).key };
            const Id vid{ toId(vi) };
            const Vec vert{ _mesh.vert(vid) };
            baseWeights.clear();
            for (const Id adjVid : _mesh.adj_v2v(vid))
            {
                baseWeights.push_back(vert.dist(_newVerts[toI(adjVid)].value_or(_mesh.vert(adjVid))));
            }
            Utils::invertAndNormalizeDistances(baseWeights);
            Vec adjVertSum{};
            Real weightSum{};
            for (const auto& [adjVid, baseWeight] : cpputils::range::zip(_mesh.adj_v2v(vid), baseWeights))
            {
                if (_distWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Real weight{ _distWeightTweak.apply(baseWeight) };
                const Vec adjVert{
                    newVerts[toI(adjVid)]
                    ? *newVerts[toI(adjVid)]
                    : _mesh.vert(adjVid)
                };
                weightSum += weight;
                adjVertSum += adjVert * weight;
            }
            newVerts[vi] = weightSum != 0.0
                ? adjVertSum / weightSum
                : vert;
            for (const Id adjVid : _mesh.adj_v2v(vid))
            {
                if (!newVerts[toI(adjVid)])
                {
                    const I oldCount{ queue.get_priority(toI(adjVid)).second };
                    queue.update(toI(adjVid), oldCount + 1);
                }
            }
        }
        _out.resize(newVerts.size());
        for (const auto& [in, out] : cpputils::range::zip(newVerts, _out))
        {
            out = *in;
        }
    }

    void fillPath(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newPathVerts, const Utils::Tweak& _distWeightTweak, const std::vector<Id>& _vidsPath, std::vector<Vec>& _out)
    {
        better_priority_queue::updatable_priority_queue<I, I> queue{};
        for (I pvi{}; pvi < _newPathVerts.size(); pvi++)
        {
            if (!_newPathVerts[pvi])
            {
                I count{};
                for (const I adjPvi : Utils::vidsPathAdjVidsI(_vidsPath, pvi))
                {
                    if (_newPathVerts[adjPvi])
                    {
                        count++;
                    }
                }
                queue.push(pvi, count);
            }
        }
        std::vector<std::optional<Vec>> newVerts{ _newPathVerts };
        std::vector<Real> baseWeights;
        baseWeights.reserve(4);
        while (!queue.empty())
        {
            const I pvi{ queue.pop_value(false).key };
            const Vec vert{ newVerts[pvi].value_or(_mesh.vert(_vidsPath[pvi])) };
            baseWeights.clear();
            const std::vector<I> adjPvis{ Utils::vidsPathAdjVidsI(_vidsPath, pvi) };
            for (const I adjPvi : adjPvis)
            {
                baseWeights.push_back(vert.dist(_newPathVerts[adjPvi].value_or(_mesh.vert(_vidsPath[adjPvi]))));
            }
            Utils::invertAndNormalizeDistances(baseWeights);
            Vec adjVertSum{};
            Real weightSum{};
            for (const auto& [adjPvi, baseWeight] : cpputils::range::zip(adjPvis, baseWeights))
            {
                if (_distWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Real weight{ _distWeightTweak.apply(baseWeight) };
                const Vec adjVert{ newVerts[adjPvi].value_or(_mesh.vert(_vidsPath[adjPvi])) };
                weightSum += weight;
                adjVertSum += adjVert * weight;
            }
            newVerts[pvi] = weightSum != 0.0
                ? adjVertSum / weightSum
                : vert;
            for (const I adjPvi : adjPvis)
            {
                if (!newVerts[adjPvi])
                {
                    const I oldCount{ queue.get_priority(adjPvi).second };
                    queue.update(adjPvi, oldCount + 1);
                }
            }
        }
        _out.resize(newVerts.size());
        for (const auto& [in, out] : cpputils::range::zip(newVerts, _out))
        {
            out = *in;
        }
    }

}
