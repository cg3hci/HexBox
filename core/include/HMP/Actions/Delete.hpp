#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Delete.hpp>

namespace HMP::Actions
{

	class Delete final: public Commander::Action
	{

	private:

		Dag::Element& m_element;
		const Dag::NodeHandle<Dag::Delete> m_operation;

		void apply() override;
		void unapply() override;

	public:

		Delete(Dag::Element& _element);

		const Dag::Element& element() const;
		const Dag::Delete& operation() const;

	};

}
