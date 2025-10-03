#include "StulovHammer.h"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void StulovHammer::init(u_sample sampleRate, u_sample m, u_sample K, u_sample p, u_sample Z, u_sample alpha){
		this->sampleRate=sampleRate;
		this->p=p;
		this->K=K;
		this->mi=1.0 / m;
		this->Z2i=1.0 / (2.0 * Z);
		this->a=0.0;
		this->F=0.0;
		this->S=3;
		this->alpha=alpha;
		this->dt=1.0 / (sampleRate * this->S);
		this->dti=1.0 / this->dt;
		this->x=0;
		this->v=0;
		this->upprev=0;
	}
	u_sample StulovHammer::load(u_sample in){
		for(u_index j=0;j < S;j++){
			u_sample up;
			u_sample v1=0;
			u_sample x1=0;
			up=(x > 0)?pow(x, p):0;
			u_sample dupdt=(up - upprev) * dti;
			for(u_index k=0;k < S;k++){
				F=K * (up + alpha * dupdt);
				if(this->F < 0) this->F=0;
				a=-F * mi;
				v1=v + a * dt;
				x1=x + (v1 - (in + F * Z2i)) * dt;
				u_sample upnew=(x1 > 0)?pow(x1, p):0;
				u_sample dupdtnew=(upnew - upprev) * 0.5 * dti;
				u_sample change=dupdtnew - dupdt;
				dupdt=dupdt + 0.5 * change;
			}
			upprev=up;
			v=v1;
			x=x1;
		}
		return F;
	}
	void StulovHammer::trigger(u_sample v){
		this->v=v;
		this->x=0.0f;
	}
	String StulovHammer::toString()const{
		return "StulovHammer";
	}
}