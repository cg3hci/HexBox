#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <vector>
#include <utility>

namespace HMP::Actions
{

	class SplitPlane final : public Commander::Action
	{

	private:

		static std::pair<const Dag::Refine&, const Dag::Element&> dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>& _pair);

		std::vector<std::pair<const Dag::NodeHandle<Dag::Refine>, Dag::Element* const>> m_operations;
		Id m_eid;
		bool m_prepared;
		Meshing::Mesher::State m_oldState;

		void apply() override;
		void unapply() override;

	public:

		using Operations = decltype(cpputils::range::ofc(m_operations).map(&dereferenceOperation));

		SplitPlane(Id _eid);

		Id eid() const;

		Operations operations() const;

	};

}
