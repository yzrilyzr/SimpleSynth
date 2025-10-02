#include "VVVF.h"
#include "dsp/PWM.h"
#include "events/ChannelEvent.h"
#include "interface/NoteTuning.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	VVVF::VVVF() : VVVF(nullptr){
		registerParamFreq("fTri", &fTri);
		registerParamFreq("fSin", &fSin);
		registerParamNormal01("vSine", &vSine);
		static int32_t min=0, max=128;
		registerParam("sync", ParamType::Int, &sync, &min, &max);
	}
	VVVF::VVVF(std::shared_ptr<PhaseSrc> freq) :Osc(freq){}
	void VVVF::cc(ChannelConfig & cfg, ChannelControl & cc){
		PNData & data=cfg.nrpn;
		if(cc.isMSB()){
			switch(data.select){
				case 10102:
					sync=data.dataMSB;
					break;
				case 10103:
					vSine=data.dataMSB / PNData::MSB_MAX;
					break;
			}
		} else if(cc.isLSB()){
			switch(data.select){
				case 10100:
					fSin=data.data / PNData::LSB_MAX * 120;
					break;
				case 10101:
					fTri=data.data / PNData::LSB_MAX * 1000;
					break;
			}
		}
	}
	u_sample VVVF::getAmp(Note & note){
		VVVFKeyData * data=getData(note);
		u_time deltaTime=note.cfg->deltaTime;
		u_freq fTri1=fTri;
		u_freq fSin1=fSin;
		if(sync == 127){
			fTri1=note.cfg->tuning->getFrequencyByID(note.id);
		} else if(1 <= sync && sync <= 60){
			if(data->lastSync != sync){
				data->lastSync=sync;
				data->pTri=data->pSin;
			}
			fTri1=sync * fSin1;
		} else if(61 <= sync && sync <= 120){
			if(data->lastSync != sync){
				data->lastSync=sync;
				data->pTri=data->pSin;
			}
			fSin1=note.cfg->tuning->getFrequencyByID(note.id);
			fTri1=(sync - 60) * fSin1;
		}
		data->pTri+=deltaTime * fTri1;
		data->pSin+=deltaTime * fSin1;
		double vTri=PWM::pwm(data->pTri, 0, 0.5, 0.5, 0);
		double phiSin=data->pSin * _2PI;
		double vSinP=vSine * sin(phiSin);
		double vSinN=vSine * -sin(phiSin);
		int v1=vSinP > vTri?1:-1;
		int v2=vSinN > vTri?1:-1;
		return (v1 - v2) * note.velocitySynth;
	}
	VVVFKeyData * VVVF::init(VVVFKeyData * data, Note & note){
		if(data == nullptr) data=new VVVFKeyData();
		data->pSin=0;
		data->pTri=0;
		return data;
	}
	String VVVF::toString() const{
		return StringFormat::object2string("VVVF", getPhaseSource());
	}
}