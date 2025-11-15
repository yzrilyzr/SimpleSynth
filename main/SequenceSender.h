#pragma once

namespace yzrilyzr_simplesynth{
	class IMixer;
	class MixerSequence;
	
	EBCLASS(SequenceSender){
		private:
		struct SequenceData{
			yzrilyzr_lang::String name;
			std::shared_ptr<MixerSequence> seq;
			u_time_ms fadeInCurrentTime;
			u_time_ms fadeOutCurrentTime;
		};
		std::unordered_map<yzrilyzr_lang::String, SequenceData > sequences;
		SequenceData currentSequence;

		std::thread workerThread;
		std::atomic<bool> running{false};
		std::atomic<bool> paused{false};
		std::atomic<bool> stopRequested{false};

		IMixer * mixer=nullptr;
		void workerFunction();

		public:
		SequenceSender();
		~SequenceSender();
		//添加序列
		void addSequence(const yzrilyzr_lang::String & name, std::shared_ptr<MixerSequence> seq);
		//拖动进度条
		void seek(u_time_ms pos);

		//切换当前序列
		void switchSequence(const yzrilyzr_lang::String & name, u_time_ms fadeout, u_time_ms fadein);
		void setMixer(IMixer * mixer);
		//开始发送器内置的线程
		void start();
		void pause();
		void stop();
	};
}