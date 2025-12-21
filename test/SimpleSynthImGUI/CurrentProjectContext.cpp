#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "ImGuiFileDialog.h"
#include "ProjectWindowSelect.h"
#include "SimpleSynthProject.h"
#include "SynthUtil.h"
#include <shared_mutex>
#include <string>
using json=nlohmann::json;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_util;
void CurrentProjectContext::setMixer(IMixer * mixer){
	this->mixer=mixer;
}
void CurrentProjectContext::newProject(){
	file="";
	objects.clear();
	dragPayloadType=nullptr;
	finalProcessor=nullptr;
	paramUIBindings.clear();
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

void CurrentProjectContext::renderCurrentProjectWindow(){
	//ImGui::Begin(LANG.getf("window.current_project.title", file).c_str());
	paramUIBindings.clear();
	// 处理和绘制选择框
	HandleSelectionAndDrag();

	// 检查右键点击的窗口
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
		ImVec2 mousePos=ImGui::GetIO().MousePos;
		rightClickedObj=nullptr;

		// 检查点击的是哪个窗口
		for(auto * obj : objects){
			if(!obj->showWindow) continue;

			if(mousePos.x >= obj->windowPos.x &&
			   mousePos.x <= obj->windowPos.x + obj->windowSize.x &&
			   mousePos.y >= obj->windowPos.y &&
			   mousePos.y <= obj->windowPos.y + obj->windowSize.y){
				rightClickedObj=obj;
				obj->isSelected=true;
				break;
			}
		}
	}

	// 渲染窗口
	auto itr=objects.iterator();
	while(itr->hasNext()){
		ProjectObject * p=itr->next();
		// 如果窗口被选中，可以绘制高亮边框
		if(p->isSelected){
			ImDrawList * drawList=ImGui::GetForegroundDrawList();
			drawList->AddRect(
				p->windowPos,
				ImVec2(p->windowPos.x + p->windowSize.x,
					   p->windowPos.y + p->windowSize.y),
				IM_COL32(255, 255, 0, 255), // 黄色边框
				0.0f, // 圆角
				0,    // 标志
				2.0f  // 线宽
			);
		}
		p->renderWindow(*this);
		if(!p->showWindow){
			itr->remove();
		}
	}
	//输出窗口
	ImGui::Begin(LANG.getc("window.notesrc_output.title"));
	static int sendToChannel=20;
	static u_sp<ParamRegister> params=mksp<ParamRegister>();
	static bool unregistered=true;
	if(unregistered){
		params->registerParam("Output", ParamType::NoteSrc, &finalProcessor, 0, 0);
		unregistered=false;
	}
	//
	bool currentType=dragPayloadType && strcmp(dragPayloadType, payloadType_NoteProcessor) == 0;
	if(currentType)ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.2f, 1.0f)); // 高亮颜色
	ImGui::Text(LANG.getc("window.notesrc_output.connect_here"));
	ImVec2 button2Min=ImGui::GetItemRectMin();
	ImVec2 button2Max=ImGui::GetItemRectMax();
	paramUIBindings.push_back(ParamUIBinding{params->RegisteredParams[0].value, ImVec2((button2Min.x + button2Max.x) * 0.5f, (button2Min.y + button2Max.y) * 0.5f)});
	if(currentType)ImGui::PopStyleColor();
	if(ImGui::BeginDragDropTarget()){
		if(const ImGuiPayload * payload=ImGui::AcceptDragDropPayload(payloadType_NoteProcessor)){
			IM_ASSERT(payload->DataSize == sizeof(ProjectObject));
			ProjectObject * pj=(ProjectObject *)(payload->Data);
			finalProcessor=std::dynamic_pointer_cast<NoteProcessor>(pj->paramRegPtr);
		}
		if(const ImGuiPayload * payload=ImGui::AcceptDragDropPayload(payloadType_Osc)){
			IM_ASSERT(payload->DataSize == sizeof(ProjectObject));
			ProjectObject * pj=(ProjectObject *)(payload->Data);
			finalProcessor=std::dynamic_pointer_cast<NoteProcessor>(pj->paramRegPtr);
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::InputInt(LANG.getc("window.notesrc_output.channel"), &sendToChannel);
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

	ImGui::End();
	//find connect lines
	std::vector<ConnectLine> connectLines;
	for(ProjectObject * obj : objects){
		if(obj->paramRegPtr == finalProcessor){
			connectLines.push_back(ConnectLine{obj->paramRegPtr.get(), params->RegisteredParams[0].value});
		}
		buildConnectLines(*obj, *obj->paramRegPtr, connectLines);
	}
	//
	ImDrawList * drawList=ImGui::GetForegroundDrawList();
	for(auto & c : connectLines){
		ImVec2 v1, v2;
		for(auto & d : paramUIBindings){
			if(d.param == c.left)v1=d.ui;
			if(d.param == c.right)v2=d.ui;
		}
		drawList->AddLine(v1, v2, IM_COL32(255, 0, 0, 255), 2.0f); // 红色线条，线宽为2.0
	}
	// 显示右键菜单
	ShowContextMenu();
}
void CurrentProjectContext::buildConnectLines(ProjectObject & obj, ParamRegister & params, std::vector<ConnectLine> & connectLines){
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
				buildConnectLines(obj, *val, connectLines);
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
						connectLines.push_back(ConnectLine{param.value, poPtr});
						break;
					}
				}
			}
			break;
		}
	}
}
void CurrentProjectContext::deleteSelected(){
	auto itr=objects.iterator();
	while(itr->hasNext()){
		ProjectObject * p=itr->next();
		if(p->isSelected){
			itr->remove();
		}
	}
}
void CurrentProjectContext::copySelected(){
	clipboardObjects.clear();
	for(auto * obj : objects){
		if(obj->isSelected){
			clipboardObjects.add(obj);
		}
	}
}
void CurrentProjectContext::pasteSelected(){
	auto j=obj2json(clipboardObjects, nullptr);
	ArrayList<ProjectObject *> n;
	json2obj(j, n);
	for(auto * a : objects){
		a->isSelected=false;
	}
	for(auto * a : n){
		a->isSelected=true;
		a->windowPos+=ImVec2(20, 20);
		objects.add(a);
	}
	n.clear();
}
void CurrentProjectContext::duplicateSelected(){
	ArrayList<ProjectObject *> n;
	for(auto * obj : objects){
		if(obj->isSelected){
			n.add(obj);
		}
	}
	auto j=obj2json(n, nullptr);
	n.clear();
	json2obj(j, n);
	for(auto * a : objects){
		a->isSelected=false;
	}
	for(auto * a : n){
		a->isSelected=true;
		a->windowPos+=ImVec2(20, 20);
		objects.add(a);
	}
	n.clear();
}
void CurrentProjectContext::saveAsSub(bool selectedOnly){
	ArrayList<ProjectObject *> n;
	if(selectedOnly){
		for(auto * obj : objects){
			if(obj->isSelected)n.add(obj);
		}
	} else{
		for(auto * obj : objects)n.add(obj);
	}
	auto j=obj2json(n, nullptr);
	n.clear();
	std::ofstream file1("/");
	file1 << j.dump(4);
}