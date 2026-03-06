#pragma once
#include "yzrutil.h"
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "lua.hpp"

namespace yzrilyzr_simplesynth{
    class Note;
    class ChannelConfig;

    ECLASS(LuaNoteProcessor, public NoteProcessor){
        private:
        yzrilyzr_lang::String script_;
        lua_State * L;
        bool luaInitialized;

        // Lua函数引用
        int luaInitRef;
        int luaGetAmpRef;
        int luaPostProcessRef;
        int luaNoMoreDataRef;
        int luaNoteOnRef;
        int luaNoteOffRef;
        int luaCCRef;

        void initializeLua();
        void registerLuaAPI();
        bool loadLuaScript();
        void cleanupLua();

        public:
        LuaNoteProcessor(const yzrilyzr_lang::String & script);
        ~LuaNoteProcessor() override;

        void init(ChannelConfig & cfg) override;
        u_sample getAmp(const Note & note) override;
        u_sample postProcess(u_sample output) override;
        bool noMoreData(const Note & note) override;
        void cc(ChannelConfig & cfg, ChannelControl & cc) override;
        void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel) override;
        void noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel) override;
        NoteProcPtr clone() override;

        // Lua绑定辅助方法
        void pushNoteToLua(const Note & note);
        void pushChannelConfigToLua(ChannelConfig & cfg);
        void pushChannelControlToLua(ChannelControl & cc);
    };
}