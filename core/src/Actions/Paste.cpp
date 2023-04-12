#include <HMP/Actions/Paste.hpp>

#include <cpputils/unreachable.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/ExtrudeUtils.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Dag/Refine.hpp>
#include <array>
#include <algorithm>
#include <utility>
#include <cassert>
#include <cpputils/range/of.hpp>
#include <cpputils/range/zip.hpp>
#include <cinolib/geometry/vec_mat_utils.h>
#include <unordered_map>

namespace HMP::Actions
{

	std::array<Vec, 3> basis(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude)
	{
		std::vector<Vec> newVerts;
		const HexVerts verts{ Meshing::Utils::verts(_mesher.mesh(), ExtrudeUtils::apply(_mesher, _extrude, newVerts), newVerts) };
		std::array<I, 3> indices{ 1,4,3 };
		if (_extrude.clockwise)
		{
			std::reverse(indices.begin(), indices.end());
		}
		return cpputils::range::of(indices).map([&](I _i) { return (verts[_i] - verts[0]).normalized(); }).toArray();
	}

	Vec origin(const Meshing::Mesher::Mesh& _mesh, const Dag::Extrude& _extrude)
	{
		Id vid{ _extrude.parents.first().vids[_extrude.firstVi] };
		return _mesh.vert(vid);
	}

	Mat4 transformMat(const Meshing::Mesher& _mesher, const Dag::Extrude& _source, const Dag::Extrude& _target)
	{
		const std::array<Vec, 3> oldBasis{ basis(_mesher, _source) };
		const std::array<Vec, 3> newBasis{ basis(_mesher, _target) };
		const Mat3 oldToNorm{ Mat3{oldBasis[0], oldBasis[1], oldBasis[2]}.transpose() };
		const Mat3 newToNorm{ Mat3{newBasis[0], newBasis[1], newBasis[2]}.transpose() };
		const Mat3 oldToNew{ newToNorm.inverse() * oldToNorm };
		return Mat4::TRANS(origin(_mesher.mesh(), _target)) * Mat4::HOMOGENEOUS(oldToNew) * Mat4::TRANS(-origin(_mesher.mesh(), _source));
	}

	std::unordered_map<Id, Id> weld(const Dag::Extrude& _extrude, const I _parentIndex, const std::array<I, 4>& _is)
	{
		const Dag::Element& firstParent{ _extrude.parents.first() };
		const Id firstVid{ firstParent.vids[_extrude.firstVi] };
		const Dag::Element& parent{ _extrude.parents[_parentIndex] };
		const QuadVertIds parentVids{ Meshing::Utils::align(Meshing::Utils::fiVids(parent.vids, _extrude.fis[_parentIndex]), firstVid) };
		std::unordered_map<Id, Id> vidsMap{};
		vidsMap.reserve(4);
		for (const auto& [newVid, targetI] : cpputils::range::zip(parentVids, _is))
		{
			const Id oldVid{ _extrude.children.single().vids[targetI] };
			vidsMap.emplace(oldVid, newVid);
		}
		return vidsMap;
	}

	Dag::Extrude& cloneTree(const Meshing::Mesher& _mesher, const Dag::Extrude& _source)
	{
		std::unordered_map<const Dag::Node*, Dag::Node*> src2clone;
		const std::vector<const Dag::Node*> srcNodes{ Dag::Utils::descendants(_source) };
		src2clone.reserve(srcNodes.size());
		for (const Dag::Node* src : srcNodes)
		{
			src2clone.insert({ src, &Dag::Utils::clone(*src) });
		}
		for (const auto [src, clone] : src2clone)
		{
			if (src == &_source)
			{
				continue;
			}
			if (src->isOperation() && src->operation().primitive == Dag::Operation::EPrimitive::Extrude)
			{
				Dag::Extrude& cloneOp{ clone->as<Dag::Extrude>() };
				const Dag::Extrude& srcOp{ src->as<Dag::Extrude>() };
				cloneOp.fis.clear();
				for (const auto& [srcParent, srcParentFi] : cpputils::range::zip(srcOp.parents, srcOp.fis))
				{
					const auto it{ src2clone.find(&srcParent) };
					if (it != src2clone.end())
					{
						cloneOp.parents.attach(it->second->element());
						cloneOp.fis.addLast(srcParentFi);
						if (cloneOp.parents.isSingle())
						{
							const Id vid{ srcOp.parents.first().vids[srcOp.firstVi] };
							cloneOp.firstVi = Meshing::Utils::vi(cloneOp.parents.first().vids, vid);
						}
					}
				}
				cloneOp.source = Dag::Extrude::sourceByParentCount(cloneOp.parents.size());
			}
			else
			{
				for (const Dag::Node& srcParent : src->parents)
				{
					const auto it{ src2clone.find(&srcParent) };
					clone->parents.attach(*src2clone.at(&srcParent));
				}
			}
		}
		return src2clone.at(&_source)->as<Dag::Extrude>();
	}

	void fixAdjacencies_TEMP_NAIVE(const Meshing::Mesher& _mesher, Dag::Extrude& _root)
	{
		for (Dag::Node* node : Dag::Utils::descendants(_root))
		{
			if (node->isOperation() && node->operation().primitive == Dag::Operation::EPrimitive::Extrude && node != &_root)
			{
				Dag::Extrude& extrude{ node->as<Dag::Extrude>() };
				for (const auto& [parent, parentFi] : cpputils::range::zip(extrude.parents, extrude.fis))
				{
					const Id fid{ static_cast<Id>(_mesher.mesh().poly_shared_face(parent.pid, extrude.children.single().pid)) };
					parentFi = Meshing::Utils::fi(parent.vids, Meshing::Utils::fidVids(_mesher.mesh(), fid));
				}
				extrude.firstVi = Meshing::Utils::hexFiVis[extrude.fis[0]][0];
				extrude.clockwise = false;
			}
		}
	}

	void Paste::apply()
	{
		m_oldState = mesher().state();
		if (!m_prepared)
		{
			m_prepared = true;
			m_operation = cloneTree(mesher(), m_sourceOperation);
			m_operation->fis = m_fis;
			m_operation->clockwise = m_clockwise;
			m_operation->firstVi = m_firstVi;
			m_operation->source = Dag::Extrude::sourceByParentCount(m_elements.size());
			for (Dag::Element* parent : m_elements)
			{
				m_operation->parents.attach(*parent);
			}
			std::array<std::array<I, 4>, 3> indices{
				std::array<I, 4>{0,1,2,3},
				std::array<I, 4>{0,4,5,1},
				std::array<I, 4>{0,3,7,4}
			};
			if (m_operation->clockwise)
			{
				std::swap(indices[1], indices[2]);
			}
			const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
			const Id numVerts{ mesh.num_verts() };
			const Mat4 transform{ transformMat(mesher(), m_sourceOperation, *m_operation) };
			if (m_sourceOperation.clockwise != m_operation->clockwise)
			{
				for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
				{
					if (node->isElement())
					{
						Dag::Element& element{ node->element() };
						element.vids = Meshing::Utils::reverse(element.vids);
					}
				}
			}
			std::unordered_map<Id, Id> vidMap{};
			for (I i{}; i < m_elements.size(); i++)
			{
				vidMap.merge(weld(*m_operation, i, indices[i]));
			}
			for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
			{
				if (node->isElement())
				{
					node->element().pid = noId;
					for (Id& vid : node->element().vids)
					{
						const auto it{ vidMap.find(vid) };
						if (it != vidMap.end())
						{
							vid = it->second;
						}
						else
						{
							const Id oldVid{ vid };
							vid = numVerts + toId(m_newVerts.size());
							vidMap.emplace_hint(it, oldVid, vid);
							m_newVerts.push_back(transform * mesh.vert(oldVid));
						}
					}
				}
			}
			for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
			{
				if (node->isOperation())
				{
					Dag::Operation& operation{ node->operation() };
					if (operation.primitive == Dag::Operation::EPrimitive::Refine)
					{
						Dag::Refine& refine{ operation.as<Dag::Refine>() };
						for (Id& vid : refine.surfVids)
						{
							vid = vidMap.at(vid);
						}
					}
				}
			}
			Meshing::Utils::addTree(mesher(), *m_operation, m_newVerts);
			fixAdjacencies_TEMP_NAIVE(mesher(), *m_operation);
			mesher().updateMesh();
		}
		else
		{
			for (Dag::Element* parent : m_elements)
			{
				m_operation->parents.attach(*parent);
			}
			Meshing::Utils::addTree(mesher(), *m_operation, m_newVerts);
			mesher().updateMesh();
		}
	}

	void Paste::unapply()
	{
		m_operation->parents.detachAll(false);
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	Paste::Paste(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise, const Dag::Extrude& _source)
		: m_elements{ _elements }, m_sourceOperation{ _source }, m_fis{ _fis }, m_firstVi{ _firstVi }, m_clockwise{ _clockwise }, m_operation{}
	{}

	Paste::Elements Paste::elements() const
	{
		return cpputils::range::of(m_elements).dereference().immutable();
	}

	const Dag::Extrude& Paste::operation() const
	{
		return *m_operation;
	}

}