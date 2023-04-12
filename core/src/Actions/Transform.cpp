#include <HMP/Actions/Transform.hpp>

#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <algorithm>
#include <utility>
#include <cpputils/range/of.hpp>
#include <cpputils/range/zip.hpp>

namespace HMP::Actions
{

	void Transform::apply()
	{
		const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
		if (!m_prepared)
		{
			m_prepared = true;
			if (m_vids)
			{
				m_otherVerts.reserve(m_vids->size());
				for (const Id vid : *m_vids)
				{
					m_otherVerts.push_back(m_transform * mesh.vert(vid));
				}
			}
			else
			{
				m_otherVerts = mesh.vector_verts();
				for (Vec& vert : m_otherVerts)
				{
					vert = m_transform * vert;
				}
			}
		}
		if (m_vids)
		{
			for (const auto& [vid, vert] : cpputils::range::zip(*m_vids, m_otherVerts))
			{
				const Vec oldVert{ mesh.vert(vid) };
				mesher().moveVert(vid, vert);
				vert = oldVert;
			}
		}
		else
		{
			std::vector<Vec> oldVerts{ mesh.vector_verts() };
			for (I vi{}; vi < oldVerts.size(); vi++)
			{
				mesher().moveVert(toId(vi), m_otherVerts[vi]);
			}
			m_otherVerts.swap(oldVerts);
		}
		mesher().updateMesh();
	}

	void Transform::unapply()
	{
		apply();
	}

	Transform::Transform(const Mat4& _transform, const std::optional<std::vector<Id>>& _vids)
		: m_transform{ _transform }, m_vids{ _vids }, m_otherVerts{}, m_prepared{ false }
	{}

	const Mat4& Transform::transform() const
	{
		return m_transform;
	}

	const std::optional<std::vector<Id>>& Transform::vids() const
	{
		return m_vids;
	}

}