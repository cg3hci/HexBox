#include <HMP/Gui/Widgets/Highlight.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <sstream>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/Utils/HrDescriptions.hpp>

namespace HMP::Gui::Widgets
{

    bool Highlight::mouseClicked(bool _right)
    {
        app().clickedWidgets.push_back(this);
        return false;
    }

	void Highlight::drawCanvas()
	{
		const float
			smallVertRadius{ 4.0f * themer->ovScale },
			vertRadius{ 6.0f * themer->ovScale },
			lineThickness{ 1.5f * themer->ovScale },
			boldLineThickness{ 3.5f * themer->ovScale },
			lineSpacing{ (10.0f * themer->ovScale + 10.0f) / 2.0f };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		using namespace Utils::Drawing;
		if (app().copiedElement && !app().cursor.element)
		{
			const Id cPid{ app().copiedElement->pid };
			const Dag::Extrude& extrude{ app().copiedElement->parents.single().as<Dag::Extrude>() };
			const auto cPidCenter2d{ Utils::Drawing::project(app().canvas, app().mesh.poly_centroid(cPid)) };
			if (cPidCenter2d)
			{
				for (const auto& [parent, fi] : extrude.parents.zip(extrude.fis))
				{
					const QuadVertIds parentFidVids{ Meshing::Utils::fiVids(parent.vids, fi) };
					const Vec parentFidCenter{ Meshing::Utils::centroid(Meshing::Utils::verts(app().mesh, parentFidVids)) };
					const auto parentFidCenter2d{ Utils::Drawing::project(app().canvas, parentFidCenter) };
					if (parentFidCenter2d)
					{
						dashedLine(drawList, { *parentFidCenter2d, *cPidCenter2d }, themer->ovMut, lineThickness, lineSpacing);
					}
				}
			}
			circle(drawList, cPidCenter2d, smallVertRadius, app().cursor.element == app().copiedElement ? themer->ovHi : themer->ovMut, lineThickness);
			const Dag::Element& firstParent{ extrude.parents.first() };
			const Id firstVid{ firstParent.vids[extrude.firstVi] };
			const QuadVertIds firstParentVids{ Meshing::Utils::align(Meshing::Utils::fiVids(firstParent.vids, extrude.fis[0]), firstVid, extrude.clockwise) };
			const auto eid2d{ Utils::Drawing::project(app().canvas, Meshing::Utils::verts(app().mesh, EdgeVertIds{ firstParentVids[0], firstParentVids[1] })) };
			dashedLine(drawList, eid2d, themer->ovMut, boldLineThickness, lineSpacing);
			if (eid2d)
			{
				circle(drawList, (*eid2d)[0], vertRadius, themer->ovMut, lineThickness);
			}
		}
        int offset = 0;
        for (const auto& clickedFace : app().clickedElements) {
            const QuadVerts clickedVerts{ Meshing::Utils::verts(app().mesh, Meshing::Utils::fiVids(clickedFace->vids, app().cursorsOfClickedElements[offset].fi)) };
            const auto clickedVerts2d{ Utils::Drawing::project(app().canvas, clickedVerts) };
            quadFilled(drawList, clickedVerts2d, themer->ovFaceHi);//fi == app().cursorsOfClickedElements[offset].fi ? themer->ovFaceHi : themer->ovPolyHi);
            offset++;
        }
		if (app().cursor.element)
		{
			for (I i{}; i < 6; i++)
			{
				const I fi{ (i + 1 + app().cursor.fi) % 6 };
				const QuadVerts fiVerts{ Meshing::Utils::verts(app().mesh, Meshing::Utils::fiVids(app().cursor.element->vids, fi)) };
				const auto fiVerts2d{ Utils::Drawing::project(app().canvas, fiVerts) };

                quadFilled(drawList, fiVerts2d, fi == app().cursor.fi ? themer->ovFaceHi : themer->ovPolyHi);
			}
			const auto hPidCenter2d{ Utils::Drawing::project(app().canvas, app().mesh.poly_centroid(app().cursor.pid)) };
			if (hPidCenter2d)
			{
				for (const Id adjPid : app().mesh.adj_p2p(app().cursor.pid))
				{
					const Id adjFid{ static_cast<Id>(app().mesh.poly_shared_face(app().cursor.pid, adjPid)) };
					const auto adjFidCenter2d{ Utils::Drawing::project(app().canvas, app().mesh.face_centroid(adjFid)) };
					const auto adjPidCenter2d{ Utils::Drawing::project(app().canvas, app().mesh.poly_centroid(adjPid)) };
					const ImU32 adjColorU32{ app().mesher.shown(adjPid) ? themer->ovHi : themer->ovMut };
					circle(drawList, adjPidCenter2d, smallVertRadius, adjColorU32, lineThickness);
					if (adjFidCenter2d)
					{
						dashedLine(drawList, { *adjFidCenter2d, *hPidCenter2d }, adjColorU32, lineThickness, lineSpacing);
					}
				}
				circle(drawList, *hPidCenter2d, smallVertRadius, themer->ovHi, lineThickness);
			}
			for (const Id adjEid : app().mesh.adj_f2e(app().cursor.fid))
			{
				const auto adjEid2d{ Utils::Drawing::project(app().canvas, Meshing::Utils::verts(app().mesh, Meshing::Utils::eidVids(app().mesh, adjEid))) };
				dashedLine(drawList, adjEid2d, adjEid == app().cursor.eid ? themer->ovHi : themer->ovMut, boldLineThickness, lineSpacing);
			}
			const auto hVert2d{ Utils::Drawing::project(app().canvas, app().mesh.vert(app().cursor.vid)) };
			circle(drawList, hVert2d, vertRadius, themer->ovHi, lineThickness);
		}
		if (app().cursor.element)
		{
			std::ostringstream stream{};
			stream
				<< "Hovering "
				<< Utils::HrDescriptions::name(*app().cursor.element, app().dagNamer)
				<< " ("
				<< "face " << app().cursor.fi
				<< ", edge " << app().cursor.ei
				<< ", vert " << app().cursor.vi
				<< ")";
			ImGui::TextColored(toImVec4(themer->ovMut), "%s", stream.str().c_str());
		}
		if (app().copiedElement)
		{
			std::ostringstream stream{};
			stream
				<< "Copied"
				<< " " << Utils::HrDescriptions::name(*app().copiedElement, app().dagNamer);
			ImGui::TextColored(toImVec4(themer->ovMut), "%s", stream.str().c_str());
		}
	}

}