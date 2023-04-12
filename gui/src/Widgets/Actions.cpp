#include <HMP/Gui/Widgets/Actions.hpp>

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Actions/Root.hpp>
#include <HMP/Actions/Pad.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/RefineSome.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Actions/Transform.hpp>
#include <HMP/Actions/Smooth.hpp>
#include <HMP/Actions/SubdivideAll.hpp>
#include <HMP/Actions/FitCircle.hpp>
#include <HMP/Actions/SplitPlane.hpp>
#include <HMP/Actions/DeleteSome.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <algorithm>
#include <vector>
#include <iostream>

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/Widget.hpp>
#endif

namespace HMP::Gui::Widgets
{

	bool Actions::keyPressed(const cinolib::KeyBinding& _key)
	{
		// extrude
		if (_key == c_kbExtrudeFace)
		{
			onExtrude(Dag::Extrude::ESource::Face);
		}
		else if (_key == c_kbExtrudeEdge)
		{
			onExtrude(Dag::Extrude::ESource::Edge);
		}
		else if (_key == c_kbExtrudeVertex)
		{
			onExtrude(Dag::Extrude::ESource::Vertex);
		}
		// extrude selected
		else if (_key == c_kbExtrudeSelectedFace)
		{
			onExtrudeSelectedFace();
		}
		// copy
		else if (_key == c_kbCopy)
		{
			onCopy();
		}
		// paste
		else if (_key == c_kbPasteFace)
		{
			onPaste(Dag::Extrude::ESource::Face);
		}
		else if (_key == c_kbPasteEdge)
		{
			onPaste(Dag::Extrude::ESource::Edge);
		}
		else if (_key == c_kbPasteVertex)
		{
			onPaste(Dag::Extrude::ESource::Vertex);
		}
		// refine hexahedron
		else if (_key == c_kbRefine)
		{
			onRefineElement(false);
		}
		// refine hexahedron twice
		else if (_key == c_kbDoubleRefine)
		{
			onRefineElement(true);
		}
		// refine face
		else if (_key == c_kbFaceRefine)
		{
			onRefineFace();
		}
		// delete hexahedron
		else if (_key == c_kbDelete)
		{
			onDelete();
		}
		// make conformant
		else if (_key == c_kbMakeConforming)
		{
			onMakeConformant();
		}
		// clear
		else if (_key == c_kbClear)
		{
			onClear();
		}
		// subdivide all
		else if (_key == c_kbSubdivideAll)
		{
			onSubdivideAll();
		}
		// subdivide selected elements
		else if (_key == c_kbRefineSelectedElements)
		{
			onRefineSelectedElements();
		}
		// fit circle
		else if (_key == c_kbFitCircle)
		{
			onFitCircle();
		}
		// delete some
		else if (_key == c_kbDeleteSelectedElements)
		{
			onDeleteSelectedElements();
		}
		// split plane
		else if (_key == c_kbSplitPlane)
		{
			onSplitPlane();
		}
		else
		{
			return false;
		}
		return true;
	}

	void Actions::onSplitPlane()
	{
		if (app().cursor.element)
		{
			app().applyAction(*new HMP::Actions::SplitPlane{ app().cursor.eid });
		}
	}

	void Actions::onFitCircle()
	{
		if (app().vertEditWidget.vids().size() > 2)
		{
			std::vector<Id> vids{ app().vertEditWidget.vids().toVector() };
			const auto it{ std::find(vids.begin(), vids.end(), app().cursor.vid) };
			if (it != vids.end())
			{
				std::swap(vids[0], *it);
			}
			app().applyAction(*new HMP::Actions::FitCircle{ vids });
		}
	}

	void Actions::onRefineSelectedElements()
	{
		app().applyAction(*new HMP::Actions::RefineSome{ selectedElements() });
	}

	void Actions::onDeleteSelectedElements()
	{
		app().applyAction(*new HMP::Actions::DeleteSome{ selectedElements() });
	}

	std::vector<Dag::Element*> Actions::selectedElements()
	{
		std::unordered_set<Id> pids{};
		for (Id pid{}; pid < app().mesh.num_polys(); pid++)
		{
			if (!app().mesher.shown(pid))
			{
				goto skip;
			}
			for (const Id vid : app().mesh.adj_p2v(pid))
			{
				if (!app().vertEditWidget.has(vid))
				{
					goto skip;
				}
			}
			pids.insert(pid);
		skip:;
		}
		return cpputils::range::of(pids).map([&](const Id _pid)
			{
				return &app().mesher.element(_pid);
			}).toVector();
	}

	void Actions::printUsage() const
	{
		cinolib::print_binding(c_kbExtrudeFace.name(), "extrude face");
		cinolib::print_binding(c_kbExtrudeEdge.name(), "extrude edge");
		cinolib::print_binding(c_kbExtrudeVertex.name(), "extrude vertex");
		cinolib::print_binding(c_kbExtrudeSelectedFace.name(), "extrude selected face");
		cinolib::print_binding(c_kbRefine.name(), "refine");
		cinolib::print_binding(c_kbDoubleRefine.name(), "refine twice");
		cinolib::print_binding(c_kbFaceRefine.name(), "refine face");
		cinolib::print_binding(c_kbDelete.name(), "delete");
		cinolib::print_binding(c_kbCopy.name(), "copy");
		cinolib::print_binding(c_kbPasteFace.name(), "paste face");
		cinolib::print_binding(c_kbPasteEdge.name(), "paste edge");
		cinolib::print_binding(c_kbPasteVertex.name(), "paste vertex");
		cinolib::print_binding(c_kbClear.name(), "clear");
		cinolib::print_binding(c_kbSubdivideAll.name(), "subdivide all");
		cinolib::print_binding(c_kbRefineSelectedElements.name(), "refine selected elements");
		cinolib::print_binding(c_kbMakeConforming.name(), "make conforming");
		cinolib::print_binding(c_kbDeleteSelectedElements.name(), "delete selected elements");
		cinolib::print_binding(c_kbFitCircle.name(), "fit circle in selected vertices");
		cinolib::print_binding(c_kbSplitPlane.name(), "split plane");
	}

	bool Actions::hoveredExtrudeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise)
	{
		cpputils::collections::FixedVector<Id, 3> pids, fids;
		if (app().cursor.element)
		{
			pids.addLast(app().cursor.pid);
			fids.addLast(app().cursor.fid);
			const Id commVid{ app().cursor.vid };
			const Id firstEid{ app().cursor.eid };
			if (_source != Dag::Extrude::ESource::Face)
			{
				const cinolib::Plane firstPlane{
					app().mesh.face_centroid(fids[0]),
						app().mesh.poly_face_normal(pids[0], fids[0])
				};
				for (const Id adjFid : app().mesh.adj_e2f(firstEid))
				{
					Id adjPid;
					if (app().mesh.face_is_visible(adjFid, adjPid)
						&& adjPid != pids[0]
						&& firstPlane.point_plane_dist_signed(app().mesh.face_centroid(adjFid)) > 0)
					{
						if (fids.size() == 2)
						{
							const auto edgeVec{ [&](const Id _pid, const Id _fid)
							 {
							return app().mesh.edge_vec(static_cast<Id>(app().mesh.edge_id(commVid, app().mesh.poly_vert_opposite_to(_pid, _fid, commVid))), true);
							} };
							const Vec firstEdge{ edgeVec(pids[0], fids[0]) };
							const Vec currSecondEdge{ edgeVec(pids[1], fids[1]) };
							const Vec candSecondEdge{ edgeVec(adjPid, adjFid) };
							if (firstEdge.dot(candSecondEdge) > firstEdge.dot(currSecondEdge))
							{
								pids[1] = adjPid;
								fids[1] = adjFid;
							}
						}
						else
						{
							pids.addLast(adjPid);
							fids.addLast(adjFid);
						}
					}
				}
				if (fids.size() != 2)
				{
					return false;
				}
				if (_source == Dag::Extrude::ESource::Vertex)
				{
					for (const Id adjFid : app().mesh.adj_f2f(fids[0]))
					{
						Id adjPid;
						if (app().mesh.face_is_visible(adjFid, adjPid)
							&& adjPid != pids[0] && adjPid != pids[1]
							&& adjFid != fids[0] && adjFid != fids[1]
							&& app().mesh.face_contains_vert(adjFid, commVid)
							&& app().mesh.faces_are_adjacent(adjFid, fids[1]))
						{
							if (fids.size() == 3)
							{
								return false;
							}
							pids.addLast(adjPid);
							fids.addLast(adjFid);
						}
					}
					if (fids.size() != 3)
					{
						return false;
					}
				}
			}
			_clockwise = Meshing::Utils::isEdgeCW(app().mesh, pids[0], fids[0], commVid, firstEid);
			_elements = cpputils::range::of(pids).map([&](Id _pid)
				{
					return &app().mesher.element(_pid);
				}).toFixedVector<3>();
				_fis = cpputils::range::zip(fids, _elements).map([&](const auto& _fidAndElement)
					{
						const auto& [fid, element] { _fidAndElement };
						const QuadVertIds vids{ Meshing::Utils::fidVids(app().mesh, fid) };
						return Meshing::Utils::fi(element->vids, vids);
					}).toFixedVector<3>();
					_firstVi = app().cursor.vi;
					return true;
		}
		return false;
	}

	void Actions::onExtrude(Dag::Extrude::ESource _source)
	{
		cpputils::collections::FixedVector<Dag::Element*, 3> elements;
		cpputils::collections::FixedVector<I, 3> fis;
		I firstVi;
		bool clockwise;
		if (hoveredExtrudeElements(_source, elements, fis, firstVi, clockwise))
		{
			app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
		}
	}

	void Actions::onExtrudeSelectedFace()
	{
		const std::vector<Id> vids{ app().vertEditWidget.vids().toVector() };
		if (vids.size() != 4)
		{
			return;
		}
		const Id fid{ static_cast<Id>(app().mesh.face_id(vids)) };
		if (fid == noId)
		{
			return;
		}
		Id pid;
		if (!app().mesh.face_is_visible(fid, pid))
		{
			return;
		}
		Dag::Element& element{ app().mesher.element(pid) };
		const I fi{ Meshing::Utils::fi(element.vids, Meshing::Utils::fidVids(app().mesh, fid)) };
		const I vi{ Meshing::Utils::vi(element.vids, vids[0]) };
		HMP::Actions::Extrude& action{ *new HMP::Actions::Extrude{ {&element}, {fi}, vi, false } };
		app().applyAction(action);
		const Id newPid{ action.operation().children.single().pid };
		const Id newFid{ app().mesh.poly_face_opposite_to(newPid, fid) };
		app().vertEditWidget.clear();
		app().vertEditWidget.add(app().mesh.face_verts_id(newFid));
	}

	void Actions::onCopy()
	{
		if (app().cursor.element && app().cursor.element->parents.isSingle() && app().cursor.element->parents.first().primitive == Dag::Operation::EPrimitive::Extrude)
		{
			app().copiedElement = app().cursor.element;
		}
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		else if (app().dagViewerWidget.hovered() && app().dagViewerWidget.hovered()->isElement() && app().dagViewerWidget.hovered()->element().parents.first().primitive == Dag::Operation::EPrimitive::Extrude)
		{
			app().copiedElement = const_cast<Dag::Element*>(&app().dagViewerWidget.hovered()->element());
		}
#endif
		else
		{
			app().copiedElement = nullptr;
		}
	}

	void Actions::onPaste(Dag::Extrude::ESource _source)
	{
		if (app().copiedElement)
		{
			cpputils::collections::FixedVector<Dag::Element*, 3> elements;
			cpputils::collections::FixedVector<I, 3> fis;
			I firstVi;
			bool clockwise;
			if (hoveredExtrudeElements(_source, elements, fis, firstVi, clockwise))
			{
				app().applyAction(*new HMP::Actions::Paste{ elements, fis, firstVi, clockwise, app().copiedElement->parents.single().as<Dag::Extrude>() });
			}
		}
	}

	void Actions::onRefineElement(bool _twice)
	{
		if (app().cursor.element)
		{
			app().applyAction(*new HMP::Actions::Refine{ *app().cursor.element, app().cursor.fi, app().cursor.vi, Refinement::EScheme::Subdivide3x3, _twice ? 2u : 1u });
		}
	}

	void Actions::onSubdivideAll()
	{
		app().applyAction(*new HMP::Actions::SubdivideAll{});
	}

	void Actions::onDelete()
	{
		if (app().mesh.num_polys() <= 1)
		{
			std::cout << "cannot delete the only element" << std::endl;
			return;
		}
		if (app().cursor.element)
		{
			app().applyAction(*new HMP::Actions::Delete{ *app().cursor.element });
		}
	}

	void Actions::onRefineFace()
	{
		if (app().cursor.element)
		{
			app().applyAction(*new HMP::Actions::Refine{ *app().cursor.element, app().cursor.fi, app().cursor.vi, Refinement::EScheme::Inset });
		}
	}

	void Actions::onMakeConformant()
	{
		app().applyAction(*new HMP::Actions::MakeConforming());
	}

	void Actions::onClear()
	{
		Dag::Element& root{ *new Dag::Element{} };
		root.vids = { 0,1,2,3,4,5,6,7 };
		app().applyAction(*new HMP::Actions::Root{ root, {
			Vec{-1,-1,-1}, Vec{+1,-1,-1}, Vec{+1,+1,-1}, Vec{-1,+1,-1},
			Vec{-1,-1,+1}, Vec{+1,-1,+1}, Vec{+1,+1,+1}, Vec{-1,+1,+1},
		} });
	}

	void Actions::clear()
	{
		onClear();
	}

}