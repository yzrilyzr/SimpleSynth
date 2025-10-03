//------------------------------------------------------------------------
// Copyright(c) 2025 yzrilyzr.
//------------------------------------------------------------------------

#include "cids.h"
#include "processor.h"
#include "Mixer2.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "events/ChannelEvent.h"
#include "controller.h"
#include "util/MIDIFile.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth_vst{
//------------------------------------------------------------------------
// SimpleSynthProcessor
//------------------------------------------------------------------------
	SimpleSynthProcessor::SimpleSynthProcessor(){
		//--- set the wanted controller for our processor
		setControllerClass(kSimpleSynthControllerUID);
		mixer=std::make_shared<Mixer2>(256); // 初始化合成器
		mixer->setSampleRate(48000);
	}

	//------------------------------------------------------------------------
	SimpleSynthProcessor::~SimpleSynthProcessor(){}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::initialize(FUnknown * context){
		tresult result=AudioEffect::initialize(context);
		if(result != kResultOk){
			return result;
		}
		addAudioOutput(STR16("MixerOut"), Steinberg::Vst::SpeakerArr::kStereo);
		addEventInput(STR16("MIDIEventIn"), 1);
		return kResultOk;
	}
	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::terminate(){
		// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
		mixer->reset();
		//---do not forget to call parent ------
		return AudioEffect::terminate();
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::setActive(TBool state){
		//--- called when the Plug-in is enable/disable (On/Off) -----
		tresult result=AudioEffect::setActive(state);
		if(result != kResultOk){
			return result;
		}
		if(!state){
			for(auto & p : mixer->getAllChannels()){
				mixer->sendInstantEvent(new ChannelControl(chID, MIDIFile::CC::ALL_NOTES_OFF, 127));
			}
		}
		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::process(Vst::ProcessData & data){
		//--- First : Read inputs parameter changes-----------

		if(data.inputParameterChanges){
			int32 numParamsChanged=data.inputParameterChanges->getParameterCount();
			for(int32 index=0; index < numParamsChanged; index++){
				if(auto * paramQueue=data.inputParameterChanges->getParameterData(index)){
					Vst::ParamValue value;
					int32 sampleOffset;
					int32 numPoints=paramQueue->getPointCount();
					if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultOk){
						switch(paramQueue->getParameterId()){
							case kParamSustain:
							{
								mixer->sendInstantEvent(new ChannelControl(chID, MIDIFile::CC::SUSTAIN_SWITCH, value > 0.5?127:0));
								break;
							}
							case kParamIsDrum:
							{
								chID=value > 0.5?9:0;
								break;
							}
							case kParamLimiter:
							{
								mixer->setUseLimiter(value > 0.5);
								break;
							}
							case kParamProgram:
							{
								mixer->sendInstantEvent(new ProgramChange(chID, static_cast<s_program_id>(std::round(value * 127.0))));
								break;
							}
							case kParamExpression:
							{
								mixer->sendInstantEvent(new ChannelControl(chID, MIDIFile::CC::EXPRESSION, static_cast<uint8_t>(value * 127.0)));
								break;
							}
							case kParamVolume:
							{
								mixer->sendInstantEvent(new ChannelControl(chID, MIDIFile::CC::VOLUME, static_cast<uint8_t>(value * 127.0)));
								break;
							}
							
						}
					}
				}
			}
		}
		if(data.inputEvents){
			for(int i=0; i < data.inputEvents->getEventCount(); ++i){
				Vst::Event event;
				if(data.inputEvents->getEvent(i, event) == kResultOk){
					if(event.type == Steinberg::Vst::Event::EventTypes::kNoteOnEvent){
						NoteOnEvent & e=event.noteOn;
						mixer->sendInstantEvent(new NoteOn(chID, e.pitch, e.velocity));
					} else if(event.type == Steinberg::Vst::Event::EventTypes::kNoteOffEvent){
						NoteOffEvent & e=event.noteOff;
						mixer->sendInstantEvent(new NoteOff(chID, e.pitch, e.velocity));
					} else if(event.type == Steinberg::Vst::Event::EventTypes::kPolyPressureEvent){
						PolyPressureEvent & e=event.polyPressure;
						mixer->sendInstantEvent(new NotePressure(chID, e.pitch, e.pressure));
					}
				}
			}
		}
		auto cbkSamples=data.numSamples;
		if(cbkSamples > 0 && data.numOutputs > 0){
			auto & bus=data.outputs[0];
			auto numChannels=bus.numChannels;
			auto sampleRate=mixer->getSampleRate();
			if(numChannels > 0 && numChannels <= mixer->getOutputChannelCount()){
				u_index bufLen=mixer->getBufferSize();
				if(!mixer->hasData()){
					bus.silenceFlags=0b11;
					if(data.symbolicSampleSize == kSample32){
						for(Steinberg::int32 ch=0;ch < numChannels;ch++){
							memset(bus.channelBuffers32[ch], 0, 4 * cbkSamples);
						}
					} else if(data.symbolicSampleSize == kSample64){
						for(Steinberg::int32 ch=0;ch < numChannels;ch++){
							memset(bus.channelBuffers64[ch], 0, 8 * cbkSamples);
						}
					}
					return kResultOk;
				}
				bus.silenceFlags=0;
				while(fifoBuffer[0].available() < cbkSamples){
					mixer->mix();
					for(Steinberg::int32 ch=0;ch < numChannels;ch++){
						fifoBuffer[ch].write(mixer->getOutput(ch), 0, bufLen);
					}
				}
				int size=sizeof(u_sample);
			#ifdef DSP_SINGLE_PRECISION
				if(data.symbolicSampleSize == kSample32){
					for(Steinberg::int32 ch=0;ch < numChannels;ch++){
						auto out=bus.channelBuffers32[ch];
						fifoBuffer[ch].read(out, 0, cbkSamples);
						fifoBuffer[ch].compact();
					}
				}else if(data.symbolicSampleSize == kSample64){
					u_sample * tmp=new u_sample[cbkSamples];
					for(Steinberg::int32 ch=0;ch < numChannels;ch++){
						auto out=bus.channelBuffers64[ch];
						fifoBuffer[ch].read(tmp, 0, cbkSamples);
						fifoBuffer[ch].compact();
						for(Steinberg::int32 i=0;i < cbkSamples;i++){
							out[i]=(Sample64)tmp[i];
						}
					}
					delete[] tmp;
				}
			#endif
			#ifdef DSP_DOUBLE_PRECISION
				if(data.symbolicSampleSize == kSample32){
					u_sample * tmp=new u_sample[cbkSamples];
					for(Steinberg::int32 ch=0;ch < numChannels;ch++){
						auto out=bus.channelBuffers32[ch];
						fifoBuffer[ch].read(tmp, 0, cbkSamples);
						fifoBuffer[ch].compact();
						for(Steinberg::int32 i=0;i < cbkSamples;i++){
							out[i]=(Sample32)tmp[i];
						}
					}
					delete[] tmp;
				}else if(data.symbolicSampleSize == kSample64){
					for(Steinberg::int32 ch=0;ch < numChannels;ch++){
						auto out=bus.channelBuffers64[ch];
						fifoBuffer[ch].read(out, 0, cbkSamples);
						fifoBuffer[ch].compact();
					}
				}
			#endif // DSP_DOUBLE_PRECISION				
			}
		}

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::setupProcessing(Vst::ProcessSetup & newSetup){
		Steinberg::tresult result=AudioEffect::setupProcessing(newSetup);
		if(result != Steinberg::kResultOk) return result;
		mixer->setSampleRate(newSetup.sampleRate);
		mixer->reset();
		return Steinberg::kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::canProcessSampleSize(int32 symbolicSampleSize){
		if(symbolicSampleSize == Vst::kSample32)
			return kResultTrue;
		if(symbolicSampleSize == Vst::kSample64)
			return kResultTrue;
		return kResultFalse;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::setState(IBStream * state){
		// called when we load a preset, the model has to be reloaded
		IBStreamer streamer(state, kLittleEndian);
		mixer->reset();
		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthProcessor::getState(IBStream * state){
		// here we need to save the model
		IBStreamer streamer(state, kLittleEndian);

		return kResultOk;
	}
	//------------------------------------------------------------------------
} // namespace yzrilyzr_simplesynth_vst