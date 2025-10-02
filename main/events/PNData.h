#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PNData){
	public:
	static constexpr float LSB_MAX=16383.0;
	static constexpr float MSB_MAX=127.0;
	bool active=false;
	uint16_t select=0;
	uint16_t data=0;
	uint16_t dataMSB=0;
	uint16_t dataLSB=0;
	void reset(){
		active=false;
		select=0;
		data=0;
		dataMSB=0;
		dataLSB=0;
	}
	void selectMSB(uint16_t v){
		active=true;
		select=v;
	}
	void selectLSB(uint16_t v){
		active=true;
		select=(select<<7)|v;
	}
	void setDataMSB(uint16_t v){
		dataMSB=v;
		data=(dataMSB << 7) | dataLSB;
	}
	void setDataLSB(uint16_t v){
		dataLSB=v;
		data=(dataMSB << 7) | dataLSB;
	}
	void set(const PNData & other){
		active=other.active;
		select=other.select;
		data=other.data;
		dataLSB=other.dataLSB;
		dataMSB=other.dataMSB;
	}
	};
}