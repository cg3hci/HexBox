#include <HMP/Projection/percentileAdvance.hpp>

#include <algorithm>
#include <limits>
#include <cinolib/parallel_for.h>

namespace HMP::Projection
{

    static constexpr unsigned int c_minVertsForParallelFor{ 1024 };

    std::vector<Vec> percentileAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, const double _percentile)
    {
        std::vector<Vec> out;
        percentileAdvance(_from, _to, out, _percentile);
        return out;
    }

    void percentileAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, std::vector<Vec>& _out, const double _percentile)
    {
        _out.resize(_from.size());
        Real maxLength{};
        {
            std::vector<Real> lengths(_from.size());
            const auto func{ [&](const Id _id) {
                const I i{ toI(_id) };
                lengths[i] = (_to[i] - _from[i]).norm();
            } };
            cinolib::PARALLEL_FOR(0, toId(_from.size()), c_minVertsForParallelFor, func);
            std::sort(lengths.begin(), lengths.end());
            I medianI{ static_cast<I>(std::round(static_cast<double>(lengths.size() - 1) * _percentile)) };
            if (medianI <= lengths.size())
            {
                maxLength = lengths[medianI];
            }
            else
            {
                maxLength = std::numeric_limits<Real>::infinity();
            }
        }
        const auto func{ [&](const Id _id) {
            const I i{ toI(_id) };
            const Vec offset{ _to[i] - _from[i] };
            const Vec clampedOffset{ offset.norm() <= maxLength ? offset : (offset.normalized() * maxLength) };
            _out[i] = _from[i] + clampedOffset;
        } };
        cinolib::PARALLEL_FOR(0, toId(_from.size()), c_minVertsForParallelFor, func);
    }

}
