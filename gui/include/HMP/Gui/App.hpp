#pragma once

#include <HMP/Project.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/gl/glcanvas.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/SetNamer.hpp>
#include <string>
#include <vector>
#include <optional>
#include <HMP/Gui/Widget.hpp>

namespace HMP::Gui
{

	namespace DagViewer
	{
		class Widget;
	}

	namespace Widgets
	{

		class Axes;
		class Debug;
		class Save;
		class Pad;
		class VertEdit;
		class DirectVertEdit;
		class Target;
		class Projection;
		class Smooth;
		class Ae3d2ShapeExporter;
		class Commander;
		class Actions;
		class Highlight;

	}

	class App final : public cpputils::mixins::ReferenceClass
	{

	public:

		struct Cursor final
		{
			Dag::Element* element{};
			I fi{}, vi{}, ei{};
			Id pid{ noId }, fid{ noId }, vid{ noId }, eid{ noId };

			bool operator==(const Cursor&) const = default;
		};

	private:

		void printUsage() const;

		HMP::Project m_project;
		cinolib::GLcanvas m_canvas;
		Vec2 m_mouse;
		Cursor m_cursor;

	public:

		const cinolib::GLcanvas& canvas;
		Meshing::Mesher& mesher;
		const Meshing::Mesher::Mesh& mesh;
		Commander& commander;
		cpputils::collections::SetNamer<const HMP::Dag::Node*> dagNamer;
		const Vec2& mouse;
		const Cursor& cursor;
		unsigned long long int lastActionDurationMicro{};

		Widgets::Actions& actionsWidget;
		Widgets::Commander& commanderWidget;
		Widgets::Axes& axesWidget;
		Widgets::Target& targetWidget;
		Widgets::VertEdit& vertEditWidget;
		Widgets::DirectVertEdit& directVertEditWidget;
		Widgets::Save& saveWidget;
		Widgets::Projection& projectionWidget;
		Widgets::Debug& debugWidget;
		Widgets::Pad& padWidget;
		Widgets::Smooth& smoothWidget;
		Widgets::Highlight& highlightWidget;

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		DagViewer::Widget& dagViewerWidget;
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		Widgets::Ae3d2ShapeExporter& ae3d2ShapeExporter;
#endif

	private:

		const std::vector<Widget*> m_widgets;

		// actions
		void onActionApplied();

		// other events
		void onThemeChanged();

		// mesher events
		void onMesherRestored(const Meshing::Mesher::State& _state);

		// canvas events
		void onCameraChanged();
		bool onMouseLeftClicked(int _modifiers);
		bool onMouseRightClicked(int _modifiers);
		bool onKeyPressed(int _key, int _modifiers);
		bool onMouseMoved(double _x, double _y);
		void onFilesDropped(const std::vector<std::string>& _files);

		void loadTargetMeshOrProjectFile(const std::string& _file);

		int launch();

		void setCursor();

#ifdef HMP_GUI_CAPTURE
		void prepareForCapture();
#endif

		App();
		~App();

	public:

		static int run(const std::optional<std::string>& _file = std::nullopt);

		Dag::Element* copiedElement{};

		const Dag::Element& root() const;

		bool redo();
		void applyAction(Commander::Action& _action);
		bool undo();

		bool updateCursor();
		void refitScene();
		void resetCamera();

		void serialize(const std::string& _filename) const;
		void deserialize(const std::string& _filename);

	};

}