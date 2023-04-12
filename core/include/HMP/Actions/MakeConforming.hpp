#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>
#include <vector>
#include <unordered_map>
#include <set>
#include <cpputils/range/of.hpp>
#include <utility>

namespace HMP::Actions
{

	class MakeConforming final: public Commander::Action
	{

	private:

		static std::pair<const Dag::Refine&, const Dag::Element&> dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>& _pair);

		std::vector<std::pair<const Dag::NodeHandle<Dag::Refine>, Dag::Element* const>> m_operations;
		Meshing::Mesher::State m_oldState;
		bool m_prepared;

		void apply() override;
		void unapply() override;

		void installSub3x3Adapters();

	public:

		using Operations = decltype(cpputils::range::ofc(m_operations).map(&dereferenceOperation));

		MakeConforming();

		Operations operations() const;

	};

}
