#include "IMixer.h"
#include "IChannel.h"
#include "array/Array.hpp"
#include "dsp/DSP3D.h"

using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	bool IMixer::isDrumSetChannel(s_midichannel_id id){
		return (id % 16) == 9;
	}
	bool IMixer::hasMIDIChannel(s_midichannel_id id){
		return hasMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, id);
	}
	u_sp<IChannel> IMixer::getMIDIChannel(s_midichannel_id id){
		return getMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, id);
	}
	u_sample_rate IMixer::getSampleRate() const{
		return globalChannelConfig.sampleRate;
	}
	void IMixer::setSampleRate(u_sample_rate sr){
		globalChannelConfig.sampleRate=sr;
	}
	u_index IMixer::getOutputChannelCount()const{
		return outputChannelCount;
	}
	int8_t IMixer::getSynthMode() const{
		return synthMode;
	}
	void IMixer::setUseEQ(bool use){
		useEQ=use;
	}
	bool IMixer::isUseEQ() const{
		return useEQ;
	}
	void IMixer::setUseLimiter(bool use){
		useLimiter=use;
	}
	bool IMixer::isUseLimiter() const{
		return useLimiter;
	}
	void IMixer::setChannelUseDSP(bool use){
		channelUseDSP=use;
	}
	bool IMixer::isChannelUseDSP() const{
		return channelUseDSP;
	}
	ChannelConfig & IMixer::getGlobalConfig(){
		return globalChannelConfig;
	}
	u_time_f IMixer::getProcessStandardTime() const{
		return (u_time_f)getBufferSize() / getSampleRate();
	}
	u_time_f IMixer::getProcessTime()const{
		return processTime;
	}
	String IMixer::toString()const{
		return "IMixer";
	}
	std::shared_mutex & IMixer::getDSPLock(){
		return dspLock;
	}
	std::shared_mutex & IMixer::getEventLock(){
		return eventLock;
	}
	std::shared_mutex & IMixer::getChannelLock(){
		return channelLock;
	}
	void IMixer::asyncMix(){
		mixFuture=std::async(std::launch::async, [this](){ this->mix(); });
	}
	void IMixer::awaitMix(){
		if(mixFuture.has_value()){
			mixFuture->get();
			mixFuture.reset();
		}
	}
}