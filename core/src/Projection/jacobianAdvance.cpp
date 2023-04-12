#include <HMP/Projection/jacobianAdvance.hpp>

#include <cpputils/unreachable.hpp>
#include <algorithm>
#include <limits>
#include <cinolib/parallel_for.h>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/index.hpp>
#include <cinolib/quality_hex.h>
#include <unordered_set>
#include <unordered_map>

#define HMP_PROJECTION_JACOBIANADVANCE_LOG

namespace HMP::Projection
{

    template<typename TIt>
    std::vector<Vec> offsets(const Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _from, const TIt& _vids)
    {
        std::vector<Vec> offs;
        offs.reserve(static_cast<I>(_vids.end() - _vids.begin()));
        for (const auto& [vid, fromVert] : cpputils::range::zip(_vids, _from))
        {
            offs.push_back(_mesh.vert(vid) - fromVert);
        }
        return offs;
    }

    std::vector<Real> lengths(const std::vector<Vec>& _offs)
    {
        std::vector<Real> lens(_offs.size());
        for (I i{}; i < _offs.size(); i++)
        {
            lens[i] = _offs[i].norm();
        }
        return lens;
    }

    Vec displace(const Vec& _from, const Vec& _off, const Real _len, const Real _maxLength, const EJacobianAdvanceMode _mode, const Real _progress)
    {
        if (_len == 0.0)
        {
            return _from;
        }
        switch (_mode)
        {
            case EJacobianAdvanceMode::Length:
            {
                const Real maxProgressLen{ _maxLength * _progress };
                return _len <= maxProgressLen
                    ? _from + _off
                    : _from + _off * maxProgressLen / _len;
            }
            case EJacobianAdvanceMode::Lerp:
            {
                return _from + _progress * _off / _len;
            }
            default:
                cpputils::unreachable();
        }
    }

    bool isOk(const Meshing::Mesher::Mesh& _mesh, const Id _pid)
    {
        const std::vector<Vec>& verts{ _mesh.poly_verts(_pid) };
        return cinolib::hex_scaled_jacobian(verts[0], verts[1], verts[2], verts[3], verts[4], verts[5], verts[6], verts[7]) >= 0.0;
    }

    template<typename TIt, typename TIt2>
    void jacobianAdvance(Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _from, EJacobianAdvanceMode _mode, I _maxTests, Real _stopThreshold, const TIt& _vids, const std::function<I(Id)>& _vidToVidsI, const TIt2& _pids)
    {
        const std::vector<Vec> offs{ offsets(_mesh, _from, _vids) };
        const std::vector<Real> lens{ lengths(offs) };
        const Real maxLen{ lens.empty() ? 0.0 : *std::max_element(lens.begin(), lens.end()) };
        Real minProg{ 0.0 }, maxProg{ 1.0 };
        Id failurePid{ noId };
        I tests{};
#ifdef HMP_PROJECTION_JACOBIANADVANCE_LOG
        std::cout << "Jacobian Advance:" << std::endl;
#endif
        do
        {
            const Real prog{ tests > 0 ? (maxProg + minProg) / 2.0 : 1.0 };
#ifdef HMP_PROJECTION_JACOBIANADVANCE_LOG
            std::cout << "Pass " << tests << ", prog = [" << minProg << "," << maxProg << "] (" << prog << ")";
            if (failurePid != noId)
            {
                std::cout << ", failurePid = " << failurePid;
            }
#endif
            if (failurePid != noId)
            {
                for (const Id vid : _mesh.adj_p2v(failurePid))
                {
                    const I vidsI{ _vidToVidsI(vid) };
                    if (vidsI != static_cast<I>(-1))
                    {
                        _mesh.vert(vid) = displace(_from[vidsI], offs[vidsI], lens[vidsI], maxLen, _mode, prog);
                    }
                }
                if (isOk(_mesh, failurePid))
                {
                    failurePid = noId;
                }
            }
            if (failurePid == noId)
            {
                for (const auto& [from, off, len, vid] : cpputils::range::zip(_from, offs, lens, _vids))
                {
                    _mesh.vert(vid) = displace(from, off, len, maxLen, _mode, prog);
                }
                for (const Id pid : _pids)
                {
                    if (!_mesh.poly_data(pid).flags[cinolib::HIDDEN] && !isOk(_mesh, pid))
                    {
                        failurePid = pid;
                        break;
                    }
                }
            }
            if (failurePid == noId)
            {
                minProg = prog;
            }
            else if (tests > 0)
            {
                maxProg = prog;
            }
#ifdef HMP_PROJECTION_JACOBIANADVANCE_LOG
            std::cout << ": ";
            if (failurePid == noId)
            {
                std::cout << "ok";
            }
            else
            {
                std::cout << "failurePid = " << failurePid;
            }
            std::cout << ", prog = " << "[" << minProg << "," << maxProg << "]" << std::endl;
#endif
        }
        while (maxProg - minProg > _stopThreshold && ++tests < _maxTests);
        if (failurePid != noId)
        {
#ifdef HMP_PROJECTION_JACOBIANADVANCE_LOG
            std::cout << "falling back to prog = " << minProg << std::endl;
#endif
            for (const auto& [from, off, len, vid] : cpputils::range::zip(_from, offs, lens, _vids))
            {
                _mesh.vert(vid) = displace(from, off, len, maxLen, _mode, minProg);
            }
        }
    }

    void jacobianAdvance(Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _from, EJacobianAdvanceMode _mode, I _maxTests, Real _stopThreshold)
    {
        const auto vids{ cpputils::range::count<Id>(_mesh.num_verts()) };
        const auto pids{ cpputils::range::count<Id>(_mesh.num_polys()) };
        jacobianAdvance(_mesh, _from, _mode, _maxTests, _stopThreshold, vids, toI, pids);
    }

    void jacobianAdvance(Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _from, const std::vector<Id>& _vids, EJacobianAdvanceMode _mode, I _maxTests, Real _stopThreshold)
    {
        std::unordered_set<Id> pids;
        for (const Id vid : _vids)
        {
            for (const Id adjPid : _mesh.adj_v2p(vid))
            {
                if (!_mesh.poly_data(adjPid).flags[cinolib::HIDDEN])
                {
                    pids.insert(adjPid);
                }
            }
        }
        std::unordered_map<Id, I> vidToVidsIMap;
        for (I i{}; i < _vids.size(); i++)
        {
            vidToVidsIMap.emplace(_vids[i], i);
        }
        const auto vidToVidsI{ [&vidToVidsIMap](const Id _vid)
        {
            const auto it { vidToVidsIMap.find(_vid) };
            if (it != vidToVidsIMap.end())
            {
                return it->second;
            }
            return static_cast<I>(-1);
        } };
        jacobianAdvance(_mesh, _from, _mode, _maxTests, _stopThreshold, _vids, vidToVidsI, pids);
    }

}
