#pragma once
#include "interface/PhaseSrc.h"
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "synth/generators/sine/SineWave.h"
#include "synth/generators/pulse/SawWave.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/pulse/SquareWave.h"
#include "synth/composed/AmpAdder.h"
#include "synth/composed/AmpMultiplier.h"
#include "lang/StringFormat.hpp"
#include "lang/Exception.h"
#include "dsp/DSP.h"
namespace yzrilyzr_simplesynth{
	ECLASS(_NotePhase, public PhaseSrc){
	public:
	inline s_phase getPhase(const Note & note) override final{
		return note.phaseSynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NotePhase";
	}
	U_GET_CLASS_NAME(NotePhase)
	};
	ECLASS(_MulPhase, public PhaseSrc){
	private:
	u_sp<PhaseSrc> src;
	u_sample r;
	public:
	_MulPhase() : src(nullptr), r(0){}
	_MulPhase(u_sp<PhaseSrc> src, u_sample r) : src(src), r(r){}
	void init()override{
		if(src == nullptr)throw yzrilyzr_lang::NullPointerException("src == null");
		src->init();
	}
	inline s_phase getPhase(const Note & note) override final{
		return src->getPhase(note) * r;
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("MulPhase(%s, %f)", src, r);
	}
	void onRegisterParam() override{
		static u_sample min=-100, max=100;
		yzrilyzr_util::RegisterUtil::registerParamPhaseSrc(*this, "Src", &src);
		registerParam("Multiply", yzrilyzr_util::ParamType::Sample, &r, &min, &max);
	}
	U_GET_CLASS_NAME(MulPhase)
	};
	ECLASS(_AddPhase, public PhaseSrc){
	private:
	u_sp<PhaseSrc> a=nullptr;
	u_sp<PhaseSrc> b=nullptr;
	public:
	_AddPhase(){}
	_AddPhase(u_sp<PhaseSrc> a, u_sp<PhaseSrc> b) : a(a), b(b){}
	void init()override{
		if(a == nullptr)throw yzrilyzr_lang::NullPointerException("a == null");
		if(b == nullptr)throw yzrilyzr_lang::NullPointerException("b == null");
		a->init();
		b->init();
	}
	inline s_phase getPhase(const Note & note) override final{
		return a->getPhase(note) + b->getPhase(note);
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("AddPhase(%s, %s)", a, b);
	}
	void onRegisterParam() override{
		static u_sample min=-100, max=100;
		yzrilyzr_util::RegisterUtil::registerParamPhaseSrc(*this, "A", &a);
		yzrilyzr_util::RegisterUtil::registerParamPhaseSrc(*this, "B", &b);
	}
	U_GET_CLASS_NAME(AddPhase)
	};
	ECLASS(_ConstPhase, public PhaseSrc){
	private:
	u_freq hz;
	public:
	_ConstPhase() : hz(1000){}
	_ConstPhase(u_freq hz) : hz(hz){}
	inline s_phase getPhase(const Note & note) override final{
		return hz * note.passedTime;
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("ConstPhase(%f)", hz);
	}
	void onRegisterParam() override{
		yzrilyzr_util::RegisterUtil::registerParamFreq(*this, "Freq", &hz);
	}
	U_GET_CLASS_NAME(ConstPhase)
	};
	ECLASS(_ConstAmp, public NoteProcessor){
	public:
	u_sample value;
	_ConstAmp() : value(1){}
	_ConstAmp(u_sample value) : value(value){}
	inline u_sample getAmp(const Note & note) override final{
		return value;
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("ConstAmp(%f)", value);
	}
	void onRegisterParam() override{
		static u_sample min=-100, max=100;
		registerParam("Value", yzrilyzr_util::ParamType::Sample, &value, &min, &max);
	}
	U_GET_CLASS_NAME(ConstAmp)
	};
	ECLASS(_NoteIDAmp, public NoteProcessor){
	public:
	inline u_sample getAmp(const Note & note) override final{
		return note.idSynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NoteIDAmp";
	}
	U_GET_CLASS_NAME(NoteIDAmp)
	};
	ECLASS(_NoteFreqAmp, public NoteProcessor){
	public:
	inline u_sample getAmp(const Note & note) override final{
		return note.freqSynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NoteFreqAmp";
	}
	U_GET_CLASS_NAME(NoteFreqAmp)
	};
	ECLASS(_NoteVelAmp, public NoteProcessor){
	public:
	inline u_sample getAmp(const Note & note) override final{
		return note.velocitySynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NoteVelAmp";
	}
	U_GET_CLASS_NAME(NoteVelAmp)
	};
#ifndef NotePhase
#define NotePhase mksp<_NotePhase>()
#endif
#ifndef NoteIDAmp
#define NoteIDAmp mksp<_NoteIDAmp>()
#endif
#ifndef NoteVelAmp
#define NoteVelAmp mksp<_NoteVelAmp>()
#endif
#ifndef NoteFreqAmp
#define NoteFreqAmp mksp<_NoteFreqAmp>()
#endif
	static u_sp<PhaseSrc> MulPhase(u_sp<PhaseSrc> freq, u_sample r){
		return mksp<_MulPhase>(freq, r);
	}
	static u_sp<PhaseSrc> AddPhase(u_sp<PhaseSrc> a, u_sp<PhaseSrc> b){
		return mksp<_AddPhase>(a, b);
	}
	static u_sp<PhaseSrc> ConstPhase(u_freq hz){
		return mksp<_ConstPhase>(hz);
	}
	static NoteProcPtr ConstAmp(u_sample value){
		return mksp<_ConstAmp>(value);
	}
	static NoteProcPtr SineAmp(u_freq hz){
		return mksp<SineWave>(ConstPhase(hz));
	}
	static NoteProcPtr SineW(){
		return mksp<SineWave>();
	}
	static NoteProcPtr SawW(){
		return mksp<SawWave>();
	}
	static NoteProcPtr SquareW(){
		return mksp<SquareWave>();
	}
	static NoteProcPtr TriW(){
		return mksp<TriWave>();
	}

	template<typename T>
	U_EXPORT_API u_sp<AmpAdder> operator+(u_sp<T> a, u_sp<T> b){
		return mksp< yzrilyzr_simplesynth::AmpAdder>(a, b);
	}
	template<typename T>
	U_EXPORT_API u_sp<AmpAdder> operator+(u_sp<T> a, NoteProcPtr b){
		return mksp< yzrilyzr_simplesynth::AmpAdder>(a, b);
	}
	template<typename T>
	U_EXPORT_API u_sp<AmpAdder> operator+(u_sp<T> a, u_sample b){
		return mksp< yzrilyzr_simplesynth::AmpAdder>(a, ConstAmp(b));
	}
	template<typename T>
	U_EXPORT_API u_sp<AmpMultiplier> operator*(u_sp<T> a, u_sp<T> b){
		return mksp< yzrilyzr_simplesynth::AmpMultiplier>(a, b);
	}
	template<typename T>
	U_EXPORT_API u_sp<AmpMultiplier> operator*(u_sp<T> a, u_sample b){
		return mksp< yzrilyzr_simplesynth::AmpMultiplier>(a, ConstAmp(b));
	}
}