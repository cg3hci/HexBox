#include <HMP/Project.hpp>

namespace HMP
{

	Project::Project()
		: m_mesher{}, m_commander{ *reinterpret_cast<Project*>(this) }, m_root{ nullptr }
	{}

	Commander& Project::commander()
	{
		return m_commander;
	}

	const Commander& Project::commander() const
	{
		return m_commander;
	}

	Dag::NodeHandle<Dag::Element>& Project::root()
	{
		return m_root;
	}

	const Dag::Element* Project::root() const
	{
		return m_root;
	}

	Meshing::Mesher& Project::mesher()
	{
		return m_mesher;
	}

	const Meshing::Mesher& Project::mesher() const
	{
		return m_mesher;
	}

}