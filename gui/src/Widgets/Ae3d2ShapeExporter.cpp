#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>

#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/FilePicking.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <algorithm>
#include <imgui.h>
#include <string>
#include <fstream>
#include <ctime>
#include <cassert>
#include <iomanip>
#include <cinolib/quality_hex.h>
#include <HMP/Gui/Widgets/Target.hpp>

namespace HMP::Gui::Widgets
{

	struct JProp final
	{
		const char* key;
	};

	struct JVec final
	{
		const Vec& vec;
	};

	std::ostream& operator<<(std::ostream& _stream, JProp _prop)
	{
		return _stream << '"' << _prop.key << '"' << ':';
	}

	std::ostream& operator<<(std::ostream& _stream, JVec _vec)
	{
		return _stream
			<< '['
			<< _vec.vec.x() << ','
			<< _vec.vec.y() << ','
			<< _vec.vec.z()
			<< ']';
	}

	Ae3d2ShapeExporter::Ae3d2ShapeExporter(): SidebarWidget{ "ae-3d2shape exporter" }, m_keyframes{}, m_sampleError{}
	{}

	bool Ae3d2ShapeExporter::exportKeyframes(const std::vector<Ae3d2ShapeExporter::Keyframe>& _keyframes) const
	{
		static constexpr double c_keyframeDuration{ 1.0 };
		assert(!_keyframes.empty());
		const std::optional<std::string> filename{ Utils::FilePicking::save("json") };
		if (filename)
		{
			std::ofstream file{};
			file.open(*filename);
			file << '{'; // project
			{
				std::time_t time = std::time(nullptr);
				file << JProp{ "name" } << '"' << "HMP export (" << std::put_time(std::localtime(&time), "%d-%m-%Y %H-%M-%S") << ')' << '"'; // project.name
			}
			file << ','; // project
			{
				file << JProp{ "keyframes" } << '['; // project.keyframes
				for (I k{}; k < _keyframes.size(); k++)
				{
					if (k > 0)
					{
						file << ','; // project.keyframes
					}
					file
						<< '{'
						<< JProp{ "time" } << static_cast<double>(k) * c_keyframeDuration << ','
						<< JProp{ "scene" } << '{'; // project.keyframes[k]
					{
						const cinolib::FreeCamera<Real>& camera{ _keyframes[k].camera };
						file << JProp{ "camera" } << '{'; // project.keyframes[k].scene.camera
						{
							file
								<< JProp{ "view" } << '{'
								<< JProp{ "up" } << JVec{ camera.view.up } << ','
								<< JProp{ "eye" } << JVec{ camera.view.eye } << ','
								<< JProp{ "forward" } << JVec{ camera.view.forward } << '}'; // project.keyframes[k].scene.camera.view
						}
						file << ','; // project.keyframes[k].scene.camera
						{
							file << JProp{ "projection" } << '{';  // project.keyframes[k].scene.camera.projection
							if (camera.projection.perspective)
							{
								file
									<< JProp{ "kind" } << '"' << "perspective" << '"' << ','
									<< JProp{ "fov" } << camera.projection.verticalFieldOfView;  // project.keyframes[k].scene.camera.projection
							}
							else
							{
								file
									<< JProp{ "kind" } << '"' << "orthographic" << '"' << ','
									<< JProp{ "scale" } << 1.0 / camera.projection.verticalFieldOfView;  // project.keyframes[k].scene.camera.projection
							}
							file << '}'; // project.keyframes[k].scene.camera.projection
						}
						file << '}'; // project.keyframes[k].scene.camera
					}
					file << ','; // project.keyframes[k].scene
					{
						const cinolib::FreeCamera<Real>& camera{ _keyframes[k].camera };
						file << JProp{ "lights" } << '['
							<< '{'
							<< JProp{ "kind" } << '"' << "directional" << '"' << ','
							<< JProp{ "direction" } << JVec{ camera.view.forward }
						<< '}' << ']'; // project.keyframes[k].scene.lights
					}
					file << ','; // project.keyframes[k].scene
					{
						file << JProp{ "polygons" } << '['; // project.keyframes[k].polygons
						bool firstPolygon{ true };
						for (const Polygon& face : _keyframes[k].polygons)
						{
							if (!firstPolygon)
							{
								file << ','; // project.keyframes[k].scene.polygons
							}
							firstPolygon = false;
							file << '{' << JProp{ "vertices" } << '[';
							bool firstVert{ true };
							for (const Vec& vert : face.vertices)
							{
								if (!firstVert)
								{
									file << ','; // project.keyframes[k].scene.polygons[p].vertices
								}
								firstVert = false;
								file << JVec{ vert };
							}
							file << ']'; // project.keyframes[k].polygons[p].vertices
							constexpr int colorCount{ 8192 };
							const cinolib::Color qualityColor{
								face.quality > 0.0
								? cinolib::Color::parula_ramp(colorCount, static_cast<int>(std::round((1.0 - face.quality) * static_cast<Real>(colorCount - 1))))
								: cinolib::Color::RED()
							};
							const cinolib::Color color{ cinolib::Color::lerp(cinolib::Color::WHITE(), qualityColor, _keyframes[k].qualityFactor) };
							file << ',' << JProp{ "color" } << JVec{ cinolib::vec3f{ color.r(), color.g(), color.b() }.cast<Real>() }; // project.keyframes[k].polygons[p].color
							file << '}'; // project.keyframes[k].polygons[p]
						}
						file << ']'; // project.keyframes[k].scene.polygons
					}
					file << '}'; // project.keyframes[k].scene
					file << '}'; // project.keyframe[k]
				}
				file << ']'; // project.keyframes
			}
			if (m_includeFrameSize)
			{
				file << ','; // project
				{
					file << JProp{ "frameSize" } << '{'
						<< JProp{ "width" } << _keyframes[0].camera.projection.aspectRatio << ','
						<< JProp{ "height" } << 1
						<< '}'; // project.frameSize
				}
			}
			file << '}'; // project
			file.close();
			return true;
		}
		return false;
	}

	bool Ae3d2ShapeExporter::requestExport() const
	{
		return exportKeyframes(m_keyframes);
	}

	bool Ae3d2ShapeExporter::requestTargetExport() const
	{
		Keyframe frame{
			.polygons{},
			.camera { app().canvas.camera },
			.qualityFactor = 0.0
		};
		const cinolib::Polygonmesh<>& mesh{ app().targetWidget.meshForProjection() };
		frame.polygons.reserve(toI(mesh.num_polys()));
		for (Id fid{}; fid < mesh.num_polys(); fid++)
		{
			frame.polygons.push_back(Polygon{
				.vertices{mesh.poly_verts(fid)},
				.quality = 1.0
				});
		}
		return exportKeyframes({ frame });
	}

	bool Ae3d2ShapeExporter::requestSample()
	{
		if (!m_keyframes.empty() && m_keyframes[0].camera.projection.perspective != app().canvas.camera.projection.perspective)
		{
			m_sampleError = "Camera projection kind changed";
			return false;
		}
		Keyframe keyframe{};
		keyframe.camera = app().canvas.camera;
		keyframe.qualityFactor = m_qualityFactor;
		for (Id fid{}; fid < app().mesh.num_faces(); fid++)
		{
			Id pid;
			if (app().mesh.face_is_visible(fid, pid))
			{
				std::vector<Vec> verts{ app().mesh.face_verts(fid) };
				if (app().mesh.poly_face_is_CW(pid, fid))
				{
					std::reverse(verts.begin(), verts.end());
				}
				keyframe.polygons.push_back(Polygon{
					.vertices{verts},
					.quality = cinolib::hex_scaled_jacobian(
						app().mesh.poly_vert(pid, 0),
						app().mesh.poly_vert(pid, 1),
						app().mesh.poly_vert(pid, 2),
						app().mesh.poly_vert(pid, 3),
						app().mesh.poly_vert(pid, 4),
						app().mesh.poly_vert(pid, 5),
						app().mesh.poly_vert(pid, 6),
						app().mesh.poly_vert(pid, 7)
					)
					});
			}
		}
		if (!m_keyframes.empty() && m_keyframes[0].polygons.size() != keyframe.polygons.size())
		{
			m_sampleError = "Number of polygons changed";
			return false;
		}
		m_sampleError = std::nullopt;
		m_keyframes.push_back(keyframe);
		return true;
	}

	void Ae3d2ShapeExporter::clear()
	{
		m_keyframes.clear();
		m_sampleError = std::nullopt;
	}

	I Ae3d2ShapeExporter::keyframeCount() const
	{
		return m_keyframes.size();
	}

	bool Ae3d2ShapeExporter::empty() const
	{
		return m_keyframes.empty();
	}

	void Ae3d2ShapeExporter::drawSidebar()
	{
		Utils::Controls::sliderPercentage("Quality factor", m_qualityFactor);
		if (ImGui::Button("Sample"))
		{
			requestSample();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			clear();
		}
		ImGui::SameLine();
		ImGui::TextColored(empty() ? themer->sbWarn : themer->sbOk, "%d keyframes", static_cast<int>(keyframeCount()));
		if (m_sampleError)
		{
			ImGui::TextColored(themer->sbErr, "%s", m_sampleError->c_str());
		}
		ImGui::Spacing();
		ImGui::Checkbox("Include frame size", &m_includeFrameSize);
		if (Utils::Controls::disabledButton("Export keyframes", !empty()))
		{
			requestExport();
		}
		ImGui::SameLine();
		if (Utils::Controls::disabledButton("Export target", app().targetWidget.hasMesh()))
		{
			requestTargetExport();
		}
	}

}