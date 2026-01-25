#include "ParamHelper.h"
#include "ProjectObject.h"
#include "SimpleSynthProject.h"
#include "imnodes.h"
#include "lang/Boxing.h"
#include "synth/source/AmplitudeSources.h"
#include <string>
using json=nlohmann::json;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;

void ProjectObject::paramToJSON(json & a, const ParamRegister & paramReg){
	for(auto & p : paramReg.RegisteredParams){
		//a[(p->name + "type").c_str()]=p->type;
		switch(p.type){
			case ParamType::Float:
				a[p.name.c_str()]=*static_cast<float *>(p.value);
				break;
			case ParamType::Double:
				a[p.name.c_str()]=*static_cast<double *>(p.value);
				break;
			case ParamType::Int:
				a[p.name.c_str()]=*static_cast<int32_t *>(p.value);
				break;
			case ParamType::UInt:
				a[p.name.c_str()]=*static_cast<uint32_t *>(p.value);
				break;
			case ParamType::Long:
				a[p.name.c_str()]=*static_cast<int64_t *>(p.value);
				break;
			case ParamType::ULong:
				a[p.name.c_str()]=*static_cast<uint64_t *>(p.value);
				break;
			case ParamType::Bool:
				a[p.name.c_str()]=*static_cast<bool *>(p.value);
				break;
			case ParamType::Size:
				a[p.name.c_str()]=*static_cast<u_index *>(p.value);
				break;
			case ParamType::Enum:
				a[p.name.c_str()]=*static_cast<int *>(p.value);
				break;
			case ParamType::Freq:
				a[p.name.c_str()]=*static_cast<u_freq *>(p.value);
				break;
			case ParamType::Time:
				a[p.name.c_str()]=*static_cast<u_time *>(p.value);
				break;
			case ParamType::TimeMs:
				a[p.name.c_str()]=*static_cast<u_time_ms *>(p.value);
				break;
			case ParamType::Sample:
				a[p.name.c_str()]=*static_cast<u_sample *>(p.value);
				break;
			case ParamType::Gain:
				a[p.name.c_str()]=*static_cast<u_sample *>(p.value);
				break;
			case ParamType::Sub:
			{
				json b=json::object();
				paramToJSON(b, *static_cast<ParamRegister *>(p.value));
				a[p.name.c_str()]=b;
			}
			break;
			default:
				u_sp<void> * paramRegPtr=reinterpret_cast<u_sp<void>*>(p.value);
				void * rawPtr=paramRegPtr->get();
				a[p.name.c_str()]=reinterpret_cast<uint64_t>(rawPtr);
				break;
		}
	}
}

json ProjectObject::to_json() const{
	json a={
		{"name", name.tostring()},
		{"category", category.tostring()},
		{"id", reinterpret_cast<uint64_t>(paramRegPtr.get())},
		{"showWindow", showWindow},
		{"windowPosX", windowPos.x},
		{"windowPosY", windowPos.y},
		{"windowSizeX", windowSize.x},
		{"windowSizeY", windowSize.y}
	};
	paramToJSON(a, *paramRegPtr);
	json storedDataJSON;
	for(auto & p : storeData){
		auto datap=p.second;
		if(auto datap2=spdc<SampleArray>(datap)){
			json jsonArray=json::array();
			for(u_index i=0;i < datap2->length;i++){
				jsonArray.push_back((*datap2)[i]);
			}
			storedDataJSON[p.first.c_str()]={{"data", jsonArray}, {"type", "SampleArray"}};
		} else if(auto datap2=spdc<DoubleArray>(datap)){
			json jsonArray=json::array();
			for(u_index i=0;i < datap2->length;i++){
				jsonArray.push_back((*datap2)[i]);
			}
			storedDataJSON[p.first.c_str()]={{"data", jsonArray}, {"type", "DoubleArray"}};
		} else if(auto datap2=spdc<Short>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		} else if(auto datap2=spdc<Integer>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		} else if(auto datap2=spdc<Long>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		} else if(auto datap2=spdc<Float>(datap)){
			a[p.first.c_str()]=datap2->value;
		} else if(auto datap2=spdc<Double>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		}
	}
	a["StoredData"]=storedDataJSON;
	return a;
}
void ProjectObject::JSONToParam(const json & j, ParamRegister & paramReg){
	for(auto & param : paramReg.RegisteredParams){
		const char * key=param.name.c_str();
		if(!j.contains(key))continue;
		switch(param.type){
			case ParamType::Float:
				*static_cast<float *>(param.value)=j.value(key, 0.0f);
				break;
			case ParamType::Double:
				*static_cast<double *>(param.value)=j.value(key, 0.0);
				break;
			case ParamType::Int:
				*static_cast<int32_t *>(param.value)=j.value(key, static_cast<int32_t>(0));
				break;
			case ParamType::UInt:
				*static_cast<uint32_t *>(param.value)=j.value(key, static_cast<uint32_t>(0));
				break;
			case ParamType::Long:
				*static_cast<int64_t *>(param.value)=j.value(key, static_cast<int64_t>(0));
				break;
			case ParamType::ULong:
				*static_cast<uint64_t *>(param.value)=j.value(key, static_cast<uint64_t>(0));
				break;
			case ParamType::Bool:
				*static_cast<bool *>(param.value)=j.value(key, false);
				break;
			case ParamType::Size:
				*static_cast<u_index *>(param.value)=j.value(key, static_cast<u_index>(0));
				break;
			case ParamType::Enum:
				*static_cast<int *>(param.value)=j.value(key, static_cast<int>(0));
				break;
			case ParamType::Freq:
				*static_cast<u_freq *>(param.value)=j.value(key, static_cast<u_freq>(0));
				break;
			case ParamType::Time:
				*static_cast<u_time *>(param.value)=j.value(key, static_cast<u_time>(0));
				break;
			case ParamType::TimeMs:
				*static_cast<u_time_ms *>(param.value)=j.value(key, static_cast<u_time_ms>(0));
				break;
			case ParamType::Sample:
				*static_cast<u_sample *>(param.value)=j.value(key, static_cast<u_sample>(0));
				break;
			case ParamType::Gain:
				*static_cast<u_sample *>(param.value)=j.value(key, static_cast<u_sample>(0));
				break;
			case ParamType::Sub:
				JSONToParam(j[key], *static_cast<ParamRegister *>(param.value));
				break;
		}
	}
}
void ProjectObject::from_json(const json & j){
	name=std::string(j["name"]);
	category=std::string(j["category"]);
	showWindow=j.value("showWindow", true);
	windowPos.x=j.value("windowPosX", 100.0f);
	windowPos.y=j.value("windowPosY", 100.0f);
	windowSize.x=j.value("windowSizeX", 300.0f);
	windowSize.y=j.value("windowSizeY", 200.0f);
	name2obj(category, name, &name, &category, &showName, &paramRegPtr, &renderFunc, &enableOriginalRender);
	if(paramRegPtr == nullptr){
		throw Exception(String("Object not found: [category: ") + category + ", name: " + name + "]");
	}
	JSONToParam(j, *paramRegPtr);
	if(j.find("StoredData") != j.end()){
		auto & storedDataJSON=j["StoredData"];
		if(!storedDataJSON.is_null()){
			for(auto & item : storedDataJSON.items()){
				auto key=item.key();
				auto & value=item.value();
				if(value.is_object()){
					auto data=value["data"];
					auto type=value["type"];
					if(type == "SampleArray"){
						auto arrPtr=mksp<SampleArray>(data.size());
						for(u_index i=0;i < data.size();i++){
							(*arrPtr)[i]=data[i].get<u_sample>();
						}
						storeData[key]=arrPtr;
					} else if(type == "DoubleArray"){
						auto arrPtr=mksp<DoubleArray>(data.size());
						for(u_index i=0;i < data.size();i++){
							(*arrPtr)[i]=data[i].get<double>();
						}
						storeData[key]=arrPtr;
					}
				} else if(value.is_number_integer()){
					auto intPtr=mksp<Integer>(value.get<int32_t>());
					storeData[key]=intPtr;
				} else if(value.is_number_unsigned()){
					auto longPtr=mksp<Long>(value.get<int64_t>());
					storeData[key]=longPtr;
				} else if(value.is_number_float()){
					auto doublePtr=mksp<Double>(value.get<double>());
					storeData[key]=doublePtr;
				}
			}
		}
	}
	fromJSON=j;
	loadStoredData=true;
}

void ProjectObject::renderWindow(CurrentProjectContext & ctx){
	int paramId=reinterpret_cast<int>(paramRegPtr.get());
	int nodeId=reinterpret_cast<int>(this);
	auto color=getPinColor(paramRegPtr);
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, color);
	ImNodes::BeginNode(nodeId);
	
	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted(showName.c_str(UTF8));
	ImNodes::EndNodeTitleBar();

	//输出节点
	ImNodes::PushColorStyle(ImNodesCol_Pin, color);

	ImNodes::BeginOutputAttribute(paramId, ImNodesPinShape_TriangleFilled);
	ImGui::Text(ctx.LANG.getc("window.project_object.output"));
	ImNodes::EndOutputAttribute();

	ImNodes::PopColorStyle();

	if(paramRegPtr->RegisteredParams.empty()){
		ImNodes::BeginStaticAttribute(paramId);
		ImNodes::EndStaticAttribute();
	} else{
		//ImGui::Text(ctx.LANG.getc("window.project_object.params"));
		if(enableOriginalRender){
			ctx.paramChange=renderParams(ctx, paramRegPtr->RegisteredParams) || ctx.paramChange;
		}
		if(renderFunc != nullptr)renderFunc(ctx, *this);
	}
	ImNodes::EndNode();
	ImNodes::PopColorStyle();
}

bool ProjectObject::renderParams(CurrentProjectContext & ctx, std::vector<ParamReg> & paramReg){
	bool change=false;
	for(auto & param : paramReg){
		String guiName=param.name;
		if(param.aliasName != ""){
			guiName=param.aliasName + "(" + guiName + ")";
		}
		const char * cstrName=guiName.c_str(UTF8);
		bool localChange=false;
		switch(param.type){
			case ParamType::Bool:
				localChange=ImGui::Checkbox(cstrName, static_cast<bool *>(param.value));
				break;
			case ParamType::Double:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::Float:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Float, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::Freq:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f Hz", ImGuiSliderFlags_Logarithmic);
				ImGui::PopItemWidth();
				break;
			case ParamType::Time:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f s", ImGuiSliderFlags_Logarithmic);
				ImGui::PopItemWidth();
				break;
			case ParamType::TimeMs:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f ms", ImGuiSliderFlags_Logarithmic);
				ImGui::PopItemWidth();
				break;
			case ParamType::Int:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_S32, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::UInt:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_U32, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::Long:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_S64, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::ULong:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_U64, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::Size:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, sizeof(u_index) == 8?ImGuiDataType_U64:ImGuiDataType_U32, param.value, param.valueMin, param.valueMax);
				ImGui::PopItemWidth();
				break;
			case ParamType::Gain:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, sizeof(u_sample) == 8?ImGuiDataType_Double:ImGuiDataType_Float, param.value, param.valueMin, param.valueMax, "%.3f mul", ImGuiSliderFlags_Logarithmic);
				ImGui::PopItemWidth();
				break;
			case ParamType::Sample:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::SliderScalar(cstrName, sizeof(u_sample) == 8?ImGuiDataType_Double:ImGuiDataType_Float, param.value, param.valueMin, param.valueMax, "%.3f");
				ImGui::PopItemWidth();
				break;
			case ParamType::Enum:
				ImGui::PushItemWidth(150.0f);
				localChange=ImGui::Combo(cstrName, static_cast<int *>(param.value), static_cast<const char **>(param.valueMin), *static_cast<int *>(param.valueMax));
				ImGui::PopItemWidth();
				break;
			case ParamType::Sub:
				ImGui::Text(cstrName);
				localChange=renderParams(ctx, static_cast<ParamRegister *>(param.value)->RegisteredParams);
				break;
			case ParamType::NoteSrc:
				renderObjectParamInput(ctx, param, cstrName);
				break;
			case ParamType::PhaseSrc:
				renderObjectParamInput(ctx, param, cstrName);
				break;
			case ParamType::OscSrc:
				renderObjectParamInput(ctx, param, cstrName);
				break;
			case ParamType::SampleData:
				renderObjectParamInput(ctx, param, cstrName);
				break;
			case ParamType::Interpolator:
				renderObjectParamInput(ctx, param, cstrName);
				break;
			case ParamType::DSP:
				renderObjectParamInput(ctx, param, cstrName);
				break;
			default:
				std::cout << "ImGui Type " << param.type << std::endl;
				break;
		}
		change=change || localChange;
	}
	return change;
}



