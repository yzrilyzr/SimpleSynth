#pragma once
#include "SimpleSynth.h"
#include "Note.h"
#include "events/ChannelEvent.h"
#include <util/MIDIFile.h>
#include <unordered_map>
namespace yzrilyzr_simplesynth{
	EBCLASS(NRPNBinding){
	private:
	struct NRPNBindStruct{
		void * paramRegPtr=nullptr;
		bool lsb=false;
		float (*convertFuncF)(int16_t val)=nullptr;
		int16_t(*convertFuncI)(int16_t val)=nullptr;
		bool (*convertFuncB)(int16_t val)=nullptr;
	};
	std::unordered_map<int16_t, NRPNBindStruct> nrpnMap;
	void bind(int16_t NRPN_ID, void * val, bool bigRange){
		NRPNBindStruct str;
		str.lsb=bigRange;
		str.paramRegPtr=val;
		nrpnMap[NRPN_ID]=str;
	}
	public:
	static constexpr int SMALL_MAX=127;
	static constexpr int BIG_MAX=16384;
	static float _defConvertF(int16_t val){
		return static_cast<float>(val) / static_cast<float>(BIG_MAX);
	}
	static int16_t _defConvertI(int16_t val){
		return val;
	}
	static bool _defConvertB(int16_t val){
		return val > 64;
	}
	void bindFloat(int16_t NRPN_ID, float * val, bool bigRange, float(*convertFunc)(int16_t val)=_defConvertF){
		bind(NRPN_ID, val, bigRange);
		nrpnMap[NRPN_ID].convertFuncF=convertFunc;
	}
	void bindInt(int16_t NRPN_ID, int16_t * val, bool bigRange, int16_t(*convertFunc)(int16_t val)=_defConvertI){
		bind(NRPN_ID, val, bigRange);
		nrpnMap[NRPN_ID].convertFuncI=convertFunc;
	}
	void bindBoolean(int16_t NRPN_ID, bool * val, bool bigRange, bool(*convertFunc)(int16_t val)=_defConvertB){
		bind(NRPN_ID, val, bigRange);
		nrpnMap[NRPN_ID].convertFuncB=convertFunc;
	}
	void procCC(IMixer * mixer, IChannel * channel, ChannelControl * cc){
		/*if(cc->control == MIDIFile::CC::DATA_ENTRY_MSB){
			int16_t nrpnid=channel->getNRPN();
			NRPNBindStruct & str=nrpnMap[nrpnid];
			if(str.lsb)return;
			int16_t pndata=channel->getNRPNData();
			if(str.convertFuncF != nullptr)(*(float*)str.ptr)=str.convertFuncF(pndata);
			if(str.convertFuncI != nullptr)(*(int16_t *)str.ptr)=str.convertFuncI(pndata);
			if(str.convertFuncB != nullptr)(*(bool *)str.ptr)=str.convertFuncB(pndata);
		} else if(cc->control == MIDIFile::CC::DATA_ENTRY_LSB){
			int16_t nrpnid=channel->getNRPN();
			NRPNBindStruct & str=nrpnMap[nrpnid];
			if(!str.lsb)return;
			int16_t pndata=channel->getNRPNData();
			if(str.convertFuncF != nullptr)(*(float *)str.ptr)=str.convertFuncF(pndata);
			if(str.convertFuncI != nullptr)(*(int16_t *)str.ptr)=str.convertFuncI(pndata);
			if(str.convertFuncB != nullptr)(*(bool *)str.ptr)=str.convertFuncB(pndata);
		}*/
	}
	};
}