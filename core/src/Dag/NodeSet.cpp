#include <HMP/Dag/NodeSet.hpp>

#include <cpputils/range/of.hpp>

namespace HMP::Dag
{

	namespace Internal
	{

		// NodeSetData

		bool NodeSetData::add(Node& _node)
		{
			const auto it{ m_map.find(&_node) };
			if (it != m_map.end())
			{
				return false;
			}
			else
			{
				m_list.push_back(&_node);
				m_map.insert(it, { &_node, --m_list.end() });
				return true;
			}
		}

		bool NodeSetData::remove(Node& _node)
		{
			const auto it{ m_map.find(&_node) };
			if (it != m_map.end())
			{
				m_list.erase(it->second);
				m_map.erase(it);
				return true;
			}
			else
			{
				return false;
			}
		}

		bool NodeSetData::clear()
		{
			const bool wasEmpty{ m_map.empty() };
			m_map.clear();
			m_list.clear();
			return !wasEmpty;
		}

		// NodeSetHandle

		NodeSetHandle::NodeSetHandle(NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll)
			: m_data{ _data }, m_onAttach{ _onAttach }, m_onDetach{ _onDetach }, m_onDetachAll{ _onDetachAll }
		{}

	}

}