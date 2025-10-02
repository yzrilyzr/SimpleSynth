#pragma once
#include <vector>
#include <string>
#include "util/Lang.h"
#include "lang/String.h"
class LangToEnum{
	public:
	std::vector<std::string> strs;
	std::vector<const char *> cstrs;
	yzrilyzr_lang::String currentLocale;
	bool empty(yzrilyzr_util::Lang & lang) const{
		return cstrs.empty() || currentLocale != lang.getCurrentLocale();
	}
	void init(yzrilyzr_util::Lang & lang, std::vector<yzrilyzr_lang::String> keys){
		cstrs.clear();
		strs.clear();
		currentLocale=lang.getCurrentLocale();
		for(auto & str : keys){
			std::string tmp=lang.get(str).tostring(yzrilyzr_lang::StringEncoding::UTF8);
			strs.emplace_back(tmp);
		}
		for(const auto & str : strs){
			cstrs.emplace_back(str.c_str());
		}
	}
	u_index size() const{
		return cstrs.size();
	}
	const char ** data(){
		return cstrs.data();
	}
	const char * operator[](u_index index){
		return cstrs[index];
	}
};