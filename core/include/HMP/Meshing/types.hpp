#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <cstddef>
#include <array>

namespace HMP
{

	using Id = unsigned int;
	using I = std::size_t;
	using Real = double;

	using IVec = cinolib::vec<3, I>;
	using IVec2 = cinolib::vec<2, I>;

	using Vec = cinolib::vec<3, Real>;
	using Vec2 = cinolib::vec<2, Real>;
	using Vec4 = cinolib::vec<4, Real>;
	using Mat4 = cinolib::mat4d;
	using Mat3 = cinolib::mat3d;

	constexpr Id noId{ static_cast<Id>(-1) };

	inline constexpr Id toId(I _i) { return static_cast<Id>(_i); }
	inline constexpr I toI(Id _id) { return static_cast<I>(_id); }

	template<typename TData>
	using HexFaceData = std::array<TData, 6>;

	template<typename TData>
	using HexEdgeData = std::array<TData, 12>;

	template<typename TData>
	using HexVertData = std::array<TData, 8>;

	template<typename TData>
	using QuadVertData = std::array<TData, 4>;

	template<typename TData>
	using QuadEdgeData = std::array<TData, 4>;

	template<typename TData>
	using EdgeVertData = std::array<TData, 2>;

	using HexVerts = HexVertData<Vec>;
	using HexVertIds = HexVertData<Id>;
	using HexVertIs = HexVertData<I>;
	using HexFaceIds = HexFaceData<Id>;
	using HexFaceIs = HexFaceData<I>;
	using HexEdgeIds = HexEdgeData<Id>;
	using HexEdgeIs = HexEdgeData<I>;
	using QuadVerts = QuadVertData<Vec>;
	using QuadVertIds = QuadVertData<Id>;
	using QuadVertIs = QuadVertData<I>;
	using QuadEdgeIds = QuadEdgeData<Id>;
	using QuadEdgeIs = QuadEdgeData<I>;
	using EdgeVerts = EdgeVertData<Vec>;
	using EdgeVertIds = EdgeVertData<Id>;
	using EdgeVertIs = EdgeVertData<I>;

}