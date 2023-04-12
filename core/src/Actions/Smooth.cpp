#include <HMP/Actions/Smooth.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <HMP/Projection/smooth.hpp>
#include <cinolib/export_surface.h>
#include <cinolib/meshes/polygonmesh.h>
#include <cpputils/range/of.hpp>
#include <cpputils/range/enumerate.hpp>
#include <unordered_map>

namespace HMP::Actions
{

	void Smooth::apply()
	{
		if (!m_prepared)
		{
			m_prepared = true;
			cinolib::Polygonmesh<> surf;
			std::unordered_map<Id, Id> surf2vol, vol2surf;
			cinolib::export_surface(mesher().mesh(), surf, vol2surf, surf2vol, false);
			const std::vector<Id> surfVids{ cpputils::range::of(surf2vol).map([](const auto& _it) {
				return std::get<1>(_it);
			}).toVector() };
			m_otherVerts = mesher().mesh().vector_verts();
			for (I i{}; i < m_surfIterations; ++i)
			{
				const std::vector<Vec>& newSurfVerts{ Projection::smooth(surf) };
				for (const auto& [vi, vert] : cpputils::range::enumerate(newSurfVerts))
				{
					m_otherVerts[surf2vol.at(toId(vi))] = vert;
				}
			}
			for (I i{}; i < m_internalIterations; ++i)
			{
				m_otherVerts = Projection::smoothInternal(mesher().mesh(), surfVids, m_surfVertWeight);
			}
		}
		for (I vi{}; vi < m_otherVerts.size(); vi++)
		{
			const Vec oldVert{ mesher().mesh().vert(toId(vi)) };
			mesher().moveVert(toId(vi), m_otherVerts[vi]);
			m_otherVerts[vi] = oldVert;
		}
		mesher().updateMesh();
	}

	void Smooth::unapply()
	{
		apply();
	}

	Smooth::Smooth(I _surfaceIterations, I _internalIterations, Real _surfVertWeight)
		: m_surfIterations{ _surfaceIterations }, m_internalIterations{ _internalIterations }, m_surfVertWeight{_surfVertWeight}, m_prepared{ false }
	{}

	const I Smooth::surfaceIterations() const
	{
		return m_surfIterations;
	}

	const I Smooth::internalIterations() const
	{
		return m_internalIterations;
	}

	const Real Smooth::surfVertWeight() const
	{
		return m_surfVertWeight;
	}

}