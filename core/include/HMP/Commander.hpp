#pragma once

#include <deque>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Utils/DerefRanged.hpp>
#include <HMP/Meshing/Mesher.hpp>

namespace HMP
{

	class Project;

	class Commander final: public cpputils::mixins::ReferenceClass
	{

	public:

		class Action;

		class ActionBase: public cpputils::mixins::ReferenceClass
		{

		private:

			friend class Action;

			Commander* m_commander;
			bool m_applied;

			ActionBase();

			void attach(Commander& _commander);

			void prepareAndApply();
			void prepareAndUnapply();

		protected:

			virtual ~ActionBase() = default;

			Meshing::Mesher& mesher();
			const Meshing::Mesher& mesher() const;

			Dag::NodeHandle<Dag::Element>& root();
			const Dag::Element* root() const;

			virtual void apply() = 0;
			virtual void unapply() = 0;

		public:

			bool attached() const;
			bool applied() const;

		};

		class Stack;

		class StackBase: public cpputils::mixins::ReferenceClass, public HMP::Utils::ConstDerefRanged<std::deque<Action*>>
		{

		private:

			friend class Stack;

			std::deque<Action*> m_data;
			I m_limit;

			StackBase();

			Action& pop();
			void push(Action& _action);

		public:

			I limit() const;
			void limit(I _count);
			void removeOldest(I _count);
			void keepLatest(I _count);
			void clear();

		};

		class Action: public ActionBase
		{

		private:

			friend class Commander;

			using ActionBase::attach;
			using ActionBase::prepareAndApply;
			using ActionBase::prepareAndUnapply;

		public:

			using ActionBase::ActionBase;

		};

		class Stack final: public StackBase
		{

		private:

			friend class Commander;

			using StackBase::StackBase;
			using StackBase::pop;
			using StackBase::push;

		};

	private:

		Project& m_project;
		Stack m_applied, m_unapplied;

	public:

		Commander(Project& _project);
		~Commander();

		void apply(Action& _action);

		void undo();
		void redo();

		bool canUndo() const;
		bool canRedo() const;

		Stack& unapplied();
		const Stack& unapplied() const;

		Stack& applied();
		const Stack& applied() const;


	};

}