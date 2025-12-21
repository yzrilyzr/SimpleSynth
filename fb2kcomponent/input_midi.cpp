#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <mmsystem.h>        
#include <objbase.h>        
#include "SDK/foobar2000.h"
#include "yzrutil.h"
#include "SimpleSynth.h"
#include "Mixer2.h"
#include "SynthUtil.h"
#include "instrument/SimpleMIDIInstrument.h"
#include "io/ByteArrayInputStream.h"

enum{
	mixer_buffer_size=1024,
	mixer_sample_rate=48000,
	mixer_output_channels=2,
	mixer_bits_per_sample=32
};

static const char * supported_extensions[]={"mid", "midi", "rmi", "xm"};

class input_midi : public input_stubs{
	private:
	u_sp<yzrilyzr_simplesynth::Mixer2> m_mixer=nullptr;
	u_sp<yzrilyzr_simplesynth::MixerSequence> m_sequence=nullptr;
	u_time m_total_length=0;
	void initMixer(){
		if(!m_mixer){
		//初始化合成器
			m_mixer=mksp<yzrilyzr_simplesynth::Mixer2>(mixer_buffer_size);
			m_mixer->setSampleRate(mixer_sample_rate);
			m_mixer->setSynthMode(yzrilyzr_simplesynth::IMixer::MODE_THREAD_POOL, -1);
			m_mixer->setUseLimiter(true);
			// 设置默认乐器
			u_sp<yzrilyzr_simplesynth::SimpleMIDIInstrument> simple=mksp<yzrilyzr_simplesynth::SimpleMIDIInstrument>();
			m_mixer->getGlobalConfig().setInstrumentProvider(simple);
		}
	}
	public:
	void open(service_ptr_t<file> p_filehint, const char * p_path, t_input_open_reason p_reason, abort_callback & p_abort){
		if(p_reason == input_open_info_write) throw exception_tagging_unsupported();//our input does not support retagging.
		m_file=p_filehint;//p_filehint may be null, hence next line
		input_open_file_helper(m_file, p_path, p_reason, p_abort);
		t_filesize file_size=m_file->get_size(p_abort);
		if(file_size == filesize_invalid){
			throw exception_io_data();
		}

		initMixer();

		//读取文件
		yzrilyzr_array::ByteArray array((t_size)file_size);
		m_file->read(array._array, (t_size)array.length, p_abort);
		// 使用 MIDI 文件解析器
		try{
			yzrilyzr_io::ByteArrayInputStream bi(array);
			yzrilyzr_lang::String path(p_path);
			path=path.toLowerCase();
			// 创建混音序列
			if(path.contains("mid")){
				m_sequence=yzrilyzr_simplesynth::SynthUtil::parseMIDI(bi);
			} else if(path.contains("xm")){
				m_sequence=yzrilyzr_simplesynth::SynthUtil::parseXM(bi);
			}
			if(!m_sequence){
				throw exception_io_data();
			}
			// 计算总长度
			m_total_length=m_sequence->getDuration();
		} catch(...){
			throw exception_io_data();
		}
	}

	void get_info(file_info & p_info, abort_callback & p_abort){
		t_filesize size=m_file->get_size(p_abort);
		if(size != filesize_invalid){
			//file size is known, let's set length
			p_info.set_length(m_total_length);
		}
		//note that the values below should be based on contents of the file itself, NOT on user-configurable variables for an example. To report info that changes independently from file contents, use get_dynamic_info/get_dynamic_info_track instead.
		p_info.info_set_int("samplerate", mixer_sample_rate);
		p_info.info_set_int("channels", mixer_output_channels);
		p_info.info_set_int("bitspersample", mixer_bits_per_sample);
		p_info.info_set("codec", "SimpleSynth");
		p_info.info_set("channel_layout", "Mixed Stereo");

		// Indicate whether this is a fixedpoint or floatingpoint stream, when using bps >= 32
		// As 32bit fixedpoint can't be decoded losslessly by fb2k, does not fit in float32 audio_sample.
		if(mixer_bits_per_sample >= 32) p_info.info_set("bitspersample_extra", "floating-point");

		p_info.info_set("encoding", "lossless");
		//设置合成器输出比特率
		p_info.info_set_bitrate((mixer_bits_per_sample * mixer_output_channels * mixer_sample_rate + 500 /* rounding for bps to kbps*/) / 1000 /* bps to kbps */);

	}
	t_filestats2 get_stats2(unsigned f, abort_callback & a){ return m_file->get_stats2_(f, a); }
	t_filestats get_file_stats(abort_callback & p_abort){ return m_file->get_stats(p_abort); }

	void decode_initialize(unsigned p_flags, abort_callback & p_abort){
		m_file->reopen(p_abort);
		if(m_mixer){
			m_mixer->reset();
			if(m_sequence){
				m_sequence->postToMixer(m_mixer.get(), 0, "foobar2000_decode");
			}
		}
	}
	bool decode_run(audio_chunk & p_chunk, abort_callback & p_abort){
		if(!m_mixer || !m_mixer->hasData()){
			return false;
		}

		m_mixer->mix();

		u_index output_channels=m_mixer->getOutputChannelCount();
		u_sample_rate sampleRate=m_mixer->getSampleRate();
		const t_size samples_to_process=m_mixer->getBufferSize();

		// 准备交错数据缓冲区
		pfc::array_t<audio_sample> interleaved_data;
		interleaved_data.set_size(samples_to_process * output_channels);

		// 将平面数据转换为交错数据
		for(t_size sample=0; sample < samples_to_process; sample++){
			for(u_index ch=0; ch < output_channels; ch++){
				float sample_value=static_cast<float>(m_mixer->getOutput(ch)[sample]);
				interleaved_data[sample * output_channels + ch]=sample_value;
			}
		}

		// 使用 set_data 一次性设置所有参数
		p_chunk.set_data(
			interleaved_data.get_ptr(),
			samples_to_process,
			output_channels,
			sampleRate
		);

		return true;
	}
	void decode_seek(double p_seconds, abort_callback & p_abort){
		if(!m_mixer || !m_sequence)return;
		m_mixer->reset();
		m_sequence->postToMixer(m_mixer.get(), 0, p_seconds, "foobar2000_decode");
	}
	bool decode_can_seek(){ return true; }
	// deals with dynamic information such as VBR bitrates
	bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta){
		if(m_mixer){
			p_out.info_set_int("processing", m_mixer->getCurrentProcessingNoteCount());
			p_out.info_set_int("post", m_mixer->getPostedEventCount());
			p_out.info_set("synthload", (yzrilyzr_lang::String((int)(100.0 * m_mixer->getProcessTime() / m_mixer->getProcessStandardTime())) + yzrilyzr_lang::String(" %")).c_str());
		}
		p_timestamp_delta=0.1; // 更新间隔
		return true;
	}
	// deals with dynamic information such as track changes in live streams
	bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta){
		/*if(m_mixer){
			auto &allCh=m_mixer->getAllChannels();
			p_out.info_set_channels(allCh.size());
			return true;
		}*/
		return false;
	}
	void decode_on_idle(abort_callback & p_abort){ m_file->on_idle(p_abort); }

	// Note that open() already rejects requests to open for tag writing, so these two should never get called.
	void retag(const file_info & p_info, abort_callback & p_abort){ throw exception_tagging_unsupported(); }
	void remove_tags(abort_callback &){ throw exception_tagging_unsupported(); }

	static bool g_is_our_content_type(const char * p_content_type){ return false; } // match against supported mime types here
	static bool g_is_our_path(const char * p_path, const char * p_extension){
		for(const char * ext : supported_extensions){
			if(yzrilyzr_lang::String(p_extension).toLowerCase().contains(yzrilyzr_lang::String(ext))){
				return true;
			}
		}
		return false;
	}
	static const char * g_get_name(){ return "SimpleSynth"; }
	static const GUID g_get_guid(){
		// GUID of the decoder. Replace with your own when reusing code.
		static const GUID guid={0xd9c01c8d, 0x69c5, 0x4eec, {0x12, 0x53, 0x8d, 0xa4, 0xef, 0x6c, 0x9e, 0x5d}};
		return guid;
	}
	public:
	service_ptr_t<file> m_file;
};

static input_singletrack_factory_t<input_midi> g_input_midi_factory;

// Declare .RAW as a supported file type to make it show in "open file" dialog etc.
DECLARE_FILE_TYPE("MIDI files", "*.MID;*.MIDI;*.RMI");
DECLARE_FILE_TYPE("External Module files", "*.XM");
