#include "Shadertoy.h"
#include "APIKey.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib/httplib.h>
#include <json11/json11.hpp>
#include <pugixml/src/pugixml.hpp>
#include <ghc/filesystem.hpp>

#define BUTTON_SPACE_LEFT -40 * GetDPI()
#define KEYBOARD_TEXTURE_NAME "KeyboardTexture"

namespace st
{
	struct ShaderOutput
	{
		int ID;
		int Channel;
	};
	struct ShaderInputSampler
	{
		std::string Filter;
		std::string Wrap;
		bool FlipVertical;
		bool SRGB;
	};
	struct ShaderInput
	{
		int ID;
		int Channel;
		std::string Type;
		std::string Source;

		ShaderInputSampler Sampler;
	};
	struct RenderPass
	{
		std::vector<ShaderOutput> Outputs;
		std::vector<ShaderInput> Inputs;

		std::string Name;
		std::string Type;
		std::string Code;
	};

	std::string GenerateReadMe(const json11::Json& info)
	{
		std::string ret = "";

		ret += "Name: " + info["name"].string_value() + "\n";
		ret += "Shader made by: " + info["username"].string_value() + "\n";
		ret += "Link: www.shadertoy.com/view/" + info["id"].string_value() + "\n";

		return ret;
	}

	std::vector<ShaderOutput> ParseOutputs(const json11::Json& outputs)
	{
		std::vector<ShaderOutput> ret;
		if (outputs.is_array()) {
			for (const auto& output : outputs.array_items()) {
				ShaderOutput data;
				data.Channel = output["channel"].int_value();
				data.ID = output["id"].int_value();
				ret.push_back(data);
			}
		}
		return ret;
	}
	std::vector<ShaderInput> ParseInputs(const json11::Json& outputs)
	{
		std::vector<ShaderInput> ret;
		if (outputs.is_array()) {
			for (const auto& output : outputs.array_items()) {
				ShaderInput data;
				data.Channel = output["channel"].int_value();
				data.ID = output["id"].int_value();
				data.Source = output["src"].string_value();
				data.Type = output["ctype"].string_value();

				data.Sampler.Filter = output["sampler"]["filter"].string_value();
				data.Sampler.Wrap = output["sampler"]["wrap"].string_value();
				data.Sampler.FlipVertical = output["sampler"]["vflip"].bool_value();
				data.Sampler.SRGB = output["sampler"]["srgb"].bool_value();

				ret.push_back(data);
			}
		}
		return ret;
	}

	std::vector<RenderPass> ParseRenderPasses(const json11::Json& rpassContainer)
	{
		std::vector<RenderPass> ret;

		if (rpassContainer.is_array()) {
			for (const auto& rpass : rpassContainer.array_items()) {
				RenderPass data;

				data.Inputs = ParseInputs(rpass["inputs"]);
				data.Outputs = ParseOutputs(rpass["outputs"]);

				data.Name = rpass["name"].string_value();
				data.Type = rpass["type"].string_value();
				data.Code = rpass["code"].string_value();

				ret.push_back(data);
			}
		}

		return ret;
	}

	std::string GenerateItems(int index)
	{
		std::string ret =
			"<items>\n"
			"<item name=\"ScreenQuad" + std::to_string(index) + "\" type=\"geometry\">\n"
			"<type>ScreenQuadNDC</type>\n"
			"<width>1</width>\n"
			"<height>1</height>\n"
			"<depth>1</depth>\n"
			"<topology>TriangleList</topology>\n"
			"</item>\n"
			"</items>";
		return ret;
	}
	std::string GenerateVariables()
	{
		std::string ret =
			"<variables>"
			"<variable type=\"float2\" name=\"iResolution\" system=\"ViewportSize\" />"
			"<variable type=\"float\" name=\"iTime\" system=\"Time\" />"
			"<variable type=\"float\" name=\"iTimeDelta\" system=\"TimeDelta\" />"
			"<variable type=\"int\" name=\"iFrame\" system=\"FrameIndex\" />"
			"<variable type=\"float4\" name=\"iMouse\" system=\"MouseButton\" />"
			"</variables>";
		return ret;
	}
	std::string GenerateSettings()
	{
		std::string ret =
			"<entry type=\"camera\" fp=\"false\">"
			"<distance>10</distance>"
			"<pitch>0</pitch>"
			"<yaw>0</yaw>"
			"<roll>0</roll>"
			"</entry>"
			"<entry type=\"clearcolor\" r=\"0\" g=\"0\" b=\"0\" a=\"0\" />"
			"<entry type=\"usealpha\" val=\"false\" />";

		return ret;
	}
	std::string GenerateVertexShader()
	{
		const char* vs = R"(#version 330

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

out vec2 outUV;

void main() {
	gl_Position = vec4(pos, 0.0, 1.0);
	outUV = uv;
}
)";
		return std::string(vs);
	}
	std::string GenerateGLSL(const std::string& code, bool usesCommon = false)
	{
		std::string ret = "#version 330\n\n" +
			std::string(usesCommon ? "#include <common.glsl>\n" : "") +
			"uniform vec2 iResolution;\n"
			"uniform float iTime;\n"
			"uniform float iTimeDelta;\n"
			"uniform int iFrame;\n"
			"uniform vec4 iMouse;\n"
			"uniform sampler2D iChannel0;\n"
			"uniform sampler2D iChannel1;\n"
			"uniform sampler2D iChannel2;\n"
			"uniform sampler2D iChannel3;\n"
			"out vec4 shadertoy_outcolor;\n\n" + code + "\n"
			"void main()\n{\n"
			"\tmainImage(shadertoy_outcolor, gl_FragCoord.xy);\n"
			"}";
		return ret;
	}
	pugi::xml_document GenerateProject(const std::vector<RenderPass>& data)
	{
		pugi::xml_document doc;
		pugi::xml_node project = doc.append_child("project");
		project.append_attribute("version").set_value(2);

		pugi::xml_node pipelineNode = project.append_child("pipeline");
		pugi::xml_node objectsNode = project.append_child("objects");
		pugi::xml_node settingsNode = project.append_child("settings");

		/////// BUILD RESOURCE LIST ///////
		int index = 0;
		std::vector<std::string> rts;
		std::vector<int> rtIds;
		std::map<int, std::vector<std::pair<std::string, int>>> rtBind;
		std::vector<std::string> textures, textureTypes;
		std::map<std::string, std::vector<std::pair<std::string, int>>> texBinds;
		for (const auto& rpass : data) {
			if (rpass.Type == "buffer") {
				rts.push_back(rpass.Name);
				rtIds.push_back(rpass.Outputs[0].ID);
			}
			for (const auto& inp : rpass.Inputs) {
				if (inp.Type == "texture" || inp.Type == "keyboard") {
					std::string name = inp.Source;
					if (inp.Type == "keyboard")
						name = KEYBOARD_TEXTURE_NAME;

					if (std::count(textures.begin(), textures.end(), name) == 0)
						textures.push_back(name);

					texBinds[name].push_back(std::make_pair(rpass.Name, inp.Channel));
				}
				else if (inp.Type == "buffer")
					rtBind[inp.ID].push_back(std::make_pair(rpass.Name, inp.Channel));
			}

			index++;
		}

		/////// PIPELINE ///////
		for (int i = data.size() - 1; i >= 0; i--) {
			const RenderPass& pass = data[i];

			if (pass.Type == "common")
				continue;

			pugi::xml_node node = pipelineNode.append_child("pass");
			node.append_attribute("name").set_value(pass.Name.c_str());
			node.append_attribute("type").set_value("shader");
			node.append_attribute("active").set_value("true");

			pugi::xml_node vsNode = node.append_child("shader");
			vsNode.append_attribute("type").set_value("vs");
			vsNode.append_attribute("path").set_value("shaders/shadertoyVS.glsl");

			pugi::xml_node psNode = node.append_child("shader");
			psNode.append_attribute("type").set_value("ps");
			psNode.append_attribute("path").set_value(("shaders/" + pass.Name + ".glsl").c_str());

			if (pass.Type == "buffer")
				node.append_child("rendertexture").append_attribute("name").set_value(pass.Name.c_str());
			else
				node.append_child("rendertexture");

			std::string itemsNode = GenerateItems(data.size() - i);
			node.append_buffer(itemsNode.c_str(), itemsNode.size());

			std::string varNode = GenerateVariables();
			node.append_buffer(varNode.c_str(), varNode.size());
		}


		/////// OBJECTS ///////
		for (int i = 0; i < rts.size(); i++) {
			pugi::xml_node node = objectsNode.append_child("object");
			node.append_attribute("type").set_value("rendertexture");
			node.append_attribute("name").set_value(rts[i].c_str());
			node.append_attribute("rsize").set_value("1.00,1.00");
			node.append_attribute("clear").set_value("true");
			node.append_attribute("r").set_value("0");
			node.append_attribute("g").set_value("0");
			node.append_attribute("b").set_value("0");
			node.append_attribute("a").set_value("1");

			const std::vector<std::pair<std::string, int>>& myBind = rtBind[rtIds[i]];
			for (int j = 0; j < myBind.size(); j++) {
				auto& pair = myBind[j];
				pugi::xml_node bindNode = node.append_child("bind");
				bindNode.append_attribute("slot").set_value(pair.second);
				bindNode.append_attribute("name").set_value(pair.first.c_str());
			}
		}
		for (int i = 0; i < textures.size(); i++) {
			pugi::xml_node node = objectsNode.append_child("object");
			node.append_attribute("type").set_value("texture");

			if (textures[i] == KEYBOARD_TEXTURE_NAME) {
				node.append_attribute("name").set_value(textures[i].c_str());
				node.append_attribute("keyboard_texture").set_value(true);
			} else {
				node.append_attribute("path").set_value(("." + textures[i]).c_str());

				ShaderInputSampler samplerInfo;
				for (int j = 0; j < data.size(); j++)
					for (int k = 0; k < data[j].Inputs.size(); k++) {
						if (data[j].Inputs[k].Source == textures[i])
							samplerInfo = data[j].Inputs[k].Sampler;
					}

				// vertical flip
				node.append_attribute("vflip").set_value(samplerInfo.FlipVertical);

				// filter
				if (samplerInfo.Filter == "linear") {
					node.append_attribute("min_filter").set_value("Nearest");
					node.append_attribute("mag_filter").set_value("Nearest");
				} else if (samplerInfo.Filter == "nearest") {
					node.append_attribute("min_filter").set_value("Linear");
					node.append_attribute("mag_filter").set_value("Linear");
				} else if (samplerInfo.Filter == "mipmap") {
					/* TODO: not sure what this is supposed to be */
					node.append_attribute("min_filter").set_value("Linear_MipmapLinear");
					node.append_attribute("mag_filter").set_value("Linear");
				}

				// wrap
				if (samplerInfo.Wrap == "clamp") {
					node.append_attribute("wrap_s").set_value("ClampToEdge");
					node.append_attribute("wrap_t").set_value("ClampToEdge");
				} else if (samplerInfo.Wrap == "repeat") {
					node.append_attribute("wrap_s").set_value("Repeat");
					node.append_attribute("wrap_t").set_value("Repeat");
				}
			}

			const std::vector<std::pair<std::string, int>>& myBind = texBinds[textures[i]];
			for (int j = 0; j < myBind.size(); j++) {
				auto& pair = myBind[j];
				pugi::xml_node bindNode = node.append_child("bind");
				bindNode.append_attribute("slot").set_value(pair.second);
				bindNode.append_attribute("name").set_value(pair.first.c_str());
			}
		}

		/////// SETTINGS ///////
		std::string settings = GenerateSettings();
		settingsNode.append_buffer(settings.c_str(), settings.size());


		return doc;
	}

	void WriteFile(const std::string& filename, const std::string& filedata)
	{
		std::ofstream file(filename);
		file << filedata;
		file.close();
	}
	bool Generate(const std::string& shadertoyID, const std::string& outPath)
	{
		// https://www.shadertoy.com/api/v1/shaders/shaderID?key=appkey
		httplib::SSLClient cli("www.shadertoy.com");

		auto res = cli.Get(("/api/v1/shaders/" + shadertoyID + "?key=" SHADERTOY_APIKEY).c_str());

		std::vector<RenderPass> pipeline;

		if (res && res->status == 200) {
			std::string err;
			json11::Json jdata = json11::Json::parse(res->body, err);

			if (jdata["Error"].is_string()) {
				return false;
			}

			if (jdata.is_object()) {
				pipeline = ParseRenderPasses(jdata["Shader"]["renderpass"]);
			}

			if (!ghc::filesystem::exists(outPath))
				ghc::filesystem::create_directories(outPath);

			std::string shadersDir = outPath + "/shaders";
			if (!ghc::filesystem::exists(shadersDir))
				ghc::filesystem::create_directories(shadersDir);

			// README.txt
			WriteFile(outPath + "/README.txt", GenerateReadMe(jdata["Shader"]["info"]));

			// project.sprj
			pugi::xml_document doc = GenerateProject(pipeline);
			std::ofstream sprjFile(outPath + "/project.sprj");
			doc.print(sprjFile);
			sprjFile.close();

			// shaders
			bool usesCommon = false;
			for (const auto& item : pipeline) {
				if (item.Type == "common") {
					usesCommon = true;
					WriteFile(outPath + "/common.glsl", item.Code);
					break;
				}
			}

			for (const auto& item : pipeline) {
				if (item.Type == "common")
					continue;
				std::string shaderPath = outPath + "/shaders/" + item.Name + ".glsl";
				WriteFile(shaderPath, GenerateGLSL(item.Code, usesCommon));
			}
			WriteFile(outPath + "/shaders/shadertoyVS.glsl", GenerateVertexShader());

			// textures
			std::vector<std::string> exportedTexs;
			for (const auto& rpass : pipeline) {
				for (const auto& inp : rpass.Inputs) {
					if (inp.Type == "texture") {
						if (std::count(exportedTexs.begin(), exportedTexs.end(), inp.Source) > 0)
							continue;

						exportedTexs.push_back(inp.Source);

						std::string texPath = outPath + inp.Source;
						if (!ghc::filesystem::exists(texPath))
							ghc::filesystem::create_directories(ghc::filesystem::path(texPath).parent_path());

						std::ofstream texFile(texPath, std::ofstream::binary);

						auto res = cli.Get(inp.Source.c_str());

						if (res && res->status == 200)
							texFile.write(res->body.c_str(), res->body.size());

						texFile.close();
					}
				}
			}
			
			return err.size() == 0;
		}

		return false;
	}


	bool Shadertoy::Init(bool isWeb, int sedVersion) {
		m_isPopupOpened = false;
		return true;
	}
	void Shadertoy::InitUI(void* ctx)
	{
		ImGui::SetCurrentContext((ImGuiContext*)ctx);
	}
	void Shadertoy::Update(float delta)
	{
		// ##### UNIFORM MANAGER POPUP #####
		if (m_isPopupOpened) {
			ImGui::OpenPopup("Import Shadertoy project##st_import");
			m_error = "";
			m_isPopupOpened = false;
		}
		ImGui::SetNextWindowSize(ImVec2(530, 160), ImGuiCond_Once);
		if (ImGui::BeginPopupModal("Import Shadertoy project##st_import")) {
			ImGui::Text("Shadertoy link:"); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			ImGui::InputText("##st_link_insert", m_link, 256);
			ImGui::PopItemWidth();

			ImGui::Text("Project path:"); ImGui::SameLine();
			ImGui::PushItemWidth(BUTTON_SPACE_LEFT);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::InputText("##pui_vspath", m_path, MY_PATH_LENGTH);
			ImGui::PopItemFlag();
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("...##pui_vsbtn", ImVec2(-1, 0))) {
				char tempPath[MY_PATH_LENGTH];
				bool success = GetOpenDirectoryDialog(tempPath);
				if (success)
					strcpy(m_path, tempPath);
			}


			if (!m_errorOccured)
				ImGui::NewLine();
			else
				ImGui::Text("[ERROR] %s", m_error.c_str()); 


			if (ImGui::Button("Ok")) {
				std::string stLink = m_link;
				std::string errMessage = "";
				if (stLink.find("www.shadertoy.com/view/") == std::string::npos)
					errMessage = "Please insert correct Shadertoy link.";

				if (errMessage.size() == 0) {
					size_t lastSlash = stLink.find_last_of('/');
					std::string id = stLink.substr(lastSlash+1);

					std::string outPath(m_path);

					if (outPath.size() == 0)
						errMessage = "Please set the output path";
					else {
						bool res = Generate(id, outPath);
						if (!res)
							errMessage = "Shader either doesn't exist or doesn't have the PublicAPI flag set";
						else
							OpenProject(Project, UI, (outPath + "/project.sprj").c_str());
					}
				}

				m_error = errMessage;
				m_errorOccured = (m_error.size() != 0);

				if (m_error.size() == 0)
					ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	bool Shadertoy::HasMenuItems(const char* name)
	{ 
		return strcmp(name, "file") == 0;
	}
	void Shadertoy::ShowMenuItems(const char* name)
	{
		if (strcmp(name, "file") == 0) {
			if (ImGui::Selectable("Import Shadertoy project")) {
				m_isPopupOpened = true;
			}
		}
	}
}