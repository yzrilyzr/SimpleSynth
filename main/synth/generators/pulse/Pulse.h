#pragma once
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"
#include "interface/PhaseSrc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(Pulse, public Osc){
	private:
	u_normal_01 width;
	u_normal_01 rise;
	u_normal_01 fall;
	u_normal_01 delay;
	public:
	Pulse() : Pulse(.3f, 0.2f, 0.2f, 0){
		registerParamNormal01("Width", &width);
		registerParamNormal01("Rise", &rise);
		registerParamNormal01("Fall", &fall);
		registerParamNormal01("Delay", &delay);
	}
	Pulse(u_normal_01 width, u_normal_01 rise, u_normal_01 fall, u_normal_01 delay) : Pulse(nullptr, width, rise, fall, delay){}
	Pulse(std::shared_ptr<PhaseSrc> freqSrc, u_normal_01 width, u_normal_01 rise, u_normal_01 fall, u_normal_01 delay) : Osc(freqSrc){
		this->width=width;
		this->rise=rise;
		this->fall=fall;
		this->delay=delay;
	}
	u_sample getAmp(Note & note) override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	NoteProcPtr clone() override;
	std::string toString() const override;
	};
}