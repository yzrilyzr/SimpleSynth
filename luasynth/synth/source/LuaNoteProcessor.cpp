#include "LuaNoteProcessor.h"
#include "yzrutil.h"
#include "SimpleSynth.h"
#include "util/ParamRegister.h"
#include "events/ChannelConfig.h"
#include "lang/Exception.h"
#include "events/Note.h"
#include "events/ChannelEvent.h"
#include "dsp/FastSin.h"
#include "dsp/RingBuffer.h"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{

	LuaNoteProcessor::LuaNoteProcessor(const yzrilyzr_lang::String & script)
		: script_(script), L(nullptr), luaInitialized(false),
		luaInitRef(LUA_NOREF), luaGetAmpRef(LUA_NOREF),
		luaPostProcessRef(LUA_NOREF), luaNoMoreDataRef(LUA_NOREF),
		luaNoteOnRef(LUA_NOREF), luaNoteOffRef(LUA_NOREF), luaCCRef(LUA_NOREF){

		initializeLua();
	}

	LuaNoteProcessor::~LuaNoteProcessor(){
		cleanupLua();
	}

	void LuaNoteProcessor::initializeLua(){
		L=luaL_newstate();
		if(!L){
			// 处理Lua状态创建失败
			throw RuntimeException("Lua State Exception");
		}

		// 打开标准库
		luaL_openlibs(L);
		registerLuaAPI();

		if(loadLuaScript()){
			luaInitialized=true;
		} else{
			throw RuntimeException("Lua Script Load Exception");
		}
	}

	void LuaNoteProcessor::registerLuaAPI(){
		// 注册全局函数和表供Lua脚本使用

		// 创建yzr表
		lua_newtable(L);

		// 注册常量
		lua_pushnumber(L, 0.0);
		lua_setfield(L, -2, "SILENCE");

		// 注册工具函数
		lua_pushcfunction(L, [](lua_State * L) -> int{
			u_sample phase=luaL_checknumber(L, 1);
			u_sample freq=luaL_checknumber(L, 2);
			u_sample result=fast_sin(phase, freq);
			lua_pushnumber(L, result);
			return 1;
		});
		lua_setfield(L, -2, "fast_sin");

		lua_pushcfunction(L, [](lua_State * L) -> int{
			u_sample phase=luaL_checknumber(L, 1);
			u_sample result=std::sin(phase);
			lua_pushnumber(L, result);
			return 1;
		});
		lua_setfield(L, -2, "sin");

		lua_pushcfunction(L, [](lua_State * L) -> int{
			u_sample phase=luaL_checknumber(L, 1);
			u_sample result=(RingBufferUtil::mod1(phase) * 2.0 - 1.0);
			lua_pushnumber(L, result);
			return 1;
		});
		lua_setfield(L, -2, "saw");

		lua_pushcfunction(L, [](lua_State * L) -> int{
			u_sample phase=luaL_checknumber(L, 1);
			u_sample result=(RingBufferUtil::mod1(phase) > 0.5?1.0:-1.0);
			lua_pushnumber(L, result);
			return 1;
		});
		lua_setfield(L, -2, "square");

		lua_pushcfunction(L, [](lua_State * L) -> int{
			u_sample x=luaL_checknumber(L, 1);
			u_sample minVal=luaL_optnumber(L, 2, -1.0);
			u_sample maxVal=luaL_optnumber(L, 3, 1.0);
			u_sample result=std::max(minVal, std::min(maxVal, x));
			lua_pushnumber(L, result);
			return 1;
		});
		lua_setfield(L, -2, "clamp");

		// 设置全局表
		lua_setglobal(L, "ss");
	}

	bool LuaNoteProcessor::loadLuaScript(){
		if(!L) return false;

		// 加载Lua脚本
		if(luaL_loadstring(L, script_.c_str()) != LUA_OK){
			// 处理加载错误
			const char * err=lua_tostring(L, -1);
			// 可以记录错误日志
			lua_pop(L, 1);
			return false;
		}

		// 执行脚本
		if(lua_pcall(L, 0, 0, 0) != LUA_OK){
			const char * err=lua_tostring(L, -1);
			// 记录错误
			lua_pop(L, 1);
			return false;
		}

		// 获取Lua函数引用
		auto getFunctionRef=[this](const char * funcName) -> int{
			lua_getglobal(L, funcName);
			if(lua_isfunction(L, -1)){
				return luaL_ref(L, LUA_REGISTRYINDEX);
			} else{
				lua_pop(L, 1);
				return LUA_NOREF;
			}
		};

		luaInitRef=getFunctionRef("init");
		luaGetAmpRef=getFunctionRef("getAmp");
		luaPostProcessRef=getFunctionRef("postProcess");
		luaNoMoreDataRef=getFunctionRef("noMoreData");
		luaNoteOnRef=getFunctionRef("noteOn");
		luaNoteOffRef=getFunctionRef("noteOff");
		luaCCRef=getFunctionRef("cc");

		return true;
	}

	void LuaNoteProcessor::cleanupLua(){
		if(L){
			// 释放所有函数引用
			auto unref=[this](int & ref){
				if(ref != LUA_NOREF){
					luaL_unref(L, LUA_REGISTRYINDEX, ref);
					ref=LUA_NOREF;
				}
			};

			unref(luaInitRef);
			unref(luaGetAmpRef);
			unref(luaPostProcessRef);
			unref(luaNoMoreDataRef);
			unref(luaNoteOnRef);
			unref(luaNoteOffRef);
			unref(luaCCRef);

			lua_close(L);
			L=nullptr;
		}
		luaInitialized=false;
	}

	void LuaNoteProcessor::init(ChannelConfig & cfg){
		if(!luaInitialized || luaInitRef == LUA_NOREF) return;

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaInitRef);
		pushChannelConfigToLua(cfg);

		if(lua_pcall(L, 1, 0, 0) != LUA_OK){
			// 处理错误
			lua_pop(L, 1);
		}
	}

	u_sample LuaNoteProcessor::getAmp(Note & note){
		if(!luaInitialized || luaGetAmpRef == LUA_NOREF){
			return 0.0;
		}

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaGetAmpRef);
		pushNoteToLua(note);

		if(lua_pcall(L, 1, 1, 0) != LUA_OK){
			lua_pop(L, 1);
			return 0.0;
		}

		u_sample result=lua_tonumber(L, -1);
		lua_pop(L, 1);
		return result;
	}

	u_sample LuaNoteProcessor::postProcess(u_sample output){
		if(!luaInitialized || luaPostProcessRef == LUA_NOREF){
			return output;
		}

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaPostProcessRef);
		lua_pushnumber(L, output);

		if(lua_pcall(L, 1, 1, 0) != LUA_OK){
			lua_pop(L, 1);
			return output;
		}

		u_sample result=lua_tonumber(L, -1);
		lua_pop(L, 1);
		return result;
	}

	bool LuaNoteProcessor::noMoreData(Note & note){
		if(!luaInitialized || luaNoMoreDataRef == LUA_NOREF){
			return NoteProcessor::noMoreData(note);
		}

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaNoMoreDataRef);
		pushNoteToLua(note);

		if(lua_pcall(L, 1, 1, 0) != LUA_OK){
			lua_pop(L, 1);
			return NoteProcessor::noMoreData(note);
		}

		bool result=lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}

	void LuaNoteProcessor::cc(ChannelConfig & cfg, ChannelControl & cc){
		if(!luaInitialized || luaCCRef == LUA_NOREF) return;

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaCCRef);
		pushChannelConfigToLua(cfg);
		pushChannelControlToLua(cc);

		if(lua_pcall(L, 2, 0, 0) != LUA_OK){
			lua_pop(L, 1);
		}
	}

	void LuaNoteProcessor::noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
		if(!luaInitialized || luaNoteOnRef == LUA_NOREF) return;

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaNoteOnRef);
		pushChannelConfigToLua(cfg);
		lua_pushinteger(L, id);
		lua_pushnumber(L, vel);

		if(lua_pcall(L, 3, 0, 0) != LUA_OK){
			lua_pop(L, 1);
		}
	}

	void LuaNoteProcessor::noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
		if(!luaInitialized || luaNoteOffRef == LUA_NOREF) return;

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaNoteOffRef);
		pushChannelConfigToLua(cfg);
		lua_pushinteger(L, id);
		lua_pushnumber(L, vel);

		if(lua_pcall(L, 3, 0, 0) != LUA_OK){
			lua_pop(L, 1);
		}
	}

	NoteProcPtr LuaNoteProcessor::clone(){
		return std::make_shared<LuaNoteProcessor>(script_);
	}

	// Lua绑定辅助方法实现
	void LuaNoteProcessor::pushNoteToLua(Note & note){
		lua_newtable(L);
		// 这里添加Note对象的属性到Lua表
		// 例如：频率、音量、相位等
		lua_pushnumber(L, note.uniqueID);
		lua_setfield(L, -2, "uniqueID");
		lua_pushnumber(L, note.idSynth);
		lua_setfield(L, -2, "idSynth");
		lua_pushnumber(L, note.freqSynth);
		lua_setfield(L, -2, "freqSynth");
		lua_pushnumber(L, note.velocitySynth);
		lua_setfield(L, -2, "velocitySynth");
		lua_pushnumber(L, note.phaseSynth);
		lua_setfield(L, -2, "phaseSynth");

	}

	void LuaNoteProcessor::pushChannelConfigToLua(ChannelConfig & cfg){
		lua_newtable(L);
		// 添加ChannelConfig属性到Lua表
		// 例如：采样率、参数等
	}

	void LuaNoteProcessor::pushChannelControlToLua(ChannelControl & cc){
		lua_newtable(L);
		// 添加ChannelControl属性到Lua表
	}

} // namespace yzrilyzr_simplesynth