#include "nlohmann/json.hpp"
#include "collection/ArrayList.hpp"
#include "lang/Exception.h"
#include "SimpleSynthProject.h"
#include "interface/NoteProcessor.h"
#include "dsp/DSP.h"
using json=nlohmann::json;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_util;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_lang;

json obj2json(ArrayList<ProjectObject *> & arr, NoteProcPtr finalProcessor){
	json j=json::array();
	for(const ProjectObject * obj : arr){
		json & j1=obj->to_json();
		if(finalProcessor == obj->paramRegPtr){
			j1["finalProcessor"]=true;
		}
		j.push_back(j1);
	}
	return j;
}
void json2obj(json & j, ArrayList<ProjectObject *> & objects){
	for(const auto & item : j){
		ProjectObject * obj=new ProjectObject();
		obj->from_json(item);
		objects.add(obj);
	}
	for(ProjectObject * obj : objects){
		for(auto & param : obj->paramRegPtr->RegisteredParams){
			const char * key=param.name.c_str();
			json & fromJSON=obj->fromJSON;
			if(!fromJSON.contains(key))continue;
			switch(param.type){
				case ParamType::Float:
				case ParamType::Double:
				case ParamType::Int:
				case ParamType::UInt:
				case ParamType::Long:
				case ParamType::ULong:
				case ParamType::Bool:
				case ParamType::Size:
				case ParamType::Enum:
				case ParamType::Freq:
				case ParamType::Time:
				case ParamType::TimeMs:
				case ParamType::Sample:
				case ParamType::Gain:
				case ParamType::Sub:
					break;
				default:
					uint64_t id=fromJSON.value(key, static_cast<uint64_t>(0));
					if(id == 0)break;
					for(ProjectObject * obj1 : objects){
						if(obj == obj1)continue;
						json & j1=obj1->fromJSON;
						uint64_t obj1id=j1.value("id", static_cast<uint64_t>(0));
						if(id == obj1id){
							*static_cast<u_sp<void> *>(param.value)=obj1->paramRegPtr;
							break;
						}
					}
					break;
			}
		}
	}
}