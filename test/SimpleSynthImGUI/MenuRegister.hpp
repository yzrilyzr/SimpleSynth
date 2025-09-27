#pragma once
#include <yzrutil.h>
#include <util/Lang.h>
#include <util/ParamRegister.h>
#include <functional>
#include <map>
#include <vector>
#include <string>

class CurrentProjectContext;
class ProjectObject;
EBCLASS(MenuRegister){
	public:
	using CreatorFunc=std::function<std::shared_ptr<yzrilyzr_util::ParamRegister>()>;
	using RenderFunc=std::function<void(CurrentProjectContext &, ProjectObject &)>;
	struct MenuRegisterObject{
		std::string name;
		std::string category;
		std::string showName;
		std::string showCategory;
		CreatorFunc cfunc;
		RenderFunc rfunc;
		bool enableOriginalRender=true;
	};
	std::map<std::string, std::vector<MenuRegisterObject>> regObjects;
	std::vector<MenuRegisterObject> allRegObjects;
	std::map<std::string, std::string> categoryToShowCategory;
	void registerModule(yzrilyzr_util::Lang & lang, const std::string & category, const std::string & showCategory, const std::string & name, const std::string & showNameKey, CreatorFunc func, RenderFunc rf, bool enableOriginalRender){
		registerModule(category, showCategory, name, lang.get(showNameKey), func, rf, enableOriginalRender);
	}
	void registerModule(yzrilyzr_util::Lang & lang, const std::string & category, const std::string & showCategory, const std::string & name, const std::string & showNameKey, CreatorFunc func, RenderFunc rf){
		registerModule(category, showCategory, name, lang.get(showNameKey), func, rf);
	}
	void registerModule(yzrilyzr_util::Lang & lang, const std::string & category, const std::string & showCategory, const std::string & name, const std::string & showNameKey, CreatorFunc func){
		registerModule(category, showCategory, name, lang.get(showNameKey), func, nullptr);
	}
	void registerModule(const std::string & category, const std::string & showCategory, const std::string & name, const std::string & showName, CreatorFunc cf, RenderFunc rf){
		registerModule(category, showCategory, name, showName, cf, rf, true);
	}
	void registerModule(const std::string & category, const std::string & showCategory, const std::string & name, const std::string & showName, CreatorFunc cf, RenderFunc rf,bool enableOriginalRender){
		categoryToShowCategory[category]=showCategory;
		MenuRegisterObject reg{name, category, showName, showCategory, cf, rf,enableOriginalRender};
		allRegObjects.push_back(reg);
		regObjects[category].emplace_back(reg);
	}
	void clear(){
		categoryToShowCategory.clear();
		allRegObjects.clear();
		regObjects.clear();
	}
};