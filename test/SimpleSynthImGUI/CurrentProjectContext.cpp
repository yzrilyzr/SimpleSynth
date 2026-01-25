#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "ImGuiFileDialog.h"
#include "ParamHelper.h"
#include "SimpleSynthProject.h"
#include "SynthUtil.h"
#include "synth/source/AmplitudeSources.h"
#include "lang/String.h"
#include "lang/System.h"
#include "imnodes.h"
#include <string>
#include <imgui_internal.h>
using json=nlohmann::json;

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_array;


void CurrentProjectContext::setMixer(IMixer * mixer){
	this->mixer=mixer;
}
void CurrentProjectContext::newProject(){
	file="";
	objects.clear();
	dragPayloadType=nullptr;
	finalProcessor=nullptr;
}
void CurrentProjectContext::openFile(const std::string & filePath){
	try{
		file=filePath;
		std::ifstream file1(filePath);
		if(!file1) return;
		json j=json::parse(file1);
		objects.clear();
		json2obj(j, objects);
		for(ProjectObject * obj : objects){
			if(obj->fromJSON.value("finalProcessor", false)){
				finalProcessor=std::static_pointer_cast<NoteProcessor>(obj->paramRegPtr);
			}
			obj->fromJSON=nullptr;
		}
	} catch(const std::exception & e){
				// 显示错误消息
		notificationManager.AddNotification(
			LANG.getf("notification.open_file.error", file, e.what()),
			3.0f,
			ImVec4(1.0f, 0.0f, 0.0f, 1.0f)  // 红色
		);
	}
}
void CurrentProjectContext::saveFile(){
	try{
		json j=json::array();
		for(const ProjectObject * obj : objects){
			json & j1=obj->to_json();
			if(finalProcessor == obj->paramRegPtr){
				j1["finalProcessor"]=true;
			}
			j.push_back(j1);
		}
		std::ofstream file1(file.c_str());
		file1 << j.dump(4);
		notificationManager.AddNotification(
			LANG.getf("notification.save_file.success", file),
			3.0f,  // 显示3秒
			ImVec4(0.0f, 1.0f, 0.0f, 1.0f)  // 绿色
		);
	} catch(const std::exception & e){
				// 显示错误消息
		notificationManager.AddNotification(
			LANG.getf("notification.save_file.error", file, e.what()),
			3.0f,
			ImVec4(1.0f, 0.0f, 0.0f, 1.0f)  // 红色
		);
	}
}
ProjectObject * CurrentProjectContext::findNode(int nodeId){
	for(auto * obj : objects){
		int objId=reinterpret_cast<int>(obj);
		if(objId == nodeId){
			return obj;
		}
	}
	return nullptr;
}

ParamReg * CurrentProjectContext::findParam(ProjectObject & obj, int attrId){
	//查找哪个被连接了
	for(ParamReg & param : obj.paramRegPtr->RegisteredParams){
		int paramId=reinterpret_cast<int>(param.value);
		if(attrId == paramId){
			return &param;
		}
	}
}

ParamReg * CurrentProjectContext::findParam(int nodeId, int attrId){
	ProjectObject * obj=findNode(nodeId);
	if(obj == nullptr)return nullptr;
	return findParam(*obj, attrId);
}

void CurrentProjectContext::renderCurrentProjectWindow(){
	ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(20000, 20000));

	ImGui::Begin(LANG.getf("window.current_project.title", file).c_str(UTF8));
	/*ImGui::SetWindowFontScale(zoom);
	bool is_window_hovered=ImGui::IsWindowHovered();
	if(is_window_hovered){
		float wheel=ImGui::GetIO().MouseWheel;
		if(wheel != 0.0f){
			float old_zoom=zoom;
			zoom*=wheel > 0?1.1:0.9;;
			zoom=ImClamp(zoom, 0.3f, 3.0f);
		}
	}*/
	// 渲染窗口
	ImNodes::BeginNodeEditor();


	for(auto * obj : objects){
		obj->renderWindow(*this);

	}
	//输出窗口

	ImNodes::BeginNode(OUTPUT_NODE_ID);
	ImNodes::SetNodeGridSpacePos(OUTPUT_NODE_ID, ImVec2(0, 0));
	ImNodes::SetNodeDraggable(OUTPUT_NODE_ID, false);
	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted(LANG.getc("window.notesrc_output.title"));
	ImNodes::EndNodeTitleBar();

	static int sendToChannel=20;

	auto color=IM_COL32(100, 220, 100, 255);
	ImNodes::PushColorStyle(ImNodesCol_Pin, color);

	ImNodes::BeginInputAttribute(OUTPUT_ATTR_ID, ImNodesPinShape_TriangleFilled);
	ImGui::Text(LANG.getc("window.notesrc_output.connect_here"));
	ImNodes::EndInputAttribute();

	ImNodes::PopColorStyle();

	ImGui::PushItemWidth(100.0f);
	ImGui::InputInt(LANG.getc("window.notesrc_output.channel"), &sendToChannel);
	ImGui::PopItemWidth();

	if((ImGui::Button(LANG.getc("window.notesrc_output.set")) || paramChange) && finalProcessor != nullptr){
		const char * group="WM_MIDI_Instant";
		try{
			ChannelConfig & cfg=mixer->getMIDIChannel(group, sendToChannel)->getConfig();
			finalProcessor->init(cfg);
			ProgramChange * event=new ProgramChange();
			event->channelID=sendToChannel;
			event->groupName=group;
			event->noteProcessor=finalProcessor;
			mixer->sendInstantEvent(event);
		} catch(...){
			ProgramChange * event=new ProgramChange();
			event->channelID=sendToChannel;
			event->groupName=group;
			event->noteProcessor=SynthUtil::getDefault();
			mixer->sendInstantEvent(event);
		}
		paramChange=false;
	}

	ImNodes::EndNode();

	//find connect lines
	links.clear();
	for(ProjectObject * obj : objects){
		int nodeId=reinterpret_cast<int>(obj);
		auto oldWindowPos=ImNodes::GetNodeGridSpacePos(nodeId);
		if(oldWindowPos.x == 0 && oldWindowPos.y == 0){
			ImNodes::SetNodeGridSpacePos(nodeId, obj->windowPos);
		} else{
			obj->windowPos=oldWindowPos;
		}		

		if(obj->paramRegPtr == finalProcessor){
			int started_at_attribute_id=reinterpret_cast<int>(obj->paramRegPtr.get());
			links.push_back(LinkLine{nodeId, started_at_attribute_id, OUTPUT_NODE_ID, OUTPUT_ATTR_ID, color});
		}
		buildLinks(*obj, *obj->paramRegPtr);
	}

	for(size_t i=0; i < links.size(); ++i){
		auto & link=links[i];
		ImNodes::PushColorStyle(ImNodesCol_Link, link.color);
		ImNodes::Link(i, link.started_at_attribute_id, link.ended_at_attribute_id);
		ImNodes::PopColorStyle();
	}
	ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
	ImNodes::EndNodeEditor();

	int  started_at_attribute_id=0;
	int  ended_at_attribute_id=0;
	int  started_at_node_id=0;
	int  ended_at_node_id=0;
	bool created_from_snap=0;
	int destroyed_link_id=0;
	if(ImNodes::IsLinkDestroyed(&destroyed_link_id)){
		LinkLine & link=links[destroyed_link_id];
		System::out.println(String("LinkDestroyed: ") + link.started_at_node_id + " " + link.started_at_attribute_id + " " + link.ended_at_node_id + " " + link.ended_at_attribute_id);

	}
	if(ImNodes::IsLinkCreated(&started_at_node_id,
							  &started_at_attribute_id,
							  &ended_at_node_id,
							  &ended_at_attribute_id,
							  &created_from_snap)){
		if(ended_at_node_id == OUTPUT_NODE_ID){
			System::out.println(String("LinkOutput: ") + started_at_node_id + " " + started_at_attribute_id + " " + ended_at_node_id + " " + ended_at_attribute_id);
			//查找输出
			ProjectObject * nodeStart=findNode(started_at_node_id);
			if(nodeStart){
				if(auto ptr=spdc<NoteProcessor>(nodeStart->paramRegPtr)){
					//创建新有效连接
					finalProcessor=ptr;
				} else{
					finalProcessor=nullptr;
				}
				paramChange=true;
			}
		} else{
			ProjectObject * nodeStart=nullptr;
			ProjectObject * nodeEnd=nullptr;
			for(auto * obj : objects){
				int objId=reinterpret_cast<int>(obj);
				if(objId == started_at_node_id)nodeStart=obj;
				if(objId == ended_at_node_id)nodeEnd=obj;
			}
			if(nodeStart && nodeEnd){
				System::out.println(String("Link: ") + started_at_node_id + " " + started_at_attribute_id + " " + ended_at_node_id + " " + ended_at_attribute_id);
				ParamReg * endParam=findParam(*nodeEnd, ended_at_attribute_id);
				if(endParam){
					paramChange=setParamValue(*endParam, nodeStart->paramRegPtr) || paramChange;
				}
			}
		}
	}
	// 显示右键菜单
	ShowContextMenu();
	ImGui::End();
	//ImGui::SetWindowFontScale(1.0);
}
void CurrentProjectContext::buildLinks(ProjectObject & obj, ParamRegister & params){
	for(auto & param : params.RegisteredParams){
		switch(param.type){
			case ParamType::Float:
			case ParamType::Double:
			case ParamType::Int:
			case ParamType::UInt:
			case ParamType::Long:
			case ParamType::ULong:
			case ParamType::Bool:
			case ParamType::Size:
			case ParamType::Freq:
			case ParamType::Time:
			case ParamType::TimeMs:
				break;
			case ParamType::Sub:
			{
				ParamRegister * val=static_cast<ParamRegister *>(param.value);
				buildLinks(obj, *val);
			}
			break;
			default:
			{
				u_sp<void> * paramRegPtr=reinterpret_cast<u_sp<void>*>(param.value);
				void * value=paramRegPtr->get();
				for(ProjectObject * obj2 : objects){
					if(obj2 == &obj)continue;
					void * poPtr=obj2->paramRegPtr.get();
					if(value == poPtr){
						int started_at_node_id=reinterpret_cast<int>(obj2);
						int started_at_attribute_id=reinterpret_cast<int>(obj2->paramRegPtr.get());
						int ended_at_node_id=reinterpret_cast<int>(&obj);
						int ended_at_attribute_id=reinterpret_cast<int>(param.value);
						links.push_back(LinkLine{started_at_node_id, started_at_attribute_id, ended_at_node_id, ended_at_attribute_id, getPinColor(param)});
						break;
					}
				}
			}
			break;
		}
	}
}
