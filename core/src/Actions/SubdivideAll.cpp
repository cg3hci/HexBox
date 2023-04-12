#include <HMP/Actions/SubdivideAll.hpp>

#include <HMP/Refinement/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>

namespace HMP::Actions
{

	std::pair<const Dag::Element&, const Dag::Refine&> SubdivideAll::dereferenceOperation(const std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>& _pair)
	{
		const auto& [el, op] {_pair};
		return { *el, *op };
	}

	void SubdivideAll::apply()
	{
		if (!m_prepared)
		{
			const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
			m_oldState = mesher().state();
			m_prepared = true;
			for (Id pid{}; pid < mesh.num_polys(); pid++)
			{
				if (mesher().shown(pid))
				{
					Dag::Refine& refine{ Refinement::Utils::prepare(0, Meshing::Utils::hexFiVis[0][0], Refinement::EScheme::Subdivide2x2) };
					m_operations.emplace_back(&mesher().element(pid), Dag::NodeHandle<Dag::Refine>{ refine });
				}
			}
		}
		for (const auto& [parent, op] : m_operations)
		{
			op->parents.attach(*parent);
			Refinement::Utils::apply(mesher(), *op);
		}
		mesher().updateMesh();
	}

	void SubdivideAll::unapply()
	{
		for (const auto& [parent, op] : m_operations)
		{
			mesher().show(*parent, true);
			op->parents.detachAll(false);
		}
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	SubdivideAll::SubdivideAll()
		: m_prepared{ false }
	{}

	SubdivideAll::Operations SubdivideAll::operations() const
	{
		return cpputils::range::ofc(m_operations).map(&dereferenceOperation);
	}

}