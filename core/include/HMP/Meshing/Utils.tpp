#ifndef HMP_MESHING_UTILS_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Meshing/Utils.hpp>

namespace HMP::Meshing::Utils
{

	template<I TOutSize, I TInSize, typename TValue>
	std::array<TValue, TOutSize> index(const std::array<TValue, TInSize>& _source, const std::array<I, TOutSize>& _is)
	{
		std::array<TValue, TOutSize> out;
		for (I i{}; i < _is.size(); i++)
		{
			out[i] = _source[_is[i]];
		}
		return out;
	}

	template<I TSize>
	std::array<Vec, TSize> verts(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, TSize>& _vids, const std::vector<Vec>& _newVerts)
	{
		return cpputils::range::of(_vids).map([&](const Id _vid) {
			return _vid >= _mesh.num_verts() ? _newVerts[toI(_vid - _mesh.num_verts())] : _mesh.vert(_vid);
		}).toArray();
	}

	template<I TSize>
	Vec centroid(const std::array<Vec, TSize>& _verts)
	{
		Vec sum{};
		for (const Vec& vert : _verts) { sum += vert; }
		return sum / static_cast<Real>(TSize);
	}

}