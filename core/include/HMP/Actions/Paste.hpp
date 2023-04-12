#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <HMP/Meshing/Mesher.hpp>

namespace HMP::Actions
{

	class Paste final: public Commander::Action
	{

	private:

		const cpputils::collections::FixedVector<Dag::Element*, 3> m_elements;
		const Dag::Extrude& m_sourceOperation;
		const cpputils::collections::FixedVector<I, 3>& m_fis;
		const I m_firstVi;
		const bool m_clockwise;
		Dag::NodeHandle<Dag::Extrude> m_operation;
		Meshing::Mesher::State m_oldState;
		bool m_prepared{ false };
		std::vector<Vec> m_newVerts;

		void apply() override;

		void unapply() override;

	public:

		using Elements = decltype(cpputils::range::ofc(m_elements).dereference().immutable());

		Paste(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise, const Dag::Extrude& _source);

		Elements elements() const;

		const Dag::Extrude& operation() const;

	};

}
