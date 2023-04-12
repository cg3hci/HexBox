#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <vector>

namespace HMP::Actions
{

	class Root final: public Commander::Action
	{

	private:

		Dag::Element& m_newRoot;
		const std::vector<Vec> m_newVerts;
		Dag::NodeHandle<Dag::Element> m_otherRoot;
		std::vector<Vec> m_otherVerts;

		void apply() override;
		void unapply() override;

	public:

		Root(Dag::Element& _root, const std::vector<Vec>& _verts);

		const Dag::Element& newRoot() const;

		const std::vector<Vec>& newVerts() const;

	};

}
