#include "ProjectObject.h"
#include "SimpleSynthProject.h"
#include "array/SampleProvider.h"
#include "dsp/DSP.h"
#include "interpolator/Interpolator.h"
#include "lang/Boxing.h"
#include <string>
#include "synth/source/AmplitudeSources.h"
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
				std::shared_ptr<void> * paramRegPtr=reinterpret_cast<std::shared_ptr<void>*>(p.value);
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
		if(auto datap2=std::dynamic_pointer_cast<SampleArray>(datap)){
			json jsonArray=json::array();
			for(u_index i=0;i < datap2->length;i++){
				jsonArray.push_back((*datap2)[i]);
			}
			storedDataJSON[p.first.c_str()]={{"data", jsonArray}, {"type", "SampleArray"}};
		} else if(auto datap2=std::dynamic_pointer_cast<DoubleArray>(datap)){
			json jsonArray=json::array();
			for(u_index i=0;i < datap2->length;i++){
				jsonArray.push_back((*datap2)[i]);
			}
			storedDataJSON[p.first.c_str()]={{"data", jsonArray}, {"type", "DoubleArray"}};
		} else if(auto datap2=std::dynamic_pointer_cast<Short>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		} else if(auto datap2=std::dynamic_pointer_cast<Integer>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		} else if(auto datap2=std::dynamic_pointer_cast<Long>(datap)){
			storedDataJSON[p.first.c_str()]=datap2->value;
		} else if(auto datap2=std::dynamic_pointer_cast<Float>(datap)){
			a[p.first.c_str()]=datap2->value;
		} else if(auto datap2=std::dynamic_pointer_cast<Double>(datap)){
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
						auto arrPtr=std::make_shared<SampleArray>(data.size());
						for(u_index i=0;i < data.size();i++){
							(*arrPtr)[i]=data[i].get<u_sample>();
						}
						storeData[key]=arrPtr;
					} else if(type == "DoubleArray"){
						auto arrPtr=std::make_shared<DoubleArray>(data.size());
						for(u_index i=0;i < data.size();i++){
							(*arrPtr)[i]=data[i].get<double>();
						}
						storeData[key]=arrPtr;
					}
				} else if(value.is_number_integer()){
					auto intPtr=std::make_shared<Integer>(value.get<int32_t>());
					storeData[key]=intPtr;
				} else if(value.is_number_unsigned()){
					auto longPtr=std::make_shared<Long>(value.get<int64_t>());
					storeData[key]=longPtr;
				} else if(value.is_number_float()){
					auto doublePtr=std::make_shared<Double>(value.get<double>());
					storeData[key]=doublePtr;
				}
			}
		}
	}
	fromJSON=j;
	loadStoredData=true;
}

void ProjectObject::renderWindow(CurrentProjectContext & ctx){
	String windowName=showName + "##" + std::to_string((int64_t)this);
	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::Begin(windowName.c_str(UTF8), &showWindow, ImGuiWindowFlags_None);
	windowPos=ImGui::GetWindowPos();
	windowSize=ImGui::GetWindowSize();
	if(ImGui::Button(ctx.LANG.getc("window.project_object.drag_this"))){}
	if(ImGui::BeginDragDropSource()){
		if(std::dynamic_pointer_cast<Osc>(paramRegPtr))ctx.dragPayloadType=ctx.payloadType_Osc;
		else if(std::dynamic_pointer_cast<NoteProcessor>(paramRegPtr))ctx.dragPayloadType=ctx.payloadType_NoteProcessor;
		else if(std::dynamic_pointer_cast<PhaseSrc>(paramRegPtr))ctx.dragPayloadType=ctx.payloadType_PhaseSrc;
		else if(std::dynamic_pointer_cast<Interpolator>(paramRegPtr))ctx.dragPayloadType=ctx.payloadType_Interpolator;
		else if(std::dynamic_pointer_cast<DSP>(paramRegPtr))ctx.dragPayloadType=ctx.payloadType_DSP;
		else if(std::dynamic_pointer_cast<SampleProvider>(paramRegPtr))ctx.dragPayloadType=ctx.payloadType_Sample;
		ImGui::SetDragDropPayload(ctx.dragPayloadType, this, sizeof(ProjectObject));
		ImGui::Text(ctx.LANG.getf("window.project_object.dragging_this").c_str(UTF8));
		ImGui::EndDragDropSource();
	}
	ImVec2 button1Min=ImGui::GetItemRectMin();
	ImVec2 button1Max=ImGui::GetItemRectMax();
	ctx.paramUIBindings.push_back(ParamUIBinding{paramRegPtr.get(), ImVec2((button1Min.x + button1Max.x) * 0.5f, (button1Min.y + button1Max.y) * 0.5f)});
	ImGui::Separator();
	ImGui::Text(ctx.LANG.getc("window.project_object.params"));
	if(enableOriginalRender){
		ctx.paramChange=renderParams(ctx, std::static_pointer_cast<ParamRegister>(paramRegPtr)->RegisteredParams) || ctx.paramChange;
	}
	if(renderFunc != nullptr)renderFunc(ctx, *this);
	if(!ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
		ctx.dragPayloadType=nullptr;
	}
	ImGui::End();
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
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::Float:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Float, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::Freq:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f Hz", ImGuiSliderFlags_Logarithmic);
				break;
			case ParamType::Time:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f s", ImGuiSliderFlags_Logarithmic);
				break;
			case ParamType::TimeMs:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f ms", ImGuiSliderFlags_Logarithmic);
				break;
			case ParamType::Int:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_S32, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::UInt:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_U32, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::Long:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_S64, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::ULong:
				localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_U64, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::Size:
				localChange=ImGui::SliderScalar(cstrName, sizeof(u_index) == 8?ImGuiDataType_U64:ImGuiDataType_U32, param.value, param.valueMin, param.valueMax);
				break;
			case ParamType::Gain:
				localChange=ImGui::SliderScalar(cstrName, sizeof(u_sample) == 8?ImGuiDataType_Double:ImGuiDataType_Float, param.value, param.valueMin, param.valueMax, "%.3f mul", ImGuiSliderFlags_Logarithmic);
				break;
			case ParamType::Sample:
				localChange=ImGui::SliderScalar(cstrName, sizeof(u_sample) == 8?ImGuiDataType_Double:ImGuiDataType_Float, param.value, param.valueMin, param.valueMax, "%.3f");
				break;
			case ParamType::Enum:
				localChange=ImGui::Combo(cstrName, static_cast<int *>(param.value), static_cast<const char **>(param.valueMin), *static_cast<int *>(param.valueMax));
				break;
			case ParamType::Sub:
				ImGui::Text(cstrName);
				localChange=renderParams(ctx, static_cast<ParamRegister *>(param.value)->RegisteredParams);
				break;
			case ParamType::NoteSrc:
				localChange=renderProjectObjectParam<NoteProcessor>(ctx, param, cstrName, {ctx.payloadType_NoteProcessor, ctx.payloadType_Osc});
				break;
			case ParamType::PhaseSrc:
				localChange=renderProjectObjectParam<PhaseSrc>(ctx, param, cstrName, {ctx.payloadType_PhaseSrc});
				break;
			case ParamType::OscSrc:
				localChange=renderProjectObjectParam<Osc>(ctx, param, cstrName, {ctx.payloadType_Osc});
				break;
			case ParamType::SampleData:
				localChange=renderProjectObjectParam<SampleProvider>(ctx, param, cstrName, {ctx.payloadType_Sample});
				break;
			case ParamType::Interpolator:
				localChange=renderProjectObjectParam<Interpolator>(ctx, param, cstrName, {ctx.payloadType_Interpolator});
				break;
			case ParamType::DSP:
				localChange=renderProjectObjectParam<DSP>(ctx, param, cstrName, {ctx.payloadType_DSP});
				break;
			default:
				std::cout << "ImGui Type " << param.type << std::endl;
				break;
		}
		change=change || localChange;
	}
	return change;
}