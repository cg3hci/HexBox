#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Delete.hpp>
#include <vector>
#include <utility>
#include <cpputils/range/of.hpp>

namespace HMP::Actions
{

	class DeleteSome final : public Commander::Action
	{

	private:


		static std::pair<const Dag::Delete&, const Dag::Element&> dereferenceOperation(const std::pair<Dag::NodeHandle<Dag::Delete>, Dag::Element*>& _pair);

		const std::vector<std::pair<const Dag::NodeHandle<Dag::Delete>, Dag::Element* const>> m_operations;

		std::vector<std::pair<const Dag::NodeHandle<Dag::Delete>, Dag::Element* const>> prepare(const std::vector<Dag::Element*>& _elements);
		void apply() override;
		void unapply() override;

	public:

		using Operations = decltype(cpputils::range::ofc(m_operations).map(&dereferenceOperation));

		DeleteSome(const std::vector<Dag::Element*>& _elements);

		Operations operations() const;

	};

}
