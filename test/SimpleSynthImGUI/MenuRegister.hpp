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
		yzrilyzr_lang::String name;
		yzrilyzr_lang::String category;
		yzrilyzr_lang::String showName;
		yzrilyzr_lang::String showCategory;
		CreatorFunc cfunc;
		RenderFunc rfunc;
		bool enableOriginalRender=true;
	};
	std::unordered_map<yzrilyzr_lang::String, std::vector<MenuRegisterObject>> regObjects;
	std::vector<MenuRegisterObject> allRegObjects;
	std::unordered_map<yzrilyzr_lang::String, yzrilyzr_lang::String> categoryToShowCategory;
	void registerModule(yzrilyzr_util::Lang & lang, const yzrilyzr_lang::String & category, const yzrilyzr_lang::String & showCategory, const yzrilyzr_lang::String & name, const yzrilyzr_lang::String & showNameKey, CreatorFunc func, RenderFunc rf, bool enableOriginalRender){
		registerModule(category, showCategory, name, lang.get(showNameKey), func, rf, enableOriginalRender);
	}
	void registerModule(yzrilyzr_util::Lang & lang, const yzrilyzr_lang::String & category, const yzrilyzr_lang::String & showCategory, const yzrilyzr_lang::String & name, const yzrilyzr_lang::String & showNameKey, CreatorFunc func, RenderFunc rf){
		registerModule(category, showCategory, name, lang.get(showNameKey), func, rf);
	}
	void registerModule(yzrilyzr_util::Lang & lang, const yzrilyzr_lang::String & category, const yzrilyzr_lang::String & showCategory, const yzrilyzr_lang::String & name, const yzrilyzr_lang::String & showNameKey, CreatorFunc func){
		registerModule(category, showCategory, name, lang.get(showNameKey), func, nullptr);
	}
	void registerModule(const yzrilyzr_lang::String & category, const yzrilyzr_lang::String & showCategory, const yzrilyzr_lang::String & name, const yzrilyzr_lang::String & showName, CreatorFunc cf, RenderFunc rf){
		registerModule(category, showCategory, name, showName, cf, rf, true);
	}
	void registerModule(const yzrilyzr_lang::String & category, const yzrilyzr_lang::String & showCategory, const yzrilyzr_lang::String & name, const yzrilyzr_lang::String & showName, CreatorFunc cf, RenderFunc rf, bool enableOriginalRender){
		categoryToShowCategory[category]=showCategory;
		MenuRegisterObject reg{name, category, showName, showCategory, cf, rf, enableOriginalRender};
		allRegObjects.push_back(reg);
		regObjects[category].emplace_back(reg);
	}
	void clear(){
		categoryToShowCategory.clear();
		allRegObjects.clear();
		regObjects.clear();
	}
};