#include <HMP/Gui/App.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <unordered_set>
#include <ctime>
#include <cmath>
#include <string>
#include <chrono>
#include <vector>
#include <GLFW/glfw3.h>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/Root.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/Widgets/Axes.hpp>
#include <HMP/Gui/Widgets/Commander.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/Widgets/DirectVertEdit.hpp>
#include <HMP/Gui/Widgets/Projection.hpp>
#include <HMP/Gui/Widgets/Save.hpp>
#include <HMP/Gui/Widgets/Debug.hpp>
#include <HMP/Gui/Widgets/Pad.hpp>
#include <HMP/Gui/Widgets/Smooth.hpp>
#include <HMP/Gui/Widgets/Highlight.hpp>
#include <HMP/Gui/Widgets/Actions.hpp>
#include <HMP/Gui/Utils/HrDescriptions.hpp>

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/Widget.hpp>
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>
#endif

namespace HMP::Gui
{

	void App::printUsage() const
	{
		std::cout << "------ App key bindings -------\n";
		for (Widget* const widget : m_widgets)
		{
			widget->printUsage();
		}
		std::cout << "-------------------------------\n";
	}

	// actions

	void App::onActionApplied()
	{
		mesher.updateMesh();
		m_canvas.refit_scene(true, false);
		setCursor();
		for (Widget* const widget : m_widgets)
		{
			widget->actionApplied();
		}
		m_canvas.draw();
	}

	void App::applyAction(Commander::Action& _action)
	{
		for (Widget* const widget : m_widgets)
		{
			widget->actionPrepared();
		}
		const auto t1{ std::chrono::high_resolution_clock::now() };
		commander.apply(_action);
		const auto t2{ std::chrono::high_resolution_clock::now() };
		lastActionDurationMicro = static_cast<unsigned long long int>(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
		onActionApplied();
	}

	// mesher events

	void App::onMesherRestored(const Meshing::Mesher::State&)
	{
		if (m_cursor.element && m_cursor.element->pid >= mesh.num_polys())
		{
			m_cursor = {};
			for (Widget* const widget : m_widgets)
			{
				widget->cursorChanged();
			}
		}
		if (copiedElement && copiedElement->pid >= mesh.num_polys())
		{
			copiedElement = nullptr;
		}
	}

	// save events

	void App::serialize(const std::string& _filename) const
	{
		std::ofstream file;
		file.open(_filename);
		HMP::Utils::Serialization::Serializer serializer{ file };
		HMP::Dag::Utils::serialize(serializer, *m_project.root());
		serializer << toI(mesh.num_verts());
		for (const Vec& vert : mesh.vector_verts())
		{
			serializer << vert;
		}
		for (Widget* const widget : m_widgets)
		{
			widget->serialize(serializer);
		}
		file.close();
	}

	void App::deserialize(const std::string& _filename)
	{
		std::ifstream file;
		file.open(_filename);
		HMP::Utils::Serialization::Deserializer deserializer{ file };
		HMP::Dag::Element& root = HMP::Dag::Utils::deserialize(deserializer).element();
		std::vector<Vec> verts(deserializer.get<I>());
		for (Vec& vert : verts)
		{
			deserializer >> vert;
		}
		applyAction(*new Actions::Root{ root, verts });
		m_canvas.reset_camera(true, false);
		for (Widget* const widget : m_widgets)
		{
			widget->deserialize(deserializer);
		}
		file.close();
		m_canvas.draw();
	}

	// canvas events

	void App::onCameraChanged()
	{
		m_cursor = {};
		for (Widget* const widget : m_widgets)
		{
			widget->cameraChanged();
		}
	}

	bool App::onMouseLeftClicked(int)
	{
		for (Widget* const widget : m_widgets)
		{
			if (widget->mouseClicked(false))
			{
				return true;
			}
		}
		return false;
	}

	bool App::onMouseRightClicked(int)
	{
		for (Widget* const widget : m_widgets)
		{
			if (widget->mouseClicked(true))
			{
				return true;
			}
		}
		return false;
	}

	bool App::onKeyPressed(int _key, int _modifiers)
	{
		cinolib::KeyBinding key{ _key, _modifiers };
		for (Widget* const widget : m_widgets)
		{
			if (widget->keyPressed(key))
			{
				return true;
			}
		}
		return false;
	}

	bool App::onMouseMoved(double _x, double _y)
	{
		m_mouse = { _x, _y };
		const Cursor oldCursor{ m_cursor };
		m_cursor = {};
		bool handled{ false };
		for (Widget* const widget : m_widgets)
		{
			if (widget->mouseMoved({ _x, _y }))
			{
				handled = true;
				break;
			}
		}
		if (!handled)
		{
			setCursor();
		}
		if (oldCursor != m_cursor)
		{
			for (Widget* const widget : m_widgets)
			{
				widget->cursorChanged();
			}
		}
		return handled;
	}

	void App::setCursor()
	{
		const cinolib::Ray ray{ canvas.eye_to_mouse_ray() };
		if (mesher.pick(ray.begin(), ray.dir(), m_cursor.pid, m_cursor.fid, m_cursor.eid, m_cursor.vid, !canvas.camera.projection.perspective))
		{
			m_cursor.element = &mesher.element(m_cursor.pid);
			m_cursor.fi = Meshing::Utils::fi(m_cursor.element->vids, Meshing::Utils::fidVids(mesh, m_cursor.fid));
			m_cursor.ei = Meshing::Utils::ei(m_cursor.element->vids, Meshing::Utils::eidVids(mesh, m_cursor.eid));
			m_cursor.vi = Meshing::Utils::vi(m_cursor.element->vids, m_cursor.vid);
		}
		else
		{
			m_cursor = {};
		}
	}

	bool App::updateCursor()
	{
		const Cursor oldCursor{ m_cursor };
		setCursor();
		if (oldCursor != m_cursor)
		{
			for (Widget* const widget : m_widgets)
			{
				widget->cursorChanged();
			}
			return true;
		}
		return false;
	}

	// Commands

	bool App::undo()
	{
		if (!commander.canUndo())
		{
			return false;
		}
		for (Widget* const widget : m_widgets)
		{
			widget->actionPrepared();
		}
		commander.undo();
		onActionApplied();
		return true;
	}

	bool App::redo()
	{
		if (!commander.canRedo())
		{
			return false;
		}
		for (Widget* const widget : m_widgets)
		{
			widget->actionPrepared();
		}
		commander.redo();
		onActionApplied();
		return true;
	}

	void App::onFilesDropped(const std::vector<std::string>& _files)
	{
		if (_files.size() == 1)
		{
			loadTargetMeshOrProjectFile(_files[0]);
		}
	}

	void App::loadTargetMeshOrProjectFile(const std::string& _file)
	{
		static const std::unordered_set<std::string> targetMeshExts{ ".off", ".obj", ".stl" }, projectExts{ ".hmp" };
		std::string ext{ std::filesystem::path{_file}.extension().string() };
		for (char& c : ext) c = static_cast<char>(std::tolower(c));
		if (projectExts.contains(ext))
		{
			saveWidget.requestLoad(_file);
		}
		else if (targetMeshExts.contains(ext))
		{
			targetWidget.load(_file);
		}
		else
		{
			std::cerr << "unknown extension '" << ext << "'" << std::endl;
			std::cout << "only *.off, *.obj and *.stl target mesh files and *.hmp project files are supported" << std::endl;
		}
	}

	void App::onThemeChanged()
	{
		m_canvas.background = themer->bg;
		const std::vector<cinolib::GLcanvas::Font> oldFonts{ m_canvas.fonts };
		m_canvas.fonts = { cinolib::GLcanvas::Font{ static_cast<unsigned int>(std::round(themer->sbScale * 13.0f)), {} } };
		for (Widget* const widget : m_widgets)
		{
			const std::vector<cinolib::GLcanvas::Font> additionalFonts{ widget->additionalFonts(m_canvas.fonts) };
			m_canvas.fonts.insert(m_canvas.fonts.end(), additionalFonts.begin(), additionalFonts.end());
		}
		if (m_canvas.fonts != oldFonts)
		{
			m_canvas.update_fonts();
		}
		mesher.edgeColor = themer->srcEdge;
		mesher.faceColor = themer->srcFace;
		mesher.setEdgeThickness(2.0f * themer->ovScale);
		mesher.updateColors();
	}

	void App::refitScene()
	{
		m_canvas.refit_scene();
	}

	void App::resetCamera()
	{
		m_canvas.reset_camera();
	}

#ifdef HMP_GUI_CAPTURE

	void App::prepareForCapture()
	{
		m_canvas.camera = m_canvas.camera.deserialize("1.24411 0.0117405 117.405 2.60793 0 8.12292 8.21402 -11.186  -0.300145 0.859694 0.413328  -0.505145 -0.51081 0.69563   ");
		m_canvas.camera_pivot_depth(16.0804);
		static constexpr int width{ 1920 }, height{ 1080 }, posX{ 20 }, posY{ 20 };
		m_canvas.camera_settings.near_scene_radius_factor /= 10.0;
		m_canvas.camera_settings.far_scene_radius_factor *= 10.0;
		m_canvas.camera.updateProjectionAndView();
		m_canvas.update_GL_matrices();
		debugWidget.themeScale = 1.75f;
		debugWidget.themeDark = true;
		debugWidget.themeHue = 32.0f;
		debugWidget.updateTheme();
		targetWidget.edgeThickness = 1.0f;
		//targetWidget.edgeColor = cinolib::Color{ 0.0f / 255.0f, 116.0f / 255.0f, 74.0f / 255.0f, 61.0f / 255.0f };
		//targetWidget.faceColor = cinolib::Color{ 0.0f / 255.0f, 255.0f / 255.0f, 138.0f / 255.0f, 20.0f / 255.0f };
		//m_canvas.show_sidebar(true, true, false);
		glfwSetWindowPos(m_canvas.window, posX, posY);
		glfwSetWindowSize(m_canvas.window, width, height);
		glfwSetWindowPosCallback(m_canvas.window, [](GLFWwindow* w, int, int)
			{
				glfwSetWindowPos(w, posX, posY);
				glfwSetWindowSize(w, width, height);
			});
		/*
		glfwSetWindowFocusCallback(m_canvas.window, [](GLFWwindow* w, int)
			{
				glfwFocusWindow(w);
			});
		*/
#ifdef NDEBUG
#define HMP_GUI_APP_TITLE HMP_NAME " v" HMP_VERSION " (CAPTURE)"
#else
#define HMP_GUI_APP_TITLE HMP_NAME " v" HMP_VERSION " (DEV)" " (CAPTURE)"
#endif
		glfwSetWindowTitle(m_canvas.window, HMP_GUI_APP_TITLE);
#undef HMP_GUI_APP_TITLE
	}

#endif

	// launch

	App::App() :
		m_project{}, mesher{ m_project.mesher() }, mesh{ mesher.mesh() }, commander{ m_project.commander() },
		m_canvas{ 700, 600, 13, 1 }, canvas{ m_canvas },
		dagNamer{},
		m_mouse{}, m_cursor{}, mouse{ m_mouse }, cursor{ m_cursor },
		commanderWidget{ *new Widgets::Commander{} },
		axesWidget{ *new Widgets::Axes{} },
		targetWidget{ *new Widgets::Target{} },
		vertEditWidget{ *new Widgets::VertEdit{} },
		directVertEditWidget{ *new Widgets::DirectVertEdit{} },
		saveWidget{ *new Widgets::Save{} },
		projectionWidget{ *new Widgets::Projection{} },
		debugWidget{ *new Widgets::Debug{} },
		padWidget{ *new Widgets::Pad{} },
		smoothWidget{ *new Widgets::Smooth{} },
		highlightWidget{ *new Widgets::Highlight{} },
		actionsWidget{ *new Widgets::Actions{} },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		dagViewerWidget{ *new DagViewer::Widget{} },
#endif
#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		ae3d2ShapeExporter{ *new Widgets::Ae3d2ShapeExporter{} },
#endif
		m_widgets{
			&debugWidget,
			&saveWidget,
			&commanderWidget,
			&highlightWidget,
			&vertEditWidget,
			&directVertEditWidget,
			&padWidget,
			&smoothWidget,
			&targetWidget,
			&projectionWidget,
			&axesWidget,
			&actionsWidget,
#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
			& ae3d2ShapeExporter,
#endif
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
			& dagViewerWidget,
#endif
	}
	{

#ifdef NDEBUG
#define HMP_GUI_APP_TITLE HMP_NAME " v" HMP_VERSION
#else
#define HMP_GUI_APP_TITLE HMP_NAME " v" HMP_VERSION " (DEV)"
#endif
		glfwSetWindowTitle(m_canvas.window, HMP_GUI_APP_TITLE);
#undef HMP_GUI_APP_TITLE

		m_canvas.key_bindings.reset_camera = GLFW_KEY_P;
		m_canvas.key_bindings.store_camera = { GLFW_KEY_C, GLFW_MOD_ALT };
		m_canvas.key_bindings.restore_camera = { GLFW_KEY_V, GLFW_MOD_ALT };
		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_left_click = [this](auto && ..._args) { return onMouseLeftClicked(_args ...); };
		m_canvas.callback_mouse_right_click = [this](auto && ..._args) { return onMouseRightClicked(_args ...); };
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMoved(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPressed(_args...); };
		m_canvas.callback_key_event = [this](auto && ...) { updateCursor(); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChanged(_args...); };
		m_canvas.callback_drop_files = [this](std::vector<std::string> _files) { onFilesDropped(_files); };
		m_canvas.push(&mesh);

		themer.onThemeChange += [this]() { onThemeChanged(); };

		mesher.onRestored += [this](const Meshing::Mesher::State& _oldState) { onMesherRestored(_oldState); };

		commander.applied().limit(100);
		commander.unapplied().limit(100);

		for (Widget* widget : m_widgets)
		{
			widget->m_app = this;
			m_canvas.push(static_cast<cinolib::CanvasGuiItem*>(widget));
			if (SidebarWidget* const sidebarWidget{ dynamic_cast<SidebarWidget*>(widget) })
			{
				m_canvas.push(static_cast<cinolib::SideBarItem*>(sidebarWidget));
			}
			for (const cinolib::DrawableObject* additionalDrawable : widget->additionalDrawables())
			{
				m_canvas.push(additionalDrawable, false);
			}
		}

		for (Widget* widget : m_widgets)
		{
			widget->attached();
		}

		actionsWidget.clear();
		commander.applied().clear();

		debugWidget.updateTheme();

		resetCamera();

#ifdef HMP_GUI_CAPTURE
		prepareForCapture();
#endif

	}

	App::~App()
	{
		for (Widget* const widget : m_widgets)
		{
			delete widget;
		}
	}

	const Dag::Element& App::root() const
	{
		return *m_project.root();
	}

	int App::launch()
	{
		m_canvas.print_key_bindings();
		printUsage();
		try
		{
			return m_canvas.launch({}, false);
		}
		catch (...)
		{
			const std::time_t time{ std::time(nullptr) };
			const std::tm* now{ std::localtime(&time) };
			std::ostringstream filenameSs{};
			filenameSs << "crash_"
				<< std::put_time(now, "%H-%M-%S_%d-%m-%y")
				<< ".hmp";
			{
				const std::string filename{ filenameSs.str() };
				serialize(filename);
				std::cout << "Wrote state to " << std::filesystem::absolute(filename) << std::endl;
			}
			throw;
		}
	}

	int App::run(const std::optional<std::string>& _file)
	{
		App app{};
		if (_file)
		{
			app.loadTargetMeshOrProjectFile(*_file);
		}
		return app.launch();
	}

}