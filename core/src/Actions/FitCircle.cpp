#include <HMP/Actions/FitCircle.hpp>

#include <cpputils/range/zip.hpp>
#include <cpputils/range/of.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cinolib/geometry/plane.h>
#include <cinolib/pi.h>
#include <utility>
#include <cmath>
#include <algorithm>

namespace HMP::Actions
{

	void FitCircle::apply()
	{
		const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
		std::vector<Vec> oldVerts{};
		oldVerts.reserve(m_vids.size());
		for (const Id vid : m_vids)
		{
			oldVerts.push_back(mesh.vert(vid));
		}
		if (!m_prepared)
		{
			m_prepared = true;
			const cinolib::Plane plane{ oldVerts };
			const Real radius{ cpputils::range::of(oldVerts).map([&](const Vec& _v) { return _v.dist(plane.p); }).avg() };
			const Vec baseY{ (oldVerts[0] - plane.p).cross(plane.n).normalized() };
			const Vec baseX{ plane.n.cross(baseY).normalized() };
			std::vector<std::pair<I, Real>> isAndAs{
				cpputils::range::enumerate(oldVerts)
					.map([&](const std::tuple<I, Vec>& _e)
						{
							const auto& [i, v] { _e };
							const Real x{ baseX.dot(v - plane.p) };
							const Real y{ baseY.dot(v - plane.p) };
							const Real a{ std::atan2(y, x) };
							return std::pair<I, Real>{i, a};
						})
					.toVector()
			};
			std::sort(isAndAs.begin(), isAndAs.end(), [](const std::pair<I, Real>& _a, const std::pair<I, Real>& _b)
				{
					return _a.second < _b.second;
				});
			I firstI{};
			for (const auto& [i, e] : cpputils::range::enumerate(isAndAs))
			{
				if (e.first == 0)
				{
					firstI = i;
					break;
				}
			}
			std::vector<I> is{ cpputils::range::of(isAndAs).map([](const std::pair<I, Real>& _e) { return _e.first; }).toVector() };
			std::rotate(is.begin(), is.begin() + firstI, is.end());
			for (I i{}; i < is.size(); i++)
			{
				const Real angle{ 2 * M_PI * static_cast<Real>(i) / static_cast<Real>(is.size()) };
				m_otherVerts[is[i]] = plane.p + (baseX * std::cos(angle) + baseY * std::sin(angle)) * radius;
			}
		}
		for (const auto& [vid, pos] : cpputils::range::zip(m_vids, m_otherVerts))
		{
			mesher().moveVert(vid, pos);
		}
		m_otherVerts.swap(oldVerts);
		mesher().updateMesh();
	}

	void FitCircle::unapply()
	{
		apply();
	}

	FitCircle::FitCircle(const std::vector<Id>& _vids)
		: m_vids{ _vids }, m_prepared{ false }, m_otherVerts(_vids.size())
	{
		assert(m_vids.size() > 2);
	}

	const std::vector<Id>& FitCircle::vids() const
	{
		return m_vids;
	}

}