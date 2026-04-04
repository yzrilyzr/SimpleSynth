#include "BanksHammer.h"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void BanksHammer::init(u_sample sampleRate, u_sample m, u_sample K, u_sample p, u_sample Z, u_sample alpha){
		this->sampleRate=sampleRate;
		this->p=p;
		this->K=K;
		this->mi=1.0 / m;
		this->Z2i=1.0 / (2.0 * Z);
		this->a=0.0;
		this->F=0.0;
		this->intv.resetMemory();
		this->intv.init((int)(sampleRate * 2));
		this->intvh.resetMemory();
		this->intvh.init((int)(sampleRate * 2));
		this->unitDelay.init(0);
		this->vh=0;
		this->oldvin=0;
		this->intvh.procDsp(0);
	}
	u_sample BanksHammer::load(u_sample vin){
		u_sample rvin=(vin + this->oldvin) * 0.5;
		u_sample F1, F2;
		u_sample vs=(rvin + this->F * this->Z2i);
		u_sample deltaV=this->vh - vs;
		u_sample deltaY=this->intv.procDsp(deltaV);
		u_sample up=(deltaY > 0.0f)?pow(deltaY, this->p):0.0;
		up=up * this->K;
		this->a=-this->F * this->mi;
		this->vh=this->intvh.procDsp(this->a);
		F1=this->F=this->unitDelay.procDsp(up);
		vs=(vin + this->F * this->Z2i);
		deltaV=this->vh - vs;
		deltaY=this->intv.procDsp(deltaV);
		up=(deltaY > 0.0)?pow(deltaY, this->p):0.0;
		up=up * this->K;
		this->a=-this->F * this->mi;
		this->vh=this->intvh.procDsp(this->a);
		F2=this->F=this->unitDelay.procDsp(up);
		return (F1 + F2) * 0.5;
	}
	void BanksHammer::trigger(u_sample v){
		this->F=0.0;
		this->vh=0.0;
		this->intv.resetMemory();
		this->intv.init((int)(this->sampleRate * 2));
		this->intvh.resetMemory();
		this->intvh.init((int)(this->sampleRate * 2));
		this->unitDelay.init(0);
		this->intvh.procDsp(v * this->sampleRate * 2);
	}
	String BanksHammer::toString()const{
		return "BanksHammer";
	}
}