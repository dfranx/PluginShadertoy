#pragma once
#include <PluginAPI/Plugin.h>
#include <vector>
#include <string>

#define MY_PATH_LENGTH 512 // TODO: use MAX_PATH or sth

namespace st
{
	class Shadertoy : public ed::IPlugin
	{
	public:
		virtual bool Init();
		virtual void OnEvent(void* e);
		virtual void Update(float delta);
		virtual void Destroy();

		virtual void BeginRender();
		virtual void EndRender();

		virtual void BeginProjectLoading();
		virtual void EndProjectLoading();
		virtual void BeginProjectSaving();
		virtual void EndProjectSaving();
		virtual void CopyFilesOnSave(const char* dir);
		virtual bool HasCustomMenu();

		virtual bool HasMenuItems(const char* name);
		virtual void ShowMenuItems(const char* name);

		virtual bool HasContextItems(const char* name);
		virtual void ShowContextItems(const char* name, void* owner = nullptr, void* extraData = nullptr);

		// system variables
		virtual bool HasSystemVariables(ed::plugin::VariableType varType);
		virtual int GetSystemVariableNameCount(ed::plugin::VariableType varType);
		virtual const char* GetSystemVariableName(ed::plugin::VariableType varType, int index);
		virtual bool HasLastFrame(char* name, ed::plugin::VariableType varType);
		virtual void UpdateSystemVariableValue(char* data, char* name, ed::plugin::VariableType varType, bool isLastFrame);

		// functions
		virtual bool HasVariableFunctions(ed::plugin::VariableType vtype);
		virtual int GetVariableFunctionNameCount(ed::plugin::VariableType vtype);
		virtual const char* GetVariableFunctionName(ed::plugin::VariableType varType, int index);
		virtual bool ShowFunctionArgumentEdit(char* fname, char* args, ed::plugin::VariableType vtype);
		virtual void UpdateVariableFunctionValue(char* data, char* args, char* fname, ed::plugin::VariableType varType);
		virtual int GetVariableFunctionArgSpaceSize(char* fname, ed::plugin::VariableType varType);
		virtual void InitVariableFunctionArguments(char* args, char* fname, ed::plugin::VariableType vtype);
		virtual const char* ExportFunctionArguments(char* fname, ed::plugin::VariableType vtype, char* args);
		virtual void ImportFunctionArguments(char* fname, ed::plugin::VariableType vtype, char* args, const char* argsString);

		// object manager stuff
		virtual bool HasObjectPreview(const char* type);
		virtual void ShowObjectPreview(const char* type, void* data, unsigned int id);
		virtual bool IsObjectBindable(const char* type);
		virtual bool IsObjectBindableUAV(const char* type);
		virtual void RemoveObject(const char* name, const char* type, void* data, unsigned int id);
		virtual bool HasObjectExtendedPreview(const char* type);
		virtual void ShowObjectExtendedPreview(const char* type, void* data, unsigned int id);
		virtual bool HasObjectProperties(const char* type);
		virtual void ShowObjectProperties(const char* type, void* data, unsigned int id);
		virtual void BindObject(const char* type, void* data, unsigned int id);
		virtual const char* ExportObject(char* type, void* data, unsigned int id);
		virtual void ImportObject(const char* name, const char* type, const char* argsString);
		virtual bool HasObjectContext(const char* type);
		virtual void ShowObjectContext(const char* type, void* data);

		// pipeline item stuff
		virtual bool HasPipelineItemProperties(const char* type);
		virtual void ShowPipelineItemProperties(const char* type, void* data);
		virtual bool IsPipelineItemPickable(const char* type);
		virtual bool HasPipelineItemShaders(const char* type);
		virtual void OpenPipelineItemInEditor(void* CodeEditor, const char* type, void* data);
		virtual bool CanPipelineItemHaveChild(const char* type, ed::plugin::PipelineItemType itemType);
		virtual int GetPipelineItemInputLayoutSize(const char* itemName);
		virtual void GetPipelineItemInputLayoutItem(const char* itemName, int index, ed::plugin::InputLayoutItem& out);
		virtual void RemovePipelineItem(const char* itemName, const char* type, void* data);
		virtual void RenamePipelineItem(const char* oldName, const char* newName);
		virtual void AddPipelineItemChild(const char* owner, const char* name, ed::plugin::PipelineItemType type, void* data);
		virtual bool CanPipelineItemHaveChildren(const char* type);
		virtual void* CopyPipelineItemData(const char* type, void* data);
		virtual void ExecutePipelineItem(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data);
		virtual void ExecutePipelineItem(const char* type, void* data, void* children, int count);
		virtual void GetPipelineItemWorldMatrix(const char* name, float(&pMat)[16]);
		virtual bool IntersectPipelineItem(const char* type, void* data, const float* rayOrigin, const float* rayDir, float& hitDist);
		virtual void GetPipelineItemBoundingBox(const char* name, float(&minPos)[3], float(&maxPos)[3]);
		virtual bool HasPipelineItemContext(const char* type);
		virtual void ShowPipelineItemContext(const char* type, void* data);
		virtual const char* ExportPipelineItem(const char* type, void* data);
		virtual void* ImportPipelineItem(const char* ownerName, const char* name, const char* type, const char* argsString);
		virtual void MovePipelineItemDown(void* ownerData, const char* ownerType, const char* itemName);
		virtual void MovePipelineItemUp(void* ownerData, const char* ownerType, const char* itemName);

		// options
		virtual bool HasSectionInOptions();
		virtual void ShowOptions();

		// code editor
		virtual void SaveCodeEditorItem(const char* src, int srcLen, int sid);
		virtual void CloseCodeEditorItem(int sid);
		virtual int GetLanguageDefinitionKeywordCount(int sid);
		virtual const char** GetLanguageDefinitionKeywords(int sid);
		virtual int GetLanguageDefinitionTokenRegexCount(int sid);
		virtual const char* GetLanguageDefinitionTokenRegex(int index, ed::plugin::TextEditorPaletteIndex& palIndex, int sid);
		virtual int GetLanguageDefinitionIdentifierCount(int sid);
		virtual const char* GetLanguageDefinitionIdentifier(int index, int sid);
		virtual const char* GetLanguageDefinitionIdentifierDesc(int index, int sid);
		virtual const char* GetLanguageDefinitionCommentStart(int sid);
		virtual const char* GetLanguageDefinitionCommentEnd(int sid);
		virtual const char* GetLanguageDefinitionLineComment(int sid);
		virtual bool IsLanguageDefinitionCaseSensitive(int sid);
		virtual bool GetLanguageDefinitionAutoIndent(int sid);
		virtual const char* GetLanguageDefinitionName(int sid);
		virtual const char* GetLanguageAbbreviation(int id);

		// misc
		virtual bool HandleDropFile(const char* filename);
		virtual void HandleRecompile(const char* itemName);
		virtual void HandleRecompileFromSource(const char* itemName, int sid, const char* shaderCode, int shaderSize);
		virtual int GetShaderFilePathCount();
		virtual const char* GetShaderFilePath(int index);
		virtual bool HasShaderFilePathChanged();
		virtual void UpdateShaderFilePath();

	private:
		std::string m_error;
		char m_link[256], m_path[MY_PATH_LENGTH];
		bool m_isPopupOpened;
	};
}