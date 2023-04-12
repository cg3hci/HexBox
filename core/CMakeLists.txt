cmake_minimum_required (VERSION 3.14)

add_library (core STATIC
    "src/Actions/Delete.cpp"
    "src/Actions/DeleteSome.cpp"
    "src/Actions/FitCircle.cpp"
    "src/Actions/Extrude.cpp"
    "src/Actions/Root.cpp"
    "src/Actions/MakeConforming.cpp"
    "src/Actions/Paste.cpp"
    "src/Actions/Project.cpp"
    "src/Actions/Refine.cpp"
    "src/Actions/RefineSome.cpp"
    "src/Actions/Transform.cpp"
    "src/Actions/ExtrudeUtils.cpp"
    "src/Actions/Pad.cpp"
    "src/Actions/Smooth.cpp"
    "src/Actions/SubdivideAll.cpp"
    "src/Actions/SplitPlane.cpp"
    "src/Dag/Delete.cpp"
    "src/Dag/Element.cpp"
    "src/Dag/Extrude.cpp"
    "src/Dag/Node.cpp"
    "src/Dag/NodeSet.cpp"
    "src/Dag/Operation.cpp"
    "src/Dag/Refine.cpp"
    "src/Dag/Utils.cpp"
    "src/Meshing/Mesher.cpp"
    "src/Meshing/Utils.cpp"
    "src/Refinement/Scheme.cpp"
    "src/Refinement/Schemes.cpp"
    "src/Refinement/Utils.cpp"
    "src/Refinement/Sub3x3AdapterCandidate.cpp"
    "src/Refinement/Sub3x3AdapterCandidateSet.cpp"
    "src/Projection/project.cpp"
    "src/Projection/Utils.cpp"
    "src/Projection/fill.cpp"
    "src/Projection/smooth.cpp"
    "src/Projection/percentileAdvance.cpp"
    "src/Projection/jacobianAdvance.cpp"
    "src/Projection/Match.cpp"
    "src/Utils/Serialization.cpp"
    "src/Commander.cpp"
    "src/Project.cpp"
)

set_target_properties (core PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

if (MSVC AND CINOLIB_HEADER_ONLY)
    target_compile_options (core PRIVATE /bigobj)
endif()

target_include_directories (core
    PUBLIC "include"
    PRIVATE ${updatable_priority_queue_SOURCE_DIR}
)

target_link_libraries (core 
    PUBLIC cinolib
    PUBLIC cpputils
)

if (HMP_ENABLE_ALT_PROJ)

    target_sources(core
        PRIVATE "src/Projection/altProject.cpp"
    )

    target_link_libraries(core
        PUBLIC libhexsmoothing
    )

endif()
