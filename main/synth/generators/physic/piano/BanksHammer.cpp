#include "BanksHammer.h"
namespace yzrilyzr_simplesynth{
	void BanksHammer::init(double sampleRate, double m, double K, double p, double Z, double alpha){
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
	double BanksHammer::load(double vin){
		double rvin=(vin + this->oldvin) * 0.5;
		double F1, F2;
		double vs=(rvin + this->F * this->Z2i);
		double deltaV=this->vh - vs;
		double deltaY=this->intv.procDsp(deltaV);
		double up=(deltaY > 0.0f)?pow(deltaY, this->p):0.0;
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
	void BanksHammer::trigger(double v){
		this->F=0.0;
		this->vh=0.0;
		this->intv.resetMemory();
		this->intv.init((int)(this->sampleRate * 2));
		this->intvh.resetMemory();
		this->intvh.init((int)(this->sampleRate * 2));
		this->unitDelay.init(0);
		this->intvh.procDsp(v * this->sampleRate * 2);
	}
	std::string BanksHammer::toString()const{
		return "BanksHammer";
	}
}