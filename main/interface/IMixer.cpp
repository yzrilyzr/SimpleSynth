#include "IMixer.h"
#include "IChannel.h"
namespace yzrilyzr_simplesynth{
	bool IMixer::hasMIDIChannel(s_midichannel_id id){
		return hasMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, id);
	}
	std::shared_ptr<IChannel> IMixer::getMIDIChannel(s_midichannel_id id){
		return getMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, id);
	}
	u_sample_rate IMixer::getSampleRate() const{
		return sampleRate;
	}
	void IMixer::setSampleRate(u_sample_rate sr){
		sampleRate=sr;
	}
	size_t IMixer::getOutputChannelCount()const{
		return outputChannelCount;
	}
	int8_t IMixer::getSynthMode() const{
		return synthMode;
	}
	void IMixer::setInstrumentProvider(std::shared_ptr<InstrumentProvider> instr){
		instrument=instr;
	}
	std::shared_ptr<InstrumentProvider> IMixer::getInstrumentProvider()const{
		return instrument;
	}
	std::shared_ptr<NoteTuning> IMixer::getNoteTuning()const{
		return tuning;
	}
	void IMixer::setNoteTuning(std::shared_ptr<NoteTuning> tun){
		tuning=tun;
	}
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> IMixer::getNoteVelocityMap()const{
		return velocityMap;
	}
	void IMixer::setNoteVelocityMap(std::shared_ptr<yzrilyzr_interpolator::Interpolator> val){
		velocityMap=val;
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

	u_time_f IMixer::getProcessStandardTime() const{
		return (u_time_f)getBufferSize() / getSampleRate();
	}
    u_time_f IMixer::getProcessTime()const{
        return processTime;
    }
	std::string IMixer::toString()const{
		return "IMixer";
	}
	std::mutex & IMixer::getDSPLock(){
		return dspLock;
	}
	std::mutex & IMixer::getEventLock(){
		return eventLock;
	}
	std::mutex & IMixer::getChannelLock(){
		return channelLock;
	}
}