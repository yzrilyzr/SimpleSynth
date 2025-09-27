#pragma once
#include <vector>
#include <string>
#include "util/Lang.h"
class LangToEnum{
	public:
	std::vector<std::string> strs;
	std::vector<const char *> cstrs;
	std::string currentLocale;
	bool empty(yzrilyzr_util::Lang & lang) const{
		return cstrs.empty()|| currentLocale!=lang.getCurrentLocale();
	}
	void init(yzrilyzr_util::Lang & lang, std::vector<std::string> keys){
		cstrs.clear();
		strs.clear();
		currentLocale=lang.getCurrentLocale();
		for(auto &str : keys){
			strs.push_back(lang.get(str));
		}
		for(const auto &str : strs){
			cstrs.push_back(str.c_str());
		}
	}
	size_t size() const{
		return cstrs.size();
	}
	const char ** data(){
		return cstrs.data();
	}
	const char * operator[](size_t index){
		return cstrs[index];
	}
};