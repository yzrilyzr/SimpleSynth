#include "ImGuiFileDialog.h"
#include "ParamHelper.h"
#include "SimpleSynthProject.h"
#include "array/Array.hpp"
#include "imgui.h"
#include "imnodes.h"
#include "NodeAutoLayout.h"
#include <string>

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;

NodeAutoLayout autoLayout;

void CurrentProjectContext::deleteSelectedLinks(){
	const int num_link_selected=ImNodes::NumSelectedLinks();
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
void CurrentProjectContext::computeSelectedNodes(){
	const int num_node_selected=ImNodes::NumSelectedNodes();
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
}
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
		computeSelectedNodes();

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
		if(ImGui::MenuItem(LANG.getc("right_context_menu.lock_layout"), "", false, hasNodeSelection)){
			lockLayoutSelected(true);
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.unlock_layout"), "", false, hasNodeSelection)){
			lockLayoutSelected(false);
		}
		ImGui::Separator();
		if(ImGui::MenuItem(LANG.getc("right_context_menu.save_as_sub"), "Ctrl+B", false, hasNodeSelection)){
			saveAsSub(true);
		}
		ImGui::EndPopup();
	}

	if(ImGui::BeginPopup("LinkContextMenu")){
		if(ImGui::MenuItem(LANG.getc("right_context_menu.delete_link"), "Del")){
			deleteSelectedLinks();
		}
		ImGui::EndPopup();
	}
	if(ImGui::BeginPopup("ContextMenu")){
		if(ImGui::MenuItem(LANG.getc("right_context_menu.auto_layout"), "Ctrl+K")){
			autoLayout.start(this, NodeAutoLayout::SPRING);
		} else if(ImGui::MenuItem(LANG.getc("right_context_menu.auto_layout_repulsion"), "Ctrl+L")){
			autoLayout.start(this, NodeAutoLayout::SPRING_REPULSION);
		}
		ImGui::EndPopup();
	}
	if(!autoLayout.finished()){
		autoLayout.doLayout();
	}
	//
	if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_K)){
		autoLayout.start(this, NodeAutoLayout::SPRING);
	}
	if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_L)){
		autoLayout.start(this, NodeAutoLayout::SPRING_REPULSION);
	}
	if(ImGui::IsKeyPressed(ImGuiKey_Delete)){
		if(hasLinkSelection){
			deleteSelectedLinks();
		}
		if(hasNodeSelection){
			computeSelectedNodes();
			deleteSelected();
		}
	}
	if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_A)){
		computeSelectedNodes();
		for(auto * obj : objects){
			if(!obj->isSelected){
				int nodeId=reinterpret_cast<int>(obj);
				ImNodes::SelectNode(nodeId);
				obj->isSelected=true;
			}
		}
	}
	if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S)){
		saveFile();
	}
	if(hasNodeSelection){
		if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D)){
			System::out.println("aa");
			computeSelectedNodes();
			duplicateSelected();
		}
		if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C)){
			computeSelectedNodes();
			copySelected();
		}
		if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V)){
			computeSelectedNodes();
			pasteSelected();
		}
		if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_B)){
			computeSelectedNodes();
			saveAsSub(true);
		}
	}
}

void CurrentProjectContext::lockLayoutSelected(bool lock){
	for(auto * p : objects){
		if(p->isSelected){
			p->lockLayout=lock;
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
	for(auto * p : objects){
		if(p->isSelected){
			clipboardObjects.add(p);
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
		a->windowPos+=ImVec2(20, 20);
		a->isSelected=true;
		a->lockLayout=false;
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
		a->lockLayout=false;
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