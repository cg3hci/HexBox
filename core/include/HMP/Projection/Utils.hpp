#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cinolib/meshes/polygonmesh.h>
#include <cinolib/meshes/abstract_mesh.h>
#include <vector>
#include <unordered_map>

namespace HMP::Projection::Utils
{

    class Tweak final
    {

    private:

        Real m_min;
        Real m_power;

    public:

        Tweak(Real _min, Real _power = 1.0);

        Real min() const;
        Real power() const;

        bool shouldSkip(Real _value) const;
        Real apply(Real _value) const;

    };

    struct Point final
    {
        Id sourceVid, targetVid;
    };

    struct EidsPath final
    {
        std::vector<Id> sourceEids, targetEids;

        bool empty() const;
        bool empty(bool _source) const;
        const std::vector<Id>& eids(bool _source) const;
        std::vector<Id>& eids(bool _source);

    };

    struct VidsPath final
    {
        std::vector<Id> sourceVids, targetVids;

        bool empty() const;
        bool empty(bool _source) const;
        const std::vector<Id>& vids(bool _source) const;
        std::vector<Id>& vids(bool _source);
    };

    class SurfaceExporter final
    {

    private:

        std::unordered_map<Id, Id> m_s2v, m_v2s;

    public:

        cinolib::Polygonmesh<> surf;
        Meshing::Mesher::Mesh vol;

        SurfaceExporter(const Meshing::Mesher::Mesh& _mesh);

        void applySurfToVol();

        void applyVolToSurf();

        Id toSurfVid(Id _volVid) const;

        Id toVolVid(Id _surfVid) const;

        Id toSurfEid(Id _volEid) const;

        Id toVolEid(Id _surfEid) const;

        std::vector<Id> onSurfVolVids() const;

        static std::vector<Id> onSurfVids(const Meshing::Mesher::Mesh& _mesh);

    };

    void normalizeWeights(std::vector<Real>& _weights);
    void invertAndNormalizeDistances(std::vector<Real>& _distances);

    template<typename M, typename V, typename E, typename P>
    bool isEidsPathClosed(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids);

    bool isVidsPathClosed(const std::vector<Id>& _vids);

    std::vector<I> vidsPathAdjVidsI(const std::vector<Id>& _vids, I _i);

    std::vector<Id> vidsPathAdjVids(const std::vector<Id>& _vids, I _i);

    template<typename M, typename V, typename E, typename P>
    std::vector<I> eidsPathAdjEidsI(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids, I _i);

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> eidsPathAdjEids(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids, I _i);

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> vidsToEidsPath(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _vids);

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> eidsToVidsPath(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids);

    std::vector<Point> toSurfFeats(const std::vector<Point>& _feats, const SurfaceExporter& _exporter);

    std::vector<EidsPath> toSurfFeats(const std::vector<EidsPath>& _feats, const SurfaceExporter& _exporter);

    template<typename MS, typename VS, typename ES, typename PS, typename MT, typename VT, typename ET, typename PT>
    std::vector<VidsPath> eidsToVidsPaths(const std::vector<EidsPath>& _paths, const cinolib::AbstractMesh <MS, VS, ES, PS>& _source, const cinolib::AbstractMesh<MT, VT, ET, PT>& _target);

    void setVerts(const std::vector<Vec>& _from, std::vector<Vec>& _to, const std::vector<Id>& _vids);

    void setVerts(std::vector<Vec>& _source, const std::vector<Vec>& _target, const std::vector<Point>& _feats);

    void setSourceVerts(const std::vector<std::vector<Vec>>& _from, std::vector<Vec>& _to, const std::vector<VidsPath>& _vids);

    template<typename M, typename V, typename E, typename P>
    std::vector<Id> eidsPathEndVids(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Id>& _eids);

    template<typename MS, typename VS, typename ES, typename PS, typename MT, typename VT, typename ET, typename PT>
    std::vector<Point> endPoints(const EidsPath& _path, const cinolib::AbstractMesh <MS, VS, ES, PS>& _source, const cinolib::AbstractMesh<MT, VT, ET, PT>& _target);

}

#define HMP_PROJECTION_UTILS_IMPL
#include <HMP/Projection/Utils.tpp>
#undef HMP_PROJECTION_UTILS_IMPL