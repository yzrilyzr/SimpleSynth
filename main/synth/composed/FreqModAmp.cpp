#include "FreqModAmp.h"
#include "lang/StringFormat.hpp"

using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	FreqModAmp::FreqModAmp() :AmpBinaryComposition(nullptr, nullptr){
		registerParamAlias("A", "Carrier");
		registerParamAlias("B", "Mod");
		static u_sample min=0.0, max=100.0;
		registerParam("Depth", ParamType::Gain, &depth, &min, &max);
	}
	FreqModAmp::FreqModAmp(NoteProcPtr dst, NoteProcPtr src, u_sample depth) :depth(depth), AmpBinaryComposition(dst, src){}
	u_sample FreqModAmp::getAmp(Note & note){
		u_sample modFreq=b->getAmp(note) * depth;
		u_freq oldFreq=note.freqSynth;
		u_freq oldPhase=note.phaseSynth;
		FreqModKeyData & data=*getData(note);
		note.freqSynth*=1 + modFreq;
		data.phaseSynth+=note.freqSynth * note.cfg->deltaTime;
		note.phaseSynth=data.phaseSynth;
		u_sample out=a->getAmp(note);
		note.freqSynth=oldFreq;
		note.phaseSynth=oldPhase;
		return out;
	}
	NoteProcPtr FreqModAmp::clone(){
		return std::make_shared<FreqModAmp>(a->clone(), b->clone(), depth);
	}
	std::string FreqModAmp::toString() const{
		return StringFormat::object2string("FreqModAmp", a, b, depth);
	}
	FreqModKeyData * FreqModAmp::init(FreqModKeyData * data, Note & note){
		if(data == nullptr){
			data=new FreqModKeyData();
		}
		data->phaseSynth=0;
		return data;
	}
}