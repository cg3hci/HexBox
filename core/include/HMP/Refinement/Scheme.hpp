#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <vector>
#include <map>

namespace HMP::Refinement
{

	class Scheme final: public cpputils::mixins::ReferenceClass
	{

	private:

		static bool compareIVec2(const IVec2& _a, const IVec2& _b);

	public:

		using FaceSurfVisMap = std::map<IVec2, I, decltype(&compareIVec2)>;

	private:

		std::vector<I> findSurfVis() const;
		std::vector<I> findCornerVis() const;
		FaceSurfVisMap findFacesSurfVisIs(Id _dim, bool _polarity) const;
		HexFaceData<FaceSurfVisMap> findFacesSurfVisIs() const;

		const HexVertData<IVec> m_vidCorners;

	public:

		const I gridSize;
		const std::vector<IVec> verts;
		const std::vector<HexVertIs> polys;
		const std::vector<I> surfVis;
		const std::vector<I> cornerVis;
		const HexFaceData<FaceSurfVisMap> facesSurfVisIs;

		explicit Scheme(I _gridSize, const std::vector<IVec>& _verts, const std::vector<HexVertIs>& _polys);

		bool isMin(I _comp) const;

		bool isMax(I _comp) const;

		bool isExtreme(I _comp) const;

		bool isOnSurf(const IVec& _vert) const;

		bool isCorner(const IVec& _vert) const;

		I cornerVi(const IVec& _corner) const;

	};

}
