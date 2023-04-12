#pragma once

#include <cinolib/color.h>
#include <imgui.h>

namespace HMP::Gui::Utils
{

    struct Theme final
    {

        float ovScale;
        float sbScale;
        float hue;
        bool dark;
        ImVec4 sbOk;
        ImVec4 sbWarn;
        ImVec4 sbErr;
        cinolib::Color bg;
        ImU32 ovHi;
        ImU32 ovMut;
        ImU32 ovWarn;
        ImU32 ovErr;
        ImU32 ovPolyHi;
        ImU32 ovFaceHi;
        cinolib::Color srcFace;
        cinolib::Color srcEdge;
        cinolib::Color tgtFace;
        cinolib::Color tgtEdge;
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
        ImU32 dagNodeEl;
        ImU32 dagNodeElHi;
        ImU32 dagNodeElMut;
        ImU32 dagNodeRefine;
        ImU32 dagNodeExtrude;
        ImU32 dagNodeDelete;
#endif
        float ovAxesSat;
        float ovAxesVal;
        float ovPathSat;
        float ovPathVal;

        static Theme makeLight(float _hueDeg = 0.0f, float _scale = 1.0f);

        static Theme makeDark(float _hueDeg = 0.0f, float _scale = 1.0f);

        static Theme make(bool _dark = true, float _hueDeg = 0.0f, float _scale = 1.0f);

    };

}