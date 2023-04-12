#include <HMP/Refinement/Scheme.hpp>

#include <cpputils/unreachable.hpp>
#include <cinolib/geometry/lerp.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <limits>
#include <cpputils/range/of.hpp>
#include <utility>
#include <cassert>

namespace HMP::Refinement
{

	bool Scheme::compareIVec2(const IVec2& _a, const IVec2& _b)
	{
		return (_a.x() == _b.x())
			? (_a.y() < _b.y())
			: (_a.x() < _b.x());
	}

	std::vector<I> Scheme::findSurfVis() const
	{
		return cpputils::range::of(verts)
			.enumerate()
			.filter([&](const auto& _vertAndVi) { return isOnSurf(std::get<0>(_vertAndVi)); })
			.map([](const auto& _vertAndVi) { return std::get<1>(_vertAndVi); })
			.toVector();
	}

	std::vector<I> Scheme::findCornerVis() const
	{
		return cpputils::range::of(surfVis)
			.filter([&](const I _vi) { return isCorner(verts[_vi]); })
			.toVector();
	}

	IVec2 removeDim(const IVec& _vert, Id _dim)
	{
		switch (_dim)
		{
			case 0:
				return { _vert.y(), _vert.z() };
			case 1:
				return { _vert.x(), _vert.z() };
			case 2:
				return { _vert.x(), _vert.y() };
			default:
				cpputils::unreachable();
		}
	}

	Scheme::FaceSurfVisMap Scheme::findFacesSurfVisIs(Id _dim, bool _polarity) const
	{
		FaceSurfVisMap map{ &compareIVec2 };
		for (I surfViI{}; surfViI < surfVis.size(); surfViI++)
		{
			const I vi{ surfVis[surfViI] };
			const IVec& vert{ verts[vi] };
			{
				const I comp{ vert[_dim] };
				if (_polarity ? isMax(comp) : isMin(comp))
				{
					if (!isCorner(vert))
					{
						map.insert({ removeDim(vert, _dim), surfViI });
					}
				}
			}
		}
		return map;
	}

	HexFaceData<Scheme::FaceSurfVisMap> Scheme::findFacesSurfVisIs() const
	{
		static constexpr HexFaceData<std::pair<Id, bool>> surfaceDimAndPolarity{ {
			{2, false},
			{2, true},
			{0, true},
			{0, false},
			{1, false},
			{1, true}
		} };
		return cpputils::range::of(surfaceDimAndPolarity)
			.map([&](const auto& _dimAndPolarity) { return findFacesSurfVisIs(std::get<0>(_dimAndPolarity), std::get<1>(_dimAndPolarity)); })
			.toArray();
	}

	Scheme::Scheme(I _gridSize, const std::vector<IVec>& _verts, const std::vector<HexVertIs>& _polys)
		: m_vidCorners{
			IVec{0, 0, 0},
			IVec{_gridSize, 0, 0},
			IVec{_gridSize, _gridSize, 0},
			IVec{0, _gridSize, 0},
			IVec{0, 0, _gridSize},
			IVec{_gridSize, 0, _gridSize},
			IVec{_gridSize, _gridSize, _gridSize},
			IVec{0, _gridSize, _gridSize}
		},
		gridSize{ _gridSize }, verts{ _verts }, polys{ _polys }, surfVis{ findSurfVis() }, cornerVis{ findCornerVis() }, facesSurfVisIs{ findFacesSurfVisIs() }
	{}

	bool Scheme::isMin(I _comp) const
	{
		return _comp == 0;
	}

	bool Scheme::isMax(I _comp) const
	{
		return _comp == gridSize;
	}

	bool Scheme::isExtreme(I _comp) const
	{
		return isMin(_comp) || isMax(_comp);
	}

	bool Scheme::isOnSurf(const IVec& _vert) const
	{
		return isExtreme(_vert.x()) || isExtreme(_vert.y()) || isExtreme(_vert.z());
	}

	bool Scheme::isCorner(const IVec& _vert) const
	{
		return isExtreme(_vert.x()) && isExtreme(_vert.y()) && isExtreme(_vert.z());
	}

	I Scheme::cornerVi(const IVec& _corner) const
	{
		for (I vi{}; vi < 8; vi++)
		{
			if (_corner == m_vidCorners[vi])
			{
				return vi;
			}
		}
		cpputils::unreachable();
	}

}