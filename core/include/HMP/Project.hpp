#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP
{

	class Project: public cpputils::mixins::ReferenceClass
	{

	private:

		Meshing::Mesher m_mesher;
		Commander m_commander;
		Dag::NodeHandle<Dag::Element> m_root;

	public:

		Project();

		Commander& commander();
		const Commander& commander() const;

		Dag::NodeHandle<Dag::Element>& root();
		const Dag::Element* root() const;

		Meshing::Mesher& mesher();
		const Meshing::Mesher& mesher() const;

	};

}