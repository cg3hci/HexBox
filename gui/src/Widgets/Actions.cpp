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

    std::optional<unsigned int> Actions::getEdgeInCommon(std::vector<Dag::Element*> clickedElements)
    {

        if(clickedElements.size() != 2)
        {
            return std::nullopt;
        }
        Dag::Element* firstElement;
        Dag::Element* secondElement;
        firstElement = clickedElements[0];
        secondElement = clickedElements[1];
        std::vector<unsigned int> firstElementEdges = app().mesh.adj_p2e(firstElement->pid);
        std::vector<unsigned int> secondElementEdges = app().mesh.adj_p2e(secondElement->pid);

        std::sort(firstElementEdges.begin(), firstElementEdges.end());
        std::sort(secondElementEdges.begin(), secondElementEdges.end());

        std::vector<unsigned int> intersection;
        intersection.resize(std::min(firstElementEdges.size(), secondElementEdges.size()));

        auto it = std::set_intersection(firstElementEdges.begin(), firstElementEdges.end(),
                                        secondElementEdges.begin(), secondElementEdges.end(),
                                        intersection.begin());

        intersection.resize(it - intersection.begin());
        if(intersection.size() != 1)
            return std::nullopt;
        return intersection[0];
    }

    std::optional<unsigned int> Actions::getVertexInCommon(std::vector<Dag::Element*> clickedElements)
    {
        if(clickedElements.size() != 3)
        {
            return std::nullopt;
        }

        // Correctly assign the elements
        Dag::Element* firstElement = clickedElements[0];
        Dag::Element* secondElement = clickedElements[1];
        Dag::Element* thirdElement = clickedElements[2];

        // Retrieve vertices for each element
        std::vector<unsigned int> firstElementVertices = app().mesh.adj_p2v(firstElement->pid);
        std::vector<unsigned int> secondElementVertices = app().mesh.adj_p2v(secondElement->pid);
        std::vector<unsigned int> thirdElementVertices = app().mesh.adj_p2v(thirdElement->pid);

        // Sort the vertices to prepare for set intersection
        std::sort(firstElementVertices.begin(), firstElementVertices.end());
        std::sort(secondElementVertices.begin(), secondElementVertices.end());
        std::sort(thirdElementVertices.begin(), thirdElementVertices.end());

        // Find the intersection of first two elements
        std::vector<unsigned int> tempIntersection;
        tempIntersection.resize(std::min(firstElementVertices.size(), secondElementVertices.size()));

        auto it = std::set_intersection(firstElementVertices.begin(), firstElementVertices.end(),
                                        secondElementVertices.begin(), secondElementVertices.end(),
                                        tempIntersection.begin());

        tempIntersection.resize(it - tempIntersection.begin());

        // Find the intersection with the third element
        std::vector<unsigned int> finalIntersection;
        finalIntersection.resize(std::min(tempIntersection.size(), thirdElementVertices.size()));

        it = std::set_intersection(tempIntersection.begin(), tempIntersection.end(),
                                   thirdElementVertices.begin(), thirdElementVertices.end(),
                                   finalIntersection.begin());

        finalIntersection.resize(it - finalIntersection.begin());

        // Ensure that there is exactly one common vertex
        if(finalIntersection.size() != 1)
            return std::nullopt;

        return finalIntersection[0];
    }




    bool Actions::clickedExtrudeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise, Dag::Element* element, App::Cursor cursor)
    {
        cpputils::collections::FixedVector<Id, 3> pids, fids;
        if (element)
        {
            pids.addLast(cursor.pid);
            fids.addLast(cursor.fid);
            const Id commVid{ cursor.vid };
            const Id firstEid{ cursor.eid };
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
            _firstVi = cursor.vi;
            return true;
        }
        return false;

    }

    bool Actions::clickedExtrudeEdgeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise, Dag::Element* element, App::Cursor cursor)
    {
        cpputils::collections::FixedVector<Id, 3> pids, fids;
        if (element)
        {
            pids.addLast(cursor.pid);
            fids.addLast(cursor.fid);
            const Id commVid{ cursor.vid };
            const Id firstEid{ cursor.eid };
            const cinolib::Plane firstPlane{
                    app().mesh.face_centroid(fids[0]),
                    app().mesh.poly_face_normal(pids[0], fids[0])
            };
            for (const Id adjFid : app().mesh.adj_e2f(firstEid)) {
                Id adjPid;
                if (app().mesh.face_is_visible(adjFid, adjPid)
                    && adjPid != pids[0]
                    && firstPlane.point_plane_dist_signed(app().mesh.face_centroid(adjFid)) > 0) {
                    if (fids.size() == 2) {
                        const auto edgeVec{[&](const Id _pid, const Id _fid) {
                            return app().mesh.edge_vec(static_cast<Id>(app().mesh.edge_id(commVid,
                                                                                          app().mesh.poly_vert_opposite_to(
                                                                                                  _pid, _fid,
                                                                                                  commVid))), true);
                        }};
                        const Vec firstEdge{edgeVec(pids[0], fids[0])};
                        const Vec currSecondEdge{edgeVec(pids[1], fids[1])};
                        const Vec candSecondEdge{edgeVec(adjPid, adjFid)};
                        if (firstEdge.dot(candSecondEdge) > firstEdge.dot(currSecondEdge)) {
                            pids[1] = adjPid;
                            fids[1] = adjFid;
                        }
                    } else {
                        pids.addLast(adjPid);
                        fids.addLast(adjFid);
                    }
                }
            }
            if (fids.size() != 2)
            {
                return false;
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
            _firstVi = cursor.vi;
            return true;
        }
        return false;
    }

    bool Actions::clickedExtrudeVertexElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise, Dag::Element* element, App::Cursor cursor)
    {
        cpputils::collections::FixedVector<Id, 3> pids, fids;
        if (element)
        {
            pids.addLast(cursor.pid);
            fids.addLast(cursor.fid);
            const Id commVid{ cursor.vid };
            const Id firstEid{ cursor.eid };
            const cinolib::Plane firstPlane{
                    app().mesh.face_centroid(fids[0]),
                    app().mesh.poly_face_normal(pids[0], fids[0])
            };
            for (const Id adjFid : app().mesh.adj_e2f(firstEid)) {
                Id adjPid;
                if (app().mesh.face_is_visible(adjFid, adjPid)
                    && adjPid != pids[0]
                    && firstPlane.point_plane_dist_signed(app().mesh.face_centroid(adjFid)) > 0) {
                    if (fids.size() == 2) {
                        const auto edgeVec{[&](const Id _pid, const Id _fid) {
                            return app().mesh.edge_vec(static_cast<Id>(app().mesh.edge_id(commVid,
                                                                                          app().mesh.poly_vert_opposite_to(
                                                                                                  _pid, _fid,
                                                                                                  commVid))), true);
                        }};
                        const Vec firstEdge{edgeVec(pids[0], fids[0])};
                        const Vec currSecondEdge{edgeVec(pids[1], fids[1])};
                        const Vec candSecondEdge{edgeVec(adjPid, adjFid)};
                        if (firstEdge.dot(candSecondEdge) > firstEdge.dot(currSecondEdge)) {
                            pids[1] = adjPid;
                            fids[1] = adjFid;
                        }
                    } else {
                        pids.addLast(adjPid);
                        fids.addLast(adjFid);
                    }
                }
            }
            if (fids.size() != 2)
            {
                return false;
            }
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
            _firstVi = cursor.vi;
            return true;
        }
        return false;
    }

    std::vector<std::vector<unsigned int>> Actions::getClickedElementsFids(const std::vector<App::Cursor>& cursorsOfClickedElements)
    {
        std::vector<std::vector<unsigned int>> clickedElementsFids;

        for (const App::Cursor& cursor: cursorsOfClickedElements)
        {
            const std::vector<unsigned int>& constVector = app().mesh.adj_f2v(cursor.fid);
            clickedElementsFids.emplace_back(constVector.begin(), constVector.end());
        }

        return clickedElementsFids;
    }


    bool Actions::checkIfClickedExtrudeIsPossible()
    {
        std::vector<Dag::Element*> clickedElements = app().clickedElements;
        std::vector<App::Cursor> cursorsOfClickedElements = app().cursorsOfClickedElements;
        std::vector<std::vector<unsigned int>> clickedElementsFids = getClickedElementsFids(cursorsOfClickedElements);
        for (int i = 0; i < clickedElementsFids.size(); i++) {
            for (int j = i + 1; j < clickedElementsFids.size(); j++) {
                if(cursorsOfClickedElements[i].pid == cursorsOfClickedElements[j].pid)
                {
                    continue;
                }

                std::vector<unsigned int> adjacentElements = app().mesh.adj_p2p(cursorsOfClickedElements[i].pid);
                if(std::find(adjacentElements.begin(), adjacentElements.end(), cursorsOfClickedElements[j].pid) == adjacentElements.end())
                {
                    std::vector<unsigned int> fidsOfI = clickedElementsFids[i];
                    std::vector<unsigned int> fidsOfJ = clickedElementsFids[j];
                    std::sort(fidsOfI.begin(), fidsOfI.end());
                    std::sort(fidsOfJ.begin(), fidsOfJ.end());
                    std::vector<unsigned int> intersection;
                    intersection.resize(std::min(fidsOfI.size(), fidsOfJ.size()));
                    auto it = std::set_intersection(fidsOfI.begin(), fidsOfI.end(), fidsOfJ.begin(), fidsOfJ.end(), intersection.begin());
                    intersection.resize(it - intersection.begin());
                    if (intersection.size() > 1)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    template <typename T>
    std::vector<T> intersectVectors(const std::vector<T>& v1, const std::vector<T>& v2) {
        std::vector<T> v_intersection;

        for (const T& item : v1) {
            if (std::find(v2.begin(), v2.end(), item) != v2.end()) {
                if (std::find(v_intersection.begin(), v_intersection.end(), item) == v_intersection.end()) {
                    v_intersection.push_back(item);
                }
            }
        }

        return v_intersection;
    }


    bool Actions::compareByAdjacentFaces(App::Cursor a, App::Cursor b) {
        int adjFacesA = app().mesh.adj_f2f(a.fid).size();
        int adjFacesB = app().mesh.adj_f2f(b.fid).size();

        return adjFacesA < adjFacesB;
    }

// A template function to compute the set difference between two vectors.
// This function returns the elements in vec1 that are not in vec2.
    template <typename T>
    std::vector<T> vector_difference(std::vector<T> vec1, std::vector<T> vec2) {
        // Sort both vectors to prepare for set_difference algorithm.
        std::sort(vec1.begin(), vec1.end());
        std::sort(vec2.begin(), vec2.end());

        // Prepare a vector to hold the result.
        std::vector<T> result;

        // Resize result to potentially hold the maximum number of elements.
        result.resize(vec1.size());

        // Use std::set_difference to compute the difference.
        auto it = std::set_difference(vec1.begin(), vec1.end(),
                                      vec2.begin(), vec2.end(),
                                      result.begin());

        // Resize the result vector to the actual number of elements after set_difference.
        result.resize(it - result.begin());

        return result;
    }


	void Actions::onExtrude(Dag::Extrude::ESource _source)
	{
		cpputils::collections::FixedVector<Dag::Element*, 3> elements;
		cpputils::collections::FixedVector<I, 3> fis;
		I firstVi;
		bool clockwise;
        std::vector<App::Cursor> cursorsOfClickedElements = app().cursorsOfClickedElements;

        if(cursorsOfClickedElements.size() == 3)
        {
            std::optional<unsigned int> vertexInCommonOptional = getVertexInCommon(app().clickedElements);
            std::vector<Dag::Element*> firstTwoElements = app().clickedElements;
            firstTwoElements.resize(2);
            std::optional<unsigned int> edgeInCommonOptional = getEdgeInCommon(firstTwoElements);
            if (vertexInCommonOptional.has_value() && edgeInCommonOptional.has_value()) {
                unsigned int vertexInCommon = vertexInCommonOptional.value();
                unsigned int edgeInCommon = edgeInCommonOptional.value();
                App::Cursor cursor = app().cursorsOfClickedElements[0];
                cursor.eid = edgeInCommon;
                cursor.ei = Meshing::Utils::ei(app().clickedElements[0]->vids,
                                               Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                cursor.vid = vertexInCommon;
                cursor.vi = Meshing::Utils::vi(app().clickedElements[0]->vids, vertexInCommon);
                if (clickedExtrudeVertexElements(_source, elements, fis, firstVi, clockwise, app().clickedElements[0],
                                                 cursor)) {
                    app().applyAction(*new HMP::Actions::Extrude{elements, fis, firstVi, clockwise});
                    return;
                }
            }
        }
        if (cursorsOfClickedElements.size() == 2)
        {
            std::optional<unsigned int> edgeInCommonOptional = getEdgeInCommon(app().clickedElements);
            if(edgeInCommonOptional.has_value())
            {
                unsigned int edgeInCommon = edgeInCommonOptional.value();
                unsigned int vertexInCommon;
                App::Cursor cursor = app().cursorsOfClickedElements[0];
                cursor.eid = edgeInCommon;
                vertexInCommon = app().mesh.adj_e2v(edgeInCommon)[0];
                cursor.ei = Meshing::Utils::ei(app().clickedElements[0]->vids, Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                cursor.vid = vertexInCommon;
                cursor.vi = Meshing::Utils::vi(app().clickedElements[0]->vids, vertexInCommon);
                if(clickedExtrudeEdgeElements(_source, elements, fis, firstVi, clockwise, app().clickedElements[0], cursor))
                {
                    app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                    return;
                }
            }
        }
        if (!cursorsOfClickedElements.empty())
        {
            std::vector<unsigned int> alreadyFinishedFaces;
            std::vector<unsigned int> allClickedFaces;
            std::vector<unsigned int> oldPids;
            allClickedFaces.reserve(cursorsOfClickedElements.size());
            for(App::Cursor cursor : cursorsOfClickedElements)
            {
                allClickedFaces.push_back(cursor.fid);
            }
            std::vector<std::tuple<App::Cursor, int>> clickedElementsWithAdjFaces;
            for(App::Cursor cursor : cursorsOfClickedElements)
            {
                std::vector<unsigned int> adjacentClickedFaces = intersectVectors(app().mesh.adj_f2f(cursor.fid), allClickedFaces);
                std::tuple<App::Cursor, int> tuple = std::make_tuple(cursor, static_cast<int>(adjacentClickedFaces.size()));
                clickedElementsWithAdjFaces.push_back(tuple);
            }
            std::sort(clickedElementsWithAdjFaces.begin(), clickedElementsWithAdjFaces.end(),
                      [](const std::tuple<App::Cursor, int>& a, const std::tuple<App::Cursor, int>& b) {
                          return std::get<1>(a) > std::get<1>(b);
                      });
            std::vector<App::Cursor> sortedCursors;
            sortedCursors.reserve(clickedElementsWithAdjFaces.size());
            for(const auto tuple : clickedElementsWithAdjFaces)
            {
                sortedCursors.push_back(std::get<0>(tuple));
            }
            int index = 0;
            for(App::Cursor cursor : sortedCursors)
            {
                oldPids.push_back(cursor.pid);
            }
            for(App::Cursor cursor : sortedCursors)
            {
                if(std::find(alreadyFinishedFaces.begin(), alreadyFinishedFaces.end(), cursor.fid) == alreadyFinishedFaces.end())
                {
                    alreadyFinishedFaces.push_back(cursor.fid);
                    if(clickedExtrudeElements(_source, elements, fis, firstVi, clockwise, cursor.element, cursor))
                    {

                        if(checkIfClickedExtrudeIsPossible())
                        {
                            app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                        }

                        if(allClickedFaces.size() == 1)
                            return;
                    }
                }
                unsigned int prevFace = sortedCursors[index].fid;
                std::vector<unsigned int> cursorPid = {cursor.pid};
                bool workedOnAdjacentElements = false;
                std::vector<unsigned int> adjFaces = intersectVectors(app().mesh.adj_f2f(prevFace), allClickedFaces);
                std::vector<unsigned int> prevAdjFaces;
                prevAdjFaces.reserve(adjFaces.size());
                for (int i = 0; i < adjFaces.size(); ++i) {
                    prevAdjFaces.push_back(prevFace);
                }
                for (int i = 0; i < adjFaces.size(); ++i)
                {
                    prevFace = prevAdjFaces[i];
                    unsigned int fid = adjFaces[i];
                    if (std::find(alreadyFinishedFaces.begin(), alreadyFinishedFaces.end(), fid) != alreadyFinishedFaces.end())
                        continue;
                    std::vector<unsigned int> adjPids = app().mesh.adj_f2p(fid);
                    workedOnAdjacentElements = true;
                    auto it = std::find_if(sortedCursors.begin(), sortedCursors.end(), [fid](const App::Cursor& cursor) {
                        return cursor.fid == fid;
                    });
                    cursor = *it;

                    unsigned int edgeInCommon;
                    unsigned int pid1, pid2;
                    bool skipEdgeCheck = false;
                    unsigned int fidInCommon;

                    std::vector<unsigned int> adjWorkedElements = intersectVectors(app().mesh.adj_f2f(fid), alreadyFinishedFaces);
                    if(adjWorkedElements.size() > 1)
                    {
                        std::vector<unsigned int> adjPids1 = vector_difference(app().mesh.adj_f2p(adjWorkedElements[0]), oldPids);
                        std::vector<unsigned int> adjPids2 = vector_difference(app().mesh.adj_f2p(adjWorkedElements[1]), oldPids);
                        if(adjPids1.size() == 1 && adjPids2.size() == 1)
                        {
                            pid1 = adjPids1[0];
                            pid2 = adjPids2[0];
                            std::vector<unsigned int> edges = intersectVectors(app().mesh.adj_p2e(pid1), app().mesh.adj_p2e(pid2));
                            if(edges.size() == 1)
                            {
                                skipEdgeCheck = true;
                                edgeInCommon = edges[0];
                                std::vector<unsigned int> edgesForIntersection1 = app().mesh.adj_f2e(fid);
                                std::vector<unsigned int> edgesForIntersection2 = {edgeInCommon};
                                std::vector<unsigned int> fids1 = app().mesh.adj_p2f(pid1);
                                for(unsigned int fid1 : fids1)
                                {
                                    std::vector<unsigned int> supportVectorOfEdges = app().mesh.adj_f2e(fid1);
                                    std::vector<unsigned int> intersection1 = intersectVectors(supportVectorOfEdges, edgesForIntersection1);
                                    std::vector<unsigned int> intersection2 = intersectVectors(supportVectorOfEdges, edgesForIntersection2);
                                    if(intersection1.size() == 1 && intersection2.size() == 1){
                                        fidInCommon = fid1;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if(!skipEdgeCheck)
                    {
                        std::vector<unsigned int> twoPids = app().mesh.adj_f2p(prevFace);
                        std::vector<unsigned int> adjEdgesFirst = app().mesh.adj_p2e(twoPids[0]);
                        std::vector<unsigned int> adjEdgesSecond = app().mesh.adj_p2e(twoPids[1]);
                        std::vector<unsigned int> intersectedEdgesFirst = intersectVectors(adjEdgesFirst, app().mesh.adj_f2e(cursor.fid));
                        std::vector<unsigned int> intersectedEdgesSecond = intersectVectors(adjEdgesSecond, app().mesh.adj_f2e(cursor.fid));
                        if(intersectedEdgesFirst.size() == 1)
                        {
                            edgeInCommon = intersectedEdgesFirst[0];
                        }
                        else if(intersectedEdgesSecond.size() == 1)
                        {
                            break;
                        }
                        else
                            return;
                    }

                    alreadyFinishedFaces.push_back(fid);
                    unsigned int vertexInCommon;
                    cursor.eid = edgeInCommon;
                    vertexInCommon = app().mesh.adj_e2v(edgeInCommon)[0];
                    if(!skipEdgeCheck)
                    {
                        cursor.ei = Meshing::Utils::ei(cursor.element->vids, Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                        cursor.vid = vertexInCommon;
                        cursor.vi = Meshing::Utils::vi(cursor.element->vids, vertexInCommon);
                    } else
                    {
                        Dag::Element& newElement = app().mesher.element(pid1);
                        cursor.element = &newElement;
                        cursor.pid = pid1;
                        cursor.ei = Meshing::Utils::ei(cursor.element->vids, Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                        cursor.vid = vertexInCommon;
                        cursor.fid = fidInCommon;
                        cursor.vi = Meshing::Utils::vi(cursor.element->vids, vertexInCommon);
                        cursor.fi = Meshing::Utils::fi(cursor.element->vids, Meshing::Utils::fidVids(app().mesh, fidInCommon));
                    }
                    if(clickedExtrudeEdgeElements(_source, elements, fis, firstVi, clockwise, cursor.element, cursor))
                    {
                        app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                    }
                    std::vector<unsigned int> newAdjacentFaces = app().mesh.adj_f2f(fid);
                    newAdjacentFaces = intersectVectors(newAdjacentFaces, allClickedFaces);
                    for(unsigned int newFid : newAdjacentFaces)
                    {
                        if (std::find(adjFaces.begin(), adjFaces.end(), newFid) == adjFaces.end())
                        {
                            adjFaces.push_back(newFid);
                            prevAdjFaces.push_back(fid);
                        }
                    }
                }

                if(!workedOnAdjacentElements && clickedExtrudeElements(_source, elements, fis, firstVi, clockwise, cursor.element, cursor))
                {
                    index++;
                    if (std::find(alreadyFinishedFaces.begin(), alreadyFinishedFaces.end(), cursor.fid) != alreadyFinishedFaces.end())
                        continue;
                    if(checkIfClickedExtrudeIsPossible())
                    {
                        app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                    }
                }
            }

            if(index > 0)
                return;


            /*
            std::vector<unsigned int> allClickedFaces;
            allClickedFaces.reserve(cursorsOfClickedElements.size());
            for(App::Cursor cursor : cursorsOfClickedElements)
            {
                allClickedFaces.push_back(cursor.fid);
            }
            std::vector<std::tuple<App::Cursor, int>> clickedElementsWithAdjFaces;
            for(App::Cursor cursor : cursorsOfClickedElements)
            {
                std::vector<unsigned int> adjacentClickedFaces = intersectVectors(app().mesh.adj_f2f(cursor.fid), allClickedFaces);
                std::tuple<App::Cursor, int> tuple = std::make_tuple(cursor, static_cast<int>(adjacentClickedFaces.size()));
                clickedElementsWithAdjFaces.push_back(tuple);
            }
            std::sort(clickedElementsWithAdjFaces.begin(), clickedElementsWithAdjFaces.end(),
                      [](const std::tuple<App::Cursor, int>& a, const std::tuple<App::Cursor, int>& b) {
                          return std::get<1>(a) > std::get<1>(b);
                      });
            std::vector<App::Cursor> sortedCursors;
            sortedCursors.reserve(clickedElementsWithAdjFaces.size());
            for(const auto tuple : clickedElementsWithAdjFaces)
            {
                sortedCursors.push_back(std::get<0>(tuple));
            }
            int index = 0;
            for (App::Cursor cursor : sortedCursors)
            {
                if(index == 0)
                {
                    if(clickedExtrudeElements(_source, elements, fis, firstVi, clockwise, cursor.element, cursor))
                    {
                        if(checkIfClickedExtrudeIsPossible())
                        {
                            app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                        }
                    }
                } else
                {
                    std::vector<unsigned int> adjFaces = intersectVectors(app().mesh.adj_f2f(cursor.fid), allClickedFaces);
                    for(unsigned int fid : adjFaces)
                    {
                        std::vector<unsigned int> adjPids = app().mesh.adj_f2p(fid);
                        std::vector<unsigned int> cursorPid = {cursor.pid};
                        if(adjPids.size() == 2 && intersectVectors(cursorPid, adjPids).empty())
                        {
                            std::vector<unsigned int> adjEdgesFirst = app().mesh.adj_p2e(adjPids[0]);
                            std::vector<unsigned int> adjEdgesSecond = app().mesh.adj_p2e(adjPids[1]);
                            std::vector<unsigned int> intersectedEdgesFirst = intersectVectors(adjEdgesFirst, app().mesh.adj_f2e(cursor.fid));
                            std::vector<unsigned int> intersectedEdgesSecond = intersectVectors(adjEdgesSecond, app().mesh.adj_f2e(cursor.fid));
                            unsigned int edgeInCommon;
                            if(intersectedEdgesFirst.size() == 1)
                            {
                                edgeInCommon = intersectedEdgesFirst[0];
                            }
                            else if(intersectedEdgesSecond.size() == 1)
                            {
                                edgeInCommon = intersectedEdgesSecond[0];
                            }
                            else
                                return;

                            unsigned int vertexInCommon;
                            cursor.eid = edgeInCommon;
                            vertexInCommon = app().mesh.adj_e2v(edgeInCommon)[0];
                            cursor.ei = Meshing::Utils::ei(cursor.element->vids, Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                            cursor.vid = vertexInCommon;
                            cursor.vi = Meshing::Utils::vi(cursor.element->vids, vertexInCommon);
                            if(clickedExtrudeEdgeElements(_source, elements, fis, firstVi, clockwise, cursor.element, cursor))
                            {
                                app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                            }
                        }
                        else
                        {
                            if(clickedExtrudeElements(_source, elements, fis, firstVi, clockwise, cursor.element, cursor))
                            {
                                if(checkIfClickedExtrudeIsPossible())
                                {
                                    app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                                }
                            }
                        }
                    }
                }
                index++;
            }

            if(index > 0)
                return;*/

            /*for (Dag::Element* element : app().clickedElements)
            {
                if(clickedExtrudeElements(_source, elements, fis, firstVi, clockwise, element, cursorsOfClickedElements[i]))
                {
                    if(checkIfClickedExtrudeIsPossible())
                    {
                        app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                    }
                }
                i++;
            }
            return;*/
        }

        /*if(_source == Dag::Extrude::ESource::Face)
        {
            for (Dag::Element* element : app().clickedElements)
            {
                if(clickedExtrudeElements(_source, elements, fis, firstVi, clockwise, element, cursorsOfClickedElements[i]))
                {
                    if(checkIfClickedExtrudeIsPossible())
                    {
                        app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                    }
                }
                extrudedClickedElements = true;
                i++;
            }
        } else if (_source == Dag::Extrude::ESource::Edge)
        {
            std::optional<unsigned int> edgeInCommonOptional = getEdgeInCommon(app().clickedElements);
            if(edgeInCommonOptional.has_value())
            {
                unsigned int edgeInCommon = edgeInCommonOptional.value();
                unsigned int vertexInCommon;
                App::Cursor cursor = app().cursorsOfClickedElements[0];
                cursor.eid = edgeInCommon;
                vertexInCommon = app().mesh.adj_e2v(edgeInCommon)[0];
                cursor.ei = Meshing::Utils::ei(app().clickedElements[0]->vids, Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                cursor.vid = vertexInCommon;
                cursor.vi = Meshing::Utils::vi(app().clickedElements[0]->vids, vertexInCommon);
                if(clickedExtrudeEdgeElements(_source, elements, fis, firstVi, clockwise, app().clickedElements[0], cursor))
                {
                    app().applyAction(*new HMP::Actions::Extrude{ elements, fis, firstVi, clockwise });
                }
                extrudedClickedElements = true;
            }
        }
        else {
            std::optional<unsigned int> vertexInCommonOptional = getVertexInCommon(app().clickedElements);
            std::vector<Dag::Element*> firstTwoElements = app().clickedElements;
            firstTwoElements.resize(2);
            std::optional<unsigned int> edgeInCommonOptional = getEdgeInCommon(firstTwoElements);
            if (vertexInCommonOptional.has_value() && edgeInCommonOptional.has_value()) {
                unsigned int vertexInCommon = vertexInCommonOptional.value();
                unsigned int edgeInCommon = edgeInCommonOptional.value();
                App::Cursor cursor = app().cursorsOfClickedElements[0];
                cursor.eid = edgeInCommon;
                cursor.ei = Meshing::Utils::ei(app().clickedElements[0]->vids,
                                               Meshing::Utils::eidVids(app().mesh, edgeInCommon));
                cursor.vid = vertexInCommon;
                cursor.vi = Meshing::Utils::vi(app().clickedElements[0]->vids, vertexInCommon);
                if (clickedExtrudeVertexElements(_source, elements, fis, firstVi, clockwise, app().clickedElements[0],
                                               cursor)) {
                    app().applyAction(*new HMP::Actions::Extrude{elements, fis, firstVi, clockwise});
                }
                extrudedClickedElements = true;
            }
        }
        */
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