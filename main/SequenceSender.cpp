//#include "SequenceSender.h"
//#include "interface/IMixer.h"
//#include "MixerSequence.h"
//#include <thread>
//#include <atomic>
//#include <chrono>
//
//namespace yzrilyzr_simplesynth{
//
//	void SequenceSender::workerFunction(){
//		while(!stopRequested.load()){
//			if(!running.load() || paused.load()){
//				std::this_thread::sleep_for(std::chrono::milliseconds(10));
//				continue;
//			}
//
//			auto current=currentSequence;
//			if(current && mixer){
//				// 这里需要根据具体的序列播放逻辑来实现
//				// 假设 MixerSequence 有播放相关的方法
//				current->process(mixer);
//			}
//
//			std::this_thread::sleep_for(std::chrono::milliseconds(1));
//		}
//	}
//
//	SequenceSender::SequenceSender(){}
//
//	SequenceSender::~SequenceSender(){
//		stop();
//	}
//
//	void SequenceSender::addSequence(const yzrilyzr_lang::String & name, std::shared_ptr<MixerSequence> seq){
//		if(seq){
//			sequences[name]=seq;
//		}
//	}
//
//	void SequenceSender::switchSequence(const yzrilyzr_lang::String & name, u_time_ms fadeout, u_time_ms fadein){
//		auto it=sequences.find(name);
//		if(it != sequences.end()){
//			// 先停止当前序列（如果有淡出时间）
//			if(currentSequence && fadeout > 0){
//				// 这里实现淡出逻辑
//				currentSequence->fadeOut(fadeout);
//			}
//
//			// 切换到新序列
//			currentSequence=it->second;
//			currentSequenceName=name;
//
//			// 应用淡入效果（如果有淡入时间）
//			if(fadein > 0){
//				currentSequence->fadeIn(fadein);
//			}
//		}
//	}
//
//	void SequenceSender::setMixer(IMixer * mixer){
//		mixer=mixer;
//	}
//
//	void SequenceSender::start(){
//		if(running.load()){
//			return;
//		}
//
//		running=true;
//		paused=false;
//		stopRequested=false;
//
//		if(!workerThread.joinable()){
//			workerThread=std::thread(&SequenceSender::workerFunction, this);
//		}
//	}
//
//	void SequenceSender::pause(){
//		paused=true;
//	}
//
//	void SequenceSender::stop(){
//		stopRequested=true;
//		running=false;
//		paused=false;
//
//		if(workerThread.joinable()){
//			workerThread.join();
//		}
//
//		// 停止当前序列
//		if(currentSequence){
//			currentSequence->stop();
//		}
//	}
//
//}