#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Refine.hpp>
#include <vector>
#include <utility>

namespace HMP::Actions
{

	class Refine final: public Commander::Action
	{

	private:

		const std::vector<std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>> m_operations;
		const I m_depth;
		Meshing::Mesher::State m_oldState;

		void apply() override;
		void unapply() override;

	public:

		Refine(Dag::Element& _element, I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth = 1);

		const Dag::Element& element() const;
		const Dag::Refine& operation() const;
		I depth() const;

	};

}
