#include "ImGuiFileDialog.h"
#include "ParamHelper.h"
#include "SimpleSynthProject.h"
#include "array/Array.hpp"
#include "imgui.h"
#include "imnodes.h"
#include <string>

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;

void CurrentProjectContext::ShowContextMenu(){
	// 检查是否点击右键
	const int num_node_selected=ImNodes::NumSelectedNodes();
	const int num_link_selected=ImNodes::NumSelectedLinks();
	bool hasNodeSelection=num_node_selected > 0;
	bool hasLinkSelection=num_link_selected > 0;
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
		if(hasNodeSelection)ImGui::OpenPopup("NodeContextMenu");
		else if(hasLinkSelection)ImGui::OpenPopup("LinkContextMenu");
		else ImGui::OpenPopup("ContextMenu");
	}

	// 右键菜单
	if(ImGui::BeginPopup("NodeContextMenu")){
		IntArray arr(num_node_selected);
		ImNodes::GetSelectedNodes(arr._array);
		for(auto * obj : objects){
			int nodeId=reinterpret_cast<int>(obj);
			obj->isSelected=false;
			for(int node_id:arr){
				if(nodeId == node_id){
					obj->isSelected=true;
					break;
				}
			}
		}

		if(ImGui::MenuItem(LANG.getc("right_context_menu.copy"), "Ctrl+C", false, hasNodeSelection)){
			copySelected();
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.paste"), "Ctrl+V", false, !clipboardObjects.isEmpty())){
			pasteSelected();
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.duplicate"), "Ctrl+D", false, hasNodeSelection)){
			duplicateSelected();
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.delete"), "Del", false, hasNodeSelection)){
			deleteSelected();
		}
		ImGui::Separator();
		if(ImGui::MenuItem(LANG.getc("right_context_menu.save_as_sub"), "Ctrl+B", false, hasNodeSelection)){
			saveAsSub(true);
		}
		ImGui::EndPopup();
	}

	if(ImGui::BeginPopup("LinkContextMenu")){
		if(ImGui::MenuItem(LANG.getc("right_context_menu.delete_link"), "Del")){
			IntArray arr(num_link_selected);
			ImNodes::GetSelectedLinks(arr._array);
			for(int selectLinkId:arr){
				auto & link=links[selectLinkId];
				if(link.ended_at_attribute_id == OUTPUT_ATTR_ID && link.ended_at_node_id == OUTPUT_NODE_ID){
					finalProcessor=nullptr;
				} else{
					ParamReg * endParam=findParam(link.ended_at_node_id, link.ended_at_attribute_id);
					if(endParam){
						setParamValue(*endParam, nullptr);
					}
				}
				paramChange=true;
			}
		}
		ImGui::EndPopup();
	}
	if(ImGui::BeginPopup("ContextMenu")){
		if(ImGui::MenuItem(LANG.getc("right_context_menu.auto_layout"), "Ctrl+L")){
			autoLayout();
		}
		ImGui::EndPopup();
	}
}

struct NodeBox{
	ProjectObject * obj;
	float x;
	float y;
	float w;
	float h;

};

void  CurrentProjectContext::autoLayout(){
	// 力学系统参数
	const float REPULSION=15000;   // 节点间排斥力强度
	const float ATTRACTION=0.005;  // 边的吸引力强度
	const float DAMPING=0.5;       // 系统阻尼系数(控制稳定性)
	//初始化盒子
	std::vector<NodeBox> nodeBoxes;
	for(ProjectObject * obj : objects){
		int nodeId=reinterpret_cast<int>(obj);
		ImVec2 pos=ImNodes::GetNodeGridSpacePos(nodeId);
		ImVec2 dimen=ImNodes::GetNodeDimensions(nodeId);
		nodeBoxes.push_back({obj, pos.x, pos.y, dimen.x, dimen.y});
	}
	//布局代码
	//...
	//应用
	for(NodeBox & box : nodeBoxes){
		ProjectObject * obj=box.obj;
		obj->windowPos=ImVec2(box.x, box.y);
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