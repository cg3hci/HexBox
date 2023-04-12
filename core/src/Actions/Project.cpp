#include <HMP/Actions/Project.hpp>

#include <utility>

namespace HMP::Actions
{

	void Project::apply()
	{
		if (!m_prepared)
		{
			m_prepared = true;
			m_otherVerts = Projection::project(mesher().mesh(), m_target, m_pointFeats, m_pathFeats, m_options);
			if (m_options.vertexMask)
			{
				const std::vector<bool>& mask{*m_options.vertexMask};
				const std::vector<Vec>& oldVerts{mesher().mesh().vector_verts()};
				for (I vi{}; vi < m_otherVerts.size(); vi++)
				{
					if (!mask[vi])
					{
						m_otherVerts[vi] = oldVerts[vi];
					}
				}
			}
		}
		std::vector<Vec> oldVerts{ mesher().mesh().vector_verts() };
		for (I vi{}; vi < m_otherVerts.size(); vi++)
		{
			mesher().moveVert(toId(vi), m_otherVerts[vi]);
		}
		m_otherVerts.swap(oldVerts);
		mesher().updateMesh();
	}

	void Project::unapply()
	{
		apply();
	}

	Project::Project(TargetMesh&& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options)
		: m_target{ std::move(_target) }, m_pointFeats{ _pointFeats }, m_pathFeats{ _pathFeats }, m_options{ _options }, m_prepared{ false }
	{}

	Project::Project(const TargetMesh& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options)
		: m_target{ _target }, m_pointFeats{ _pointFeats }, m_pathFeats{ _pathFeats }, m_options{ _options }, m_prepared{ false }
	{}

	const Project::TargetMesh& Project::target() const
	{
		return m_target;
	}

	const Projection::Options Project::options() const
	{
		return m_options;
	}

}