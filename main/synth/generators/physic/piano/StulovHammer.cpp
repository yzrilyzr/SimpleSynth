#include "StulovHammer.h"

namespace yzrilyzr_simplesynth{
	void StulovHammer::init(double sampleRate, double m, double K, double p, double Z, double alpha){
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
	double StulovHammer::load(double in){
		for(int j=0;j < S;j++){
			double up;
			double v1=0;
			double x1=0;
			up=(x > 0)?pow(x, p):0;
			double dupdt=(up - upprev) * dti;
			for(int k=0;k < S;k++){
				F=K * (up + alpha * dupdt);
				if(this->F < 0) this->F=0;
				a=-F * mi;
				v1=v + a * dt;
				x1=x + (v1 - (in + F * Z2i)) * dt;
				double upnew=(x1 > 0)?pow(x1, p):0;
				double dupdtnew=(upnew - upprev) * 0.5 * dti;
				double change=dupdtnew - dupdt;
				dupdt=dupdt + 0.5 * change;
			}
			upprev=up;
			v=v1;
			x=x1;
		}
		return F;
	}
	void StulovHammer::trigger(double v){
		this->v=v;
		this->x=0.0f;
	}
	std::string StulovHammer::toString()const{
		return "StulovHammer";
	}
}