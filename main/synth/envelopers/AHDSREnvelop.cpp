#include "AHDSREnvelop.h"
#include "EnvUtil.h"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/PNData.h"
#include "interface/NoteProcessor.h"
#include "interpolator/Interpolator.h"
#include "lang/StringFormat.hpp"
#include "util/Util.h"
#include "yzrutil.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	AHDSREnvelop::AHDSREnvelop(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 sustainVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime,
							   std::shared_ptr<Interpolator> aCurve, std::shared_ptr<Interpolator> dCurve, std::shared_ptr<Interpolator> rCurve){
		this->delayTime=delayTime / 1000.0;
		this->attackTime=attackTime / 1000.0;
		this->holdTime=holdTime / 1000.0;
		this->decayTime=decayTime / 1000.0;
		this->releaseTime=releaseTime / 1000.0;
		this->forceReleaseTime=forceReleaseTime / 1000.0;
		this->sustainVolume=sustainVolume;
		this->aCurve=std::move(aCurve);
		this->dCurve=std::move(dCurve);
		this->rCurve=std::move(rCurve);
		this->canSustain=canSustain;
	}
	AHDSREnvelop::AHDSREnvelop(){
		registerParamTime("Delay", &delayTime);
		registerParamTime("Attack", &attackTime);
		registerParamTime("Hold", &holdTime);
		registerParamTime("Decay", &decayTime);
		registerParamNormal01("SustainVol", &sustainVolume);
		registerParamBool("Sustainable", &canSustain);
		registerParamTime("Release", &releaseTime);
		registerParamTime("ForceRelease", &forceReleaseTime);
		registerParamInterpolator("ACurve", &aCurve);
		registerParamInterpolator("DCurve", &dCurve);
		registerParamInterpolator("RCurve", &rCurve);
	}
	void AHDSREnvelop::init(ChannelConfig & cfg){
		if(aCurve == nullptr) aCurve=EnvUtil::Line();
		if(dCurve == nullptr) dCurve=EnvUtil::Line();
		if(rCurve == nullptr) rCurve=EnvUtil::Line();
	}
	bool AHDSREnvelop::noMoreData(Note & note){
		AHDSREnvelopKeyData * n1=getData(note);
		if(n1->releaseTime == -1) return false;
		u_time time1=note.cfg->currentTime - n1->releaseTime;
		u_time nmdTime=this->releaseTime;
		if(note.fclosed(*note.cfg)) nmdTime=std::min(this->releaseTime, this->forceReleaseTime);
		return time1 > nmdTime;
	}
	NoteProcPtr AHDSREnvelop::clone(){
		return std::make_shared<AHDSREnvelop>(delayTime * 1000.0, attackTime * 1000.0, holdTime * 1000.0, decayTime * 1000.0, sustainVolume, canSustain, releaseTime * 1000.0, forceReleaseTime * 1000.0, aCurve, dCurve, rCurve);
	}
	u_sample AHDSREnvelop::getAmp(Note & note){
		u_time time=note.passedTime;
		if(time < 0) return 0;
		AHDSREnvelopKeyData & n1=*getData(note);
		if(note.fclosed(*note.cfg)){
			if(n1.releaseTime == -1) n1.releaseTime=note.cfg->currentTime;
			u_time time1=note.cfg->currentTime - n1.releaseTime;
			u_time nmdTime=std::min(this->releaseTime, this->forceReleaseTime);
			if(nmdTime == 0)return 0;
			return n1.releaseVolume - n1.releaseVolume * (1 - rCurve->y(Util::clamp01(1 - time1 / nmdTime)));//decayToVolume->0
		}
		if(isNoteNotReleased(note)){
			n1.releaseTime=-1;
			u_time t=0;
			u_time lDelay=delayTime;
			u_time lAttack=n1.attackTime;
			u_time lHold=holdTime;
			u_time lDecay=decayTime;
			if(time < (t+=lDelay)){//delay
				return n1.releaseVolume=0;
			} else if(time < (t+=lAttack)){//attack
				if(lAttack == 0)return 0;
				u_time x=(time - lDelay) / lAttack;
				return n1.releaseVolume=aCurve->y(Util::clamp01(x));//0->1
			} else if(time < (t+=lHold)){//hold
				return n1.releaseVolume=1.0;
			} else if(time < (t+=lDecay)){//decay
				if(lDecay == 0)return n1.releaseVolume;
				u_time x=(time - lDelay - lAttack - lHold) / lDecay;
				return n1.releaseVolume=(1 - (1 - sustainVolume) * (1 - dCurve->y(Util::clamp01(1 - x))));//1->decayToVolume
			}
			//sustain
			return n1.releaseVolume=sustainVolume;
		} else{//release
			if(n1.releaseTime == -1) n1.releaseTime=note.cfg->currentTime;
			u_time time1=note.cfg->currentTime - n1.releaseTime;
			if(releaseTime == 0)return 0;
			return n1.releaseVolume - n1.releaseVolume * (1 - rCurve->y(Util::clamp01(1 - time1 / releaseTime)));//decayToVolume->0
		}
	}
	std::string AHDSREnvelop::toString()const{
		return StringFormat::object2string("AHDSREnvelop",
										   (int)(delayTime * 1000),
										   (int)(attackTime * 1000),
										   (int)(holdTime * 1000),
										   (int)(decayTime * 1000),
										   sustainVolume,
										   canSustain,
										   releaseTime,
										   forceReleaseTime,
										   aCurve,
										   dCurve,
										   rCurve);
	}
	AHDSREnvelopKeyData * AHDSREnvelop::init(AHDSREnvelopKeyData * data, Note & note){
		if(data == nullptr) data=new AHDSREnvelopKeyData();
		if(note.cfg->Legato){
			data->attackTime=0.005;
		} else{
			data->attackTime=attackTime;
		}
		data->releaseTime=-1;
		data->releaseVolume=0;
		return data;
	}
	bool AHDSREnvelop::isNoteNotReleased(Note & note) const{
		if(canSustain){
			return !note.closed(*note.cfg);
		}
		u_time time=note.passedTime;
		bool decaying=time < delayTime + attackTime + decayTime + holdTime;
		return !note.closed(*note.cfg) && decaying;
	}
	void AHDSREnvelop::cc(ChannelConfig & cfg, ChannelControl & cc){
		PNData & data=cfg.nrpn;
		if(cc.isMSB()){
			switch(data.select){
				case 3000:
					delayTime=pow(10000.0, data.dataMSB / 127.0f) / 1000.0;
					break;
				case 3001:
					attackTime=pow(10000.0, data.dataMSB / 127.0f) / 1000.0;
					break;
				case 3002:
					holdTime=pow(10000.0, data.dataMSB / 127.0f) / 1000.0;
					break;
				case 3003:
					decayTime=pow(10000.0, data.dataMSB / 127.0f) / 1000.0;
					break;
				case 3004:
					sustainVolume=data.dataMSB / 127.0f;
					break;
				case 3005:
					canSustain=data.dataMSB >= 64;
					break;
				case 3006:
					releaseTime=pow(10000.0, data.dataMSB / 127.0f) / 1000.0;
					break;
			}
		}
	}
}