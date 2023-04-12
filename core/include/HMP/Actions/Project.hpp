#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/meshes/polygonmesh.h>
#include <HMP/Projection/project.hpp>
#include <vector>

namespace HMP::Actions
{

	class Project final: public Commander::Action
	{

	public:

		using TargetMesh = cinolib::Polygonmesh<>;

	private:

		const TargetMesh m_target;
		const std::vector<Projection::Utils::Point> m_pointFeats;
		const std::vector<Projection::Utils::EidsPath> m_pathFeats;
		const Projection::Options m_options;
		std::vector<Vec> m_otherVerts;
		bool m_prepared;

		void apply() override;
		void unapply() override;

	public:

		Project(TargetMesh&& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options);
		Project(const TargetMesh& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options);

		const TargetMesh& target() const;

		const Projection::Options options() const;

	};

}
