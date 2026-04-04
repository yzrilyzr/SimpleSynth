#pragma once
#include "events/NRPNBinding.hpp"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(VVVFKeyData){
	public:
	double pTri;
	double pSin;
	int lastSync=-1;
	};
	/**
	 * sync:
	 * 127 fTri use note freq, fSin manual 音阶异步调制
	 * 126 fTri fSin manual 全手动异步调制
	 * 1-60 fSin manual, fTri sync with fSin (fTri=fSin*sync) 手动同步调制
	 * 61-120 fSin use note freq, fTri sync with fSin (fTri=fSin*(sync-60)) 音阶同步调制
	 */
	ECLASS(VVVF, public NoteProcessor, NoteData<VVVFKeyData>, NRPNBinding){
	private:
	u_freq fTri=400, fSin=10;
	double vSine=0.9;
	int32_t sync=0;
	public:
	VVVF();
	u_sample getAmp(const Note & note) override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	VVVFKeyData * init(VVVFKeyData * data, const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam()override;
	U_CLASS_INFO(VVVF)
	};
}