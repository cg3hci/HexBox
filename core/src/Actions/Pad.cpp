#include <HMP/Actions/Pad.hpp>

#include <HMP/Actions/ExtrudeUtils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/index.hpp>
#include <cpputils/range/enumerate.hpp>
#include <HMP/Projection/smooth.hpp>
#include <cassert>
#include <unordered_map>
#include <algorithm>

namespace HMP::Actions
{

	std::pair<const Dag::Element&, const Dag::Extrude&> Pad::dereferenceOperation(const std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Extrude>>& _pair)
	{
		const auto& [el, op] {_pair};
		return { *el, *op };
	}

	void Pad::swapVerts()
	{
		for (I vi{}; vi < m_otherVerts.size(); vi++)
		{
			const Vec oldVert{ mesher().mesh().vert(toId(vi)) };
			mesher().moveVert(toId(vi), m_otherVerts[vi]);
			m_otherVerts[vi] = oldVert;
		}
	}

	void Pad::apply()
	{
		if (!m_prepared)
		{
			const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
			m_oldState = mesher().state();
			m_prepared = true;
			std::unordered_map<Id, Id> newVertsMap;
			{
				Meshing::Mesher::Mesh tempMesh{ mesh };
				std::vector<Id> surfVids;
				for (Id vid{}; vid < tempMesh.num_verts(); ++vid)
				{
					bool visible{ false };
					Vec displacement{};
					for (const Id adjFid : mesh.adj_v2f(vid))
					{
						Id adjPid;
						if (mesh.face_is_visible(adjFid, adjPid))
						{
							if (!visible)
							{
								visible = true;
								surfVids.push_back(vid);
								newVertsMap.emplace(vid, mesh.num_verts() + toId(m_newVerts.size()));
								m_newVerts.push_back(mesh.vert(vid));
							}
							displacement += mesh.poly_face_normal(adjPid, adjFid);
						}
					}
					if (visible)
					{
						const I adjPolyCount{ cpputils::range::of(tempMesh.adj_v2p(vid)).filter([&](Id _adjPid) {
							return mesher().shown(_adjPid);
						}).size() };
						const Real factor{ (1.0 - m_cornerShrinkFactor) + static_cast<Real>(adjPolyCount) * m_cornerShrinkFactor };
						if (displacement.norm() != 0.0)
						{
							displacement.normalize();
						}
						tempMesh.vert(vid) += displacement * -m_length / factor;
					}
				}
				for (I i{}; i < m_smoothIterations; ++i)
				{
					tempMesh.vector_verts() = Projection::smoothInternal(tempMesh, surfVids, m_smoothSurfVertWeight);
				}
				m_otherVerts = tempMesh.vector_verts();
			}
			for (Id fid{}; fid < mesh.num_faces(); fid++)
			{
				Id pid;
				if (mesh.face_is_visible(fid, pid))
				{
					Dag::Element& element{ mesher().element(pid) };
					const I fi{ Meshing::Utils::fi(element.vids, Meshing::Utils::fidVids(mesh, fid)) };
					const QuadVertIds vids{ Meshing::Utils::fiVids(element.vids, fi) };
					Dag::Extrude& extrude{ ExtrudeUtils::prepare({ fi }, vids[0], false) };
					Dag::Element& child{ extrude.children.single() };
					std::copy(vids.begin(), vids.end(), child.vids.begin());
					for (I vi{}; vi < 4; vi++)
					{
						child.vids[vi + 4] = newVertsMap.at(vids[vi]);
					}
					m_operations.emplace_back(&element, Dag::NodeHandle<Dag::Extrude>{ extrude });
				}
			}
		}
		swapVerts();
		for (const auto& [parent, op] : m_operations)
		{
			op->parents.attach(*parent);
		}
		mesher().add(cpputils::range::of(m_operations).map([](const auto& _elAndOp) {
			return &std::get<1>(_elAndOp)->children.single();
		}).toVector(), m_newVerts);
		mesher().updateMesh();
	}

	void Pad::unapply()
	{
		for (const auto& [parent, op] : m_operations)
		{
			op->parents.detachAll(false);
		}
		mesher().restore(m_oldState);
		swapVerts();
		mesher().updateMesh();
	}

	Pad::Pad(Real _length, I _smoothIterations, Real _smoothSurfVertWeight, Real _cornerShrinkFactor)
		: m_length{ _length }, m_smoothIterations{ _smoothIterations }, m_smoothSurfVertWeight{ _smoothSurfVertWeight }, m_cornerShrinkFactor{ _cornerShrinkFactor }, m_prepared{ false }
	{}

	const Real Pad::length() const
	{
		return m_length;
	}

	const I Pad::smoothIterations() const
	{
		return m_smoothIterations;
	}

	const Real Pad::smoothSurfVertWeight() const
	{
		return m_smoothSurfVertWeight;
	}

	const Real Pad::cornerShrinkFactor() const
	{
		return m_cornerShrinkFactor;
	}

	Pad::Operations Pad::operations() const
	{
		return cpputils::range::ofc(m_operations).map(&dereferenceOperation);
	}

}