#include <HMP/Gui/DagViewer/createLayout.hpp>

#include <ogdf/basic/Logger.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/layered/OptimalHierarchyLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/SugiyamaLayout.h>
#include <utility>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::DagViewer
{

	constexpr Real c_nodeRadius{ 1.0 };
	constexpr Real c_lineThickness{ c_nodeRadius / 20.0 };
	constexpr Real c_nodeDistance{ c_nodeRadius + 0.25 };

	std::vector<std::pair<const Dag::Node*, ogdf::node>> populateGraph(const Dag::Node& _dag, ogdf::Graph& _graph)
	{
		std::vector< std::pair<const Dag::Node*, ogdf::node>> dagToGraphNodePairs{};
		std::unordered_map<const Dag::Node*, ogdf::node> dagToGraphNodeMap{};
		// nodes
		{
			const std::vector<const Dag::Node*> dagNodes{ Dag::Utils::descendants(_dag) };
			dagToGraphNodePairs.reserve(dagNodes.size());
			dagToGraphNodeMap.reserve(dagNodes.size());
			for (const Dag::Node* dagNode : dagNodes)
			{
				ogdf::node graphNode{ _graph.newNode() };
				dagToGraphNodePairs.push_back({ dagNode, graphNode });
				dagToGraphNodeMap.insert({ dagNode, graphNode });
			}
		}
		// edges
		{
			for (auto const& [dagNode, graphNode] : dagToGraphNodePairs)
			{
				for (const Dag::Node& dagParent : dagNode->parents)
				{
					const ogdf::node graphParent{ dagToGraphNodeMap[&dagParent] };
					_graph.newEdge(graphParent, graphNode);
				}
			}
		}

		return dagToGraphNodePairs;
	}

	ogdf::GraphAttributes layoutGraph(const ogdf::Graph& _graph)
	{
		ogdf::GraphAttributes graphAttributes(_graph,
			ogdf::GraphAttributes::nodeGraphics |
			ogdf::GraphAttributes::edgeGraphics
		);

		graphAttributes.setAllHeight(c_nodeRadius);
		graphAttributes.setAllWidth(c_nodeRadius);

		ogdf::SugiyamaLayout sugiyamaLayout{};
		sugiyamaLayout.setRanking(new ogdf::OptimalRanking);
		sugiyamaLayout.setCrossMin(new ogdf::MedianHeuristic);

		ogdf::OptimalHierarchyLayout* optimalHierarchyLayout{ new ogdf::OptimalHierarchyLayout };
		optimalHierarchyLayout->layerDistance(c_nodeDistance);
		optimalHierarchyLayout->nodeDistance(c_nodeDistance);
		optimalHierarchyLayout->weightBalancing(0.8);
		sugiyamaLayout.setLayout(optimalHierarchyLayout);

		sugiyamaLayout.call(graphAttributes);

		return graphAttributes;
	}

	std::vector<Layout::Node> createLayoutNodes(const std::vector<std::pair<const Dag::Node*, ogdf::node>>& _dagToGraphNodePairs, const ogdf::GraphAttributes& _graphAttributes)
	{
		std::vector<Layout::Node> nodes{ };
		nodes.reserve(static_cast<I>(_graphAttributes.constGraph().numberOfNodes()));
		for (auto const& [dagNode, graphNode] : _dagToGraphNodePairs)
		{
			const Vec2 center{ _graphAttributes.x(graphNode), -_graphAttributes.y(graphNode) };
			nodes.push_back(Layout::Node{ center, *dagNode });
		}
		return nodes;
	}

	std::vector<std::pair<Vec2, Vec2>> createLayoutLines(const ogdf::GraphAttributes& _graphAttributes)
	{
		std::vector<std::pair<Vec2, Vec2>> lines{};
		{
			size_t count{ 0 };
			for (const ogdf::edge edge : _graphAttributes.constGraph().edges)
			{
				count += _graphAttributes.bends(edge).size() + 1;
			}
			lines.reserve(count);
		}
		for (const ogdf::edge edge : _graphAttributes.constGraph().edges)
		{
			const ogdf::DPolyline& polyline{ _graphAttributes.bends(edge) };
			ogdf::DPoint lastPoint{ _graphAttributes.x(edge->source()), _graphAttributes.y(edge->source()) };
			for (const ogdf::DPoint point : polyline)
			{
				lines.push_back({ Vec2{lastPoint.m_x, -lastPoint.m_y}, Vec2{point.m_x, -point.m_y} });
				lastPoint = point;
			}
			const ogdf::DPoint endPoint{ _graphAttributes.x(edge->target()), _graphAttributes.y(edge->target()) };
			lines.push_back({ Vec2{lastPoint.m_x, -lastPoint.m_y}, Vec2{endPoint.m_x, -endPoint.m_y} });
		}
		return lines;
	}

	Layout createLayout(const Dag::Node& _dag)
	{
		ogdf::Logger::globalLogLevel(ogdf::Logger::Level::Alarm);
		ogdf::Graph graph{};
		const std::vector<std::pair<const Dag::Node*, ogdf::node>> dagToGraphNodePairs{ populateGraph(_dag, graph) };
		const ogdf::GraphAttributes graphAttributes{ layoutGraph(graph) };
		std::vector<Layout::Node> nodes{ createLayoutNodes(dagToGraphNodePairs, graphAttributes) };
		std::vector<std::pair<Vec2, Vec2>> lines{ createLayoutLines(graphAttributes) };
		return Layout{ std::move(nodes), std::move(lines),  c_nodeRadius, c_lineThickness };
	}

}