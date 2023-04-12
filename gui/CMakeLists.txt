cmake_minimum_required (VERSION 3.14)

add_executable (gui 
    "src/main.cpp" 
    "src/App.cpp"
    "src/Widget.cpp"
    "src/SidebarWidget.cpp"
    "src/themer.cpp"
    "src/Utils/HrDescriptions.cpp"
    "src/Utils/Transform.cpp"
    "src/Utils/Controls.cpp"
    "src/Utils/Drawing.cpp"
    "src/Utils/Themer.cpp"
    "src/Utils/FilePicking.cpp"
    "src/Utils/Theme.cpp"
    "src/Widgets/Actions.cpp"
    "src/Widgets/Axes.cpp"
    "src/Widgets/Highlight.cpp"
    "src/Widgets/Pad.cpp"
    "src/Widgets/Smooth.cpp"
    "src/Widgets/Debug.cpp"
    "src/Widgets/Commander.cpp"
    "src/Widgets/Target.cpp"
    "src/Widgets/VertEdit.cpp"
    "src/Widgets/DirectVertEdit.cpp"
    "src/Widgets/Projection.cpp"
    "src/Widgets/Save.cpp"
)

set_target_properties (gui PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_include_directories (gui
    PRIVATE "include"
)

target_link_libraries (gui 
    PRIVATE core
)

target_compile_definitions(gui 
    PUBLIC HMP_GUI_COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\"
)

target_compile_definitions(imgui
    PUBLIC ImDrawIdx=unsigned\ int
)

if (HMP_GUI_ENABLE_DAG_VIEWER)

    target_sources(gui
        PRIVATE "src/DagViewer/createLayout.cpp"
        PRIVATE "src/DagViewer/Layout.cpp"
        PRIVATE "src/DagViewer/Widget.cpp"
    )

    target_link_libraries (gui 
        PRIVATE OGDF
    )

    target_compile_definitions(gui 
        PUBLIC HMP_GUI_ENABLE_DAG_VIEWER
    )

endif()

if (HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER)

    target_sources(gui
        PRIVATE "src/Widgets/Ae3d2ShapeExporter.cpp"
    )

    target_compile_definitions(gui 
        PUBLIC HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
    )

endif()

if (HMP_GUI_CAPTURE)

    target_compile_definitions(gui 
        PUBLIC HMP_GUI_CAPTURE
    )

endif()
