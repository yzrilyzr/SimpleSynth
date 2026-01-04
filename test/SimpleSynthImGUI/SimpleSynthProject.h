#pragma once
#include "MenuRegister.hpp"
#include "NotificationManager.h"
#include "ProjectObject.h"
#include "collection/ArrayList.hpp"
#include "imgui.h"
#include "interface/IMixer.h"
#include "nlohmann/json.hpp"
#include "util/Lang.h"
#include "util/ParamRegister.h"
#include "yzrutil.h"
#include <string>
#include <vector>

#define OUTPUT_NODE_ID 44948
#define OUTPUT_ATTR_ID 18447

using json=nlohmann::json;

struct LinkLine{
	int started_at_node_id;
	int started_at_attribute_id;
	int ended_at_node_id;
	int ended_at_attribute_id;
	ImU32 color;
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
	std::vector<LinkLine> links;
	float zoom=1.0f;
	std::string file="";
	bool paramChange=false;
	NotificationManager notificationManager;
	ProjectObject * rightClickedObj=nullptr;
	yzrilyzr_simplesynth::IMixer * mixer;
	u_time_f processTime=0;
	void setMixer(yzrilyzr_simplesynth::IMixer * mixer);
	void openFile(const std::string & filePath);
	void newProject();
	void saveFile();
	void HandleShortcuts();
	void ShowContextMenu();
	void renderCurrentProjectWindow();
	void buildLinks(ProjectObject & obj, yzrilyzr_util::ParamRegister & params);
	ProjectObject * findNode(int nodeId);
	yzrilyzr_util::ParamReg * findParam(ProjectObject & obj, int attrId);
	yzrilyzr_util::ParamReg * findParam(int nodeId, int attrId);

	void autoLayout();
	void deleteSelected();
	void copySelected();
	void pasteSelected();
	void duplicateSelected();
	void saveAsSub(bool selectedOnly);
};

void name2obj(const yzrilyzr_lang::String & category, const yzrilyzr_lang::String & name, yzrilyzr_lang::String * name1, yzrilyzr_lang::String * category1, yzrilyzr_lang::String * showName, u_sp<yzrilyzr_util::ParamRegister> * obj, MenuRegister::RenderFunc * rfunc, bool * enableOriginalRender);
json obj2json(yzrilyzr_collection::ArrayList<ProjectObject *> & arr, yzrilyzr_simplesynth::NoteProcPtr finalProcessor);
void json2obj(json & j, yzrilyzr_collection::ArrayList<ProjectObject *> & arr);