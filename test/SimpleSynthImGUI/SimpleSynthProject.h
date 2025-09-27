#pragma once
#include "MenuRegister.hpp"
#include "NotificationManager.h"
#include "ProjectObject.h"
#include "collection/ArrayList.hpp"
#include "imgui.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "nlohmann/json.hpp"
#include "util/Lang.h"
#include "util/ParamRegister.h"
#include "yzrutil.h"
#include <memory>
#include <string>
#include <vector>
using json=nlohmann::json;

struct ParamUIBinding{
	void * param;
	ImVec2 ui;
};
struct ConnectLine{
	void * left;
	void * right;
};

struct SelectionBox{
	ImVec2 start;         // 开始点
	ImVec2 current;       // 当前点
	bool isSelecting;     // 是否正在选择
	bool isLeftToRight;   // 是否从左上到右下
	bool isDragging;        // 是否正在拖动
	ImVec2 dragStartPos;    // 拖动开始时的鼠标位置
};

class CurrentProjectContext{
	public:
	static constexpr const char * const payloadType_NoteProcessor="NoteProcessor";
	static constexpr const char * const payloadType_Osc="Osc";
	static constexpr const char * const payloadType_PhaseSrc="PhaseSrc";
	static constexpr const char * const payloadType_Interpolator="Interpolator";
	static constexpr const char * const payloadType_DSP="DSP";
	static constexpr const char * const payloadType_Sample="SampleData";
	u_sample_rate sampleRate;
	yzrilyzr_util::Lang LANG;
	const char * dragPayloadType=nullptr;
	yzrilyzr_simplesynth::NoteProcPtr finalProcessor=nullptr;
	yzrilyzr_collection::ArrayList<ProjectObject *> objects;
	yzrilyzr_collection::ArrayList<ProjectObject *> clipboardObjects;
	std::vector<ParamUIBinding> paramUIBindings;
	std::string file="";
	bool paramChange=false;
	NotificationManager notificationManager;
	ProjectObject * rightClickedObj=nullptr;
	SelectionBox selection;
	yzrilyzr_simplesynth::IMixer * mixer;
	u_time_f processTime=0;
	void setMixer(yzrilyzr_simplesynth::IMixer * mixer);
	void openFile(const std::string & filePath);
	void newProject();
	void saveFile();
	void HandleShortcuts();
	void ShowContextMenu();
	void renderCurrentProjectWindow();
	void buildConnectLines(ProjectObject & obj, yzrilyzr_util::ParamRegister & params, std::vector<ConnectLine> & connectLine);
	void HandleSelectionAndDrag();

	void deleteSelected();
	void copySelected();
	void pasteSelected();
	void duplicateSelected();
	void saveAsSub(bool selectedOnly);
};

void name2obj(const std::string & category, const std::string & name, std::string * name1, std::string * category1, std::string * showName, std::shared_ptr<yzrilyzr_util::ParamRegister> * obj, MenuRegister::RenderFunc * rfunc, bool * enableOriginalRender);
json obj2json(yzrilyzr_collection::ArrayList<ProjectObject *> & arr, yzrilyzr_simplesynth::NoteProcPtr finalProcessor);
void json2obj(json & j, yzrilyzr_collection::ArrayList<ProjectObject *> & arr);