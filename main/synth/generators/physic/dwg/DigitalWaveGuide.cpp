#include "DigitalWaveGuide.h"
#include "dsp/BufferDelayer.h"
#include "dsp/DSP.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	DigitalWaveGuide::DigitalWaveGuide(){}
	DigitalWaveGuide::DigitalWaveGuide(double z, double delayLen1, double delayLen2){
		init(z, delayLen1, delayLen2);
	}
	void DigitalWaveGuide::init(double z, double delayLen1, double delayLen2){
		// 初始化延迟缓冲区（大小为延迟长度，使用RingBufferSample）
		delayLength1=delayLen1;
		delayLength2=delayLen2;
		delayBuffers[0].ensureCapacity((double)delayLen1);
		delayBuffers[1].ensureCapacity((double)delayLen2);
		initNode(z);
	}
	DigitalWaveGuide::~DigitalWaveGuide(){}
	void DigitalWaveGuide::initNode(double z){
		leftNode.initialize(z);
		rightNode.initialize(z);
	}
	/**
	 * d1.r - l.d2（左侧节点与右侧节点连接）
	 */
	void DigitalWaveGuide::connectLeftRight(DigitalWaveGuide & d1, DigitalWaveGuide & d2){
		d1.connectRight(d2.leftNode);
		d2.connectLeft(d1.rightNode);
	}
	void DigitalWaveGuide::connectRight(DWGNode & r){
		connectRightEx(r, false);
	}
	void DigitalWaveGuide::connectLeft(DWGNode & l){
		connectLeftEx(l, false);
	}
	void DigitalWaveGuide::connectRightEx(DWGNode & r, bool polarity){
		rightConnectedNodes[rightNodeCount]=&r;
		rightNodePolarities[rightNodeCount]=polarity;
		rightNodeCount++;
	}
	void DigitalWaveGuide::connectLeftEx(DWGNode & l, bool polarity){
		leftConnectedNodes[leftNodeCount]=&l;
		leftNodePolarities[leftNodeCount]=polarity;
		leftNodeCount++;
	}
	/**
	 * 左侧节点级联
	 */
	void DigitalWaveGuide::connectLeftLeft(DigitalWaveGuide & d1, DigitalWaveGuide & d2){
		d1.connectLeft(d2.leftNode);
		d2.connectLeft(d1.leftNode);
	}
	/**
	 * 右侧节点级联
	 */
	void DigitalWaveGuide::connectRightRight(DigitalWaveGuide & d1, DigitalWaveGuide & d2){
		d1.connectRight(d2.rightNode);
		d2.connectRight(d1.rightNode);
	}
	// 计算负载（模拟声学系统中的能量负载）
	void DigitalWaveGuide::calculateLoad(){
		if(leftNodeCount == 0){
			leftLoad=0;
		} else{
			leftLoad=leftAlpha * leftNode.signals[0];
			for(u_index k=0; k < leftNodeCount; k++){
				// 极性转换（false→1，true→0）
				int polarity=leftNodePolarities[k]?0:1;
				leftLoad+=leftConnectedNodes[k]->load;
				leftLoad+=leftNodeAlphas[k] * leftConnectedNodes[k]->signals[polarity];
			}
		}
		if(rightNodeCount == 0){
			rightLoad=0;
		} else{
			rightLoad=rightAlpha * rightNode.signals[1];
			for(u_index k=0; k < rightNodeCount; k++){
				// 极性转换（false→0，true→1）
				int polarity=rightNodePolarities[k]?1:0;
				rightLoad+=rightConnectedNodes[k]->load;
				rightLoad+=rightNodeAlphas[k] * rightConnectedNodes[k]->signals[polarity];
			}
		}
	}
	// 处理延迟信号（核心延迟线逻辑）
	void DigitalWaveGuide::processDelay(){
		double delayedRight; // 右侧延迟信号
		double delayedLeft;  // 左侧延迟信号

		// 处理右侧延迟（对应原del1逻辑）
		if(delayLength1 <= 2){
			delayedRight=rightNode.signals[0]; // 无延迟，直接取当前值
		} else{
			// 写入当前信号到延迟缓冲区，读取延迟后的值（原procDsp逻辑）
			delayBuffers[0].write(rightNode.signals[0]);  // 写入新值
			delayedRight=BufferDelayer::cubicSplineDelay(delayBuffers[0], delayLength1 - 1);
			//delayedRight=delayBuffers[0].getFromNewest(delayLength1); // 读取最旧值（延迟后）
		}

		// 处理左侧延迟（对应原del2逻辑）
		if(delayLength2 <= 2){
			delayedLeft=leftNode.signals[1]; // 无延迟，直接取当前值
		} else{
			delayBuffers[1].write(leftNode.signals[1]);    // 写入新值
			delayedLeft=BufferDelayer::cubicSplineDelay(delayBuffers[1], delayLength2 - 1);
			//delayedLeft=delayBuffers[1].getFromNewest(delayLength2);  // 读取最旧值（延迟后）
		}

		// 更新节点信号
		leftNode.signals[0]=delayedRight;
		rightNode.signals[1]=delayedLeft;
	}
	// 更新信号流（核心信号处理）
	void DigitalWaveGuide::updateSignals(){
		double signal=(leftLoad - leftNode.signals[0]);
		// 应用色散处理（commute模式）
		if(commuteFlag){
			for(auto & d : dispersion){
				signal=d->procDsp(signal);
			}
		}
		leftNode.signals[1]=signal * damper;

		// 处理右侧信号
		signal=(rightLoad - rightNode.signals[1]);
		if(commuteFlag){
			signal=lowpass->procDsp(signal);
			fracDelay.write(signal);
			signal=BufferDelayer::cubicSplineDelay(fracDelay, fracDelayLen);
		}
		rightNode.signals[0]=signal * damper;
	}
	// 初始化alpha系数（能量分配权重）
	void DigitalWaveGuide::initAlphaCoefficients(){
		double totalImpedance; // 总阻抗（z为阻抗参数）

		// 计算左侧alpha系数
		totalImpedance=leftNode.impedance;
		for(u_index k=0; k < leftNodeCount; k++){
			totalImpedance+=leftConnectedNodes[k]->impedance;
		}
		leftAlpha=2.0 * leftNode.impedance / totalImpedance;
		for(u_index k=0; k < leftNodeCount; k++){
			leftNodeAlphas[k]=2.0 * leftConnectedNodes[k]->impedance / totalImpedance;
		}

		// 计算右侧alpha系数
		totalImpedance=rightNode.impedance;
		for(u_index k=0; k < rightNodeCount; k++){
			totalImpedance+=rightConnectedNodes[k]->impedance;
		}
		rightAlpha=2.0 * rightNode.impedance / totalImpedance;
		for(u_index k=0; k < rightNodeCount; k++){
			rightNodeAlphas[k]=2.0 * rightConnectedNodes[k]->impedance / totalImpedance;
		}
	}
	void DigitalWaveGuide::setDispersion(std::vector<std::shared_ptr<DSP>> dispersion, std::shared_ptr<DSP> lowpass, double fracDelay){
		commuteFlag=true;
		this->dispersion=dispersion;
		this->lowpass=lowpass;
		this->fracDelayLen=fracDelay;
		this->fracDelay.ensureCapacity(fracDelay);
	}
}