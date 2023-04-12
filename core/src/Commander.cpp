#include <HMP/Commander.hpp>

#include <HMP/Project.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cassert>

namespace HMP
{

	// Commander::ActionBase

	Commander::ActionBase::ActionBase()
		: m_commander{}, m_applied{ false }
	{}

	void Commander::ActionBase::attach(Commander& _commander)
	{
		assert(!attached());
		m_commander = &_commander;
	}

	void Commander::ActionBase::prepareAndApply()
	{
		assert(attached());
		assert(!m_applied);
		m_applied = true;
		apply();
	}

	void Commander::ActionBase::prepareAndUnapply()
	{
		assert(attached());
		assert(m_applied);
		m_applied = false;
		unapply();
	}

	Meshing::Mesher& Commander::ActionBase::mesher()
	{
		assert(attached());
		return m_commander->m_project.mesher();
	}

	const Meshing::Mesher& Commander::ActionBase::mesher() const
	{
		assert(attached());
		return m_commander->m_project.mesher();
	}

	Dag::NodeHandle<Dag::Element>& Commander::ActionBase::root()
	{
		assert(attached());
		return m_commander->m_project.root();
	}

	const Dag::Element* Commander::ActionBase::root() const
	{
		assert(attached());
		return m_commander->m_project.root();
	}

	bool Commander::ActionBase::attached() const
	{
		return m_commander;
	}

	bool Commander::ActionBase::applied() const
	{
		return m_applied;
	}

	// Commander::StackBase

	Commander::StackBase::StackBase()
		: HMP::Utils::ConstDerefRanged<std::deque<Action*>>{ m_data }, m_data{}, m_limit{ 1000 }
	{}

	Commander::Action& Commander::StackBase::pop()
	{
		assert(!empty());
		Action& action{ *m_data.front() };
		m_data.pop_front();
		return action;
	}

	void Commander::StackBase::push(Action& _action)
	{
		m_data.push_front(&_action);
		keepLatest(m_limit);
	}

	I Commander::StackBase::limit() const
	{
		return m_limit;
	}

	void Commander::StackBase::limit(I _count)
	{
		m_limit = _count;
		keepLatest(m_limit);
	}

	void Commander::StackBase::removeOldest(I _count)
	{
		while (!m_data.empty() && _count > 0)
		{
			Action* action{ m_data.back() };
			delete action;
			m_data.pop_back();
			_count--;
		}
	}

	void Commander::StackBase::keepLatest(I _count)
	{
		while (m_data.size() > _count)
		{
			Action* action{ m_data.back() };
			delete action;
			m_data.pop_back();
		}
	}

	void Commander::StackBase::clear()
	{
		for (Action* action : m_data)
		{
			delete action;
		}
		m_data.clear();
	}

	// Commander

	Commander::Commander(Project& _project)
		: m_project{ _project }, m_applied{}, m_unapplied{}
	{}

	Commander::~Commander()
	{
		m_unapplied.clear();
		m_applied.clear();
	}

	void Commander::apply(Action& _action)
	{
		_action.attach(*this);
		m_unapplied.clear();
		_action.apply();
		m_applied.push(_action);
	}

	void Commander::undo()
	{
		assert(canUndo());
		Action& action{ m_applied.pop() };
		action.unapply();
		m_unapplied.push(action);
	}

	void Commander::redo()
	{
		assert(canRedo());
		Action& action{ m_unapplied.pop() };
		action.apply();
		m_applied.push(action);
	}

	bool Commander::canUndo() const
	{
		return !m_applied.empty();
	}

	bool Commander::canRedo() const
	{
		return !m_unapplied.empty();
	}


	Commander::Stack& Commander::unapplied()
	{
		return m_unapplied;
	}

	const Commander::Stack& Commander::unapplied() const
	{
		return m_unapplied;
	}

	Commander::Stack& Commander::applied()
	{
		return m_applied;
	}

	const Commander::Stack& Commander::applied() const
	{
		return m_applied;
	}

}