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
namespace yzrilyzr_simplesynth{
	ECLASS(_NotePhase, public PhaseSrc){
	public:
	inline s_phase getPhase(Note & note) override final{
		return note.phaseSynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NotePhase";
	}
	};
	ECLASS(_MulPhase, public PhaseSrc){
	private:
	std::shared_ptr<PhaseSrc> src;
	u_sample r;
	public:
	_MulPhase() : src(nullptr), r(0){
		static u_sample min=-100, max=100;
		registerParamPhaseSrc("Src", &src);
		registerParam("Multiply", yzrilyzr_util::ParamType::Sample, &r, &min, &max);
	}
	_MulPhase(std::shared_ptr<PhaseSrc> src, u_sample r) : src(src), r(r){}
	void init()override{
		if(src == nullptr)throw yzrilyzr_lang::NullPointerException("src == null");
		src->init();
	}
	inline s_phase getPhase(Note & note) override final{
		return src->getPhase(note) * r;
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("MulPhase(%s, %f)", src, r);
	}
	};
	ECLASS(_AddPhase, public PhaseSrc){
	private:
	std::shared_ptr<PhaseSrc> a=nullptr;
	std::shared_ptr<PhaseSrc> b=nullptr;
	public:
	_AddPhase(){
		static u_sample min=-100, max=100;
		registerParamPhaseSrc("A", &a);
		registerParamPhaseSrc("B", &b);
	}
	_AddPhase(std::shared_ptr<PhaseSrc> a, std::shared_ptr<PhaseSrc> b) : a(a), b(b){}
	void init()override{
		if(a == nullptr)throw yzrilyzr_lang::NullPointerException("a == null");
		if(b == nullptr)throw yzrilyzr_lang::NullPointerException("b == null");
		a->init();
		b->init();
	}
	inline s_phase getPhase(Note & note) override final{
		return a->getPhase(note) + b->getPhase(note);
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("AddPhase(%s, %s)", a, b);
	}
	};
	ECLASS(_ConstPhase, public PhaseSrc){
	private:
	u_freq hz;
	public:
	_ConstPhase() : hz(1000){
		registerParamFreq("Freq", &hz);
	}
	_ConstPhase(u_freq hz) : hz(hz){}
	inline s_phase getPhase(Note & note) override final{
		return hz * note.passedTime;
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("ConstPhase(%f)", hz);
	}
	};
	ECLASS(_ConstAmp, public NoteProcessor){
	public:
	u_sample value;
	_ConstAmp() : value(1){
		static u_sample min=-100, max=100;
		registerParam("Value", yzrilyzr_util::ParamType::Sample, &value, &min, &max);
	}
	_ConstAmp(u_sample value) : value(value){}
	inline u_sample getAmp(Note & note) override final{
		return value;
	}
	yzrilyzr_lang::String toString()const override{
		return  yzrilyzr_lang::StringFormat::format("ConstAmp(%f)", value);
	}
	};
	ECLASS(_NoteIDAmp, public NoteProcessor){
	public:
	inline u_sample getAmp(Note & note) override final{
		return note.idSynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NoteIDAmp";
	}
	};
	ECLASS(_NoteFreqAmp, public NoteProcessor){
	public:
	inline u_sample getAmp(Note & note) override final{
		return note.freqSynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NoteFreqAmp";
	}
	};
	ECLASS(_NoteVelAmp, public NoteProcessor){
	public:
	inline u_sample getAmp(Note & note) override final{
		return note.velocitySynth;
	}
	yzrilyzr_lang::String toString() const override{
		return "NoteVelAmp";
	}
	};
#ifndef NotePhase
#define NotePhase std::make_shared<_NotePhase>()
#endif
#ifndef NoteIDAmp
#define NoteIDAmp std::make_shared<_NoteIDAmp>()
#endif
#ifndef NoteVelAmp
#define NoteVelAmp std::make_shared<_NoteVelAmp>()
#endif
#ifndef NoteFreqAmp
#define NoteFreqAmp std::make_shared<_NoteFreqAmp>()
#endif
	static std::shared_ptr<PhaseSrc> MulPhase(std::shared_ptr<PhaseSrc> freq, u_sample r){
		return std::make_shared<_MulPhase>(freq, r);
	}
	static std::shared_ptr<PhaseSrc> AddPhase(std::shared_ptr<PhaseSrc> a, std::shared_ptr<PhaseSrc> b){
		return std::make_shared<_AddPhase>(a, b);
	}
	static std::shared_ptr<PhaseSrc> ConstPhase(u_freq hz){
		return std::make_shared<_ConstPhase>(hz);
	}
	static NoteProcPtr ConstAmp(u_sample value){
		return std::make_shared<_ConstAmp>(value);
	}
	static NoteProcPtr SineAmp(u_freq hz){
		return std::make_shared<SineWave>(ConstPhase(hz));
	}
	static NoteProcPtr SineW(){
		return std::make_shared<SineWave>();
	}
	static NoteProcPtr SawW(){
		return std::make_shared<SawWave>();
	}
	static NoteProcPtr SquareW(){
		return std::make_shared<SquareWave>();
	}
	static NoteProcPtr TriW(){
		return std::make_shared<TriWave>();
	}

	template<typename T>
	std::shared_ptr<AmpAdder> operator+(std::shared_ptr<T> a, std::shared_ptr<T> b){
		return std::make_shared< yzrilyzr_simplesynth::AmpAdder>(a, b);
	}
	template<typename T>
	std::shared_ptr<AmpAdder> operator+(std::shared_ptr<T> a, NoteProcPtr b){
		return std::make_shared< yzrilyzr_simplesynth::AmpAdder>(a, b);
	}
	template<typename T>
	std::shared_ptr<AmpAdder> operator+(std::shared_ptr<T> a, u_sample b){
		return std::make_shared< yzrilyzr_simplesynth::AmpAdder>(a, ConstAmp(b));
	}
	template<typename T>
	std::shared_ptr<AmpMultiplier> operator*(std::shared_ptr<T> a, std::shared_ptr<T> b){
		return std::make_shared< yzrilyzr_simplesynth::AmpMultiplier>(a, b);
	}
	template<typename T>
	std::shared_ptr<AmpMultiplier> operator*(std::shared_ptr<T> a, u_sample b){
		return std::make_shared< yzrilyzr_simplesynth::AmpMultiplier>(a, ConstAmp(b));
	}
}