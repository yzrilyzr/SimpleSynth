#pragma once
#include "DWGNode.h"
#include "SimpleSynth.h"
#include "yzrutil.h"
#include "array/ObjectArray.hpp"
#include "dsp/RingBuffer.h"
#include "dsp/DSP.h"
#include <vector>
namespace yzrilyzr_simplesynth{
	class PianoString;
	// 数字波导（物理建模合成中的核心组件，用于模拟声波传播）
	EBCLASS(DigitalWaveGuide){
		public:
			// 左侧节点的极性（true/false表示正负方向）
		bool leftNodePolarities[2];
		// 右侧节点的极性
		bool rightNodePolarities[2];
		// 左侧连接的节点数组
		DWGNode * leftConnectedNodes[2];
		// 右侧连接的节点数组
		DWGNode * rightConnectedNodes[2];
		// 左侧节点的alpha系数（用于能量分配计算）
		double leftNodeAlphas[2];
		// 右侧节点的alpha系数
		double rightNodeAlphas[2];
		// 延迟缓冲区（替代原DWGDelay，存储声波延迟样本）
		yzrilyzr_dsp::RingBufferSample delayBuffers[2];
		// 第一个延迟线的长度（采样数）
		double delayLength1;
		// 第二个延迟线的长度（采样数）
		double delayLength2;
		// 左侧已连接的节点数量
		int leftNodeCount=0;
		// 右侧已连接的节点数量
		int rightNodeCount=0;
		// 左侧核心节点（波导端点）
		DWGNode leftNode;
		// 右侧核心节点（波导端点）
		DWGNode rightNode;
		// 左侧负载值（模拟声学负载）
		double leftLoad;
		// 右侧负载值
		double rightLoad;
		// 左侧alpha系数（核心节点的能量分配权重）
		double leftAlpha;
		// 右侧alpha系数
		double rightAlpha;
		// 交换标志（控制信号处理流程是否交换）
		bool commuteFlag=false;
		double damper=1;
		std::vector<std::shared_ptr<yzrilyzr_dsp::DSP>> dispersion;
		std::shared_ptr<yzrilyzr_dsp::DSP> lowpass=nullptr;
		yzrilyzr_dsp::RingBufferSample fracDelay;
		double fracDelayLen;
		~DigitalWaveGuide();
		DigitalWaveGuide();
		DigitalWaveGuide(double z, double delayLen1, double delayLen2);
		void init(double z, double delayLen1, double delayLen2);
		void initNode(double z);
		/**
		 * d1.r - l.d2（左侧节点与右侧节点连接）
		 */
		static void connectLeftRight(DigitalWaveGuide & d1, DigitalWaveGuide & d2);
		void connectRight(DWGNode & r);
		void connectLeft(DWGNode & l);
		void connectRightEx(DWGNode & r, bool polarity);
		void connectLeftEx(DWGNode & l, bool polarity);
		/**
		 * l.d1
		 * |
		 * l.d2（左侧节点级联）
		 */
		static void connectLeftLeft(DigitalWaveGuide & d1, DigitalWaveGuide & d2);
		/**
		 * d1.r
		 * |
		 * d2.r（右侧节点级联）
		 */
		static void connectRightRight(DigitalWaveGuide & d1, DigitalWaveGuide & d2);
		void calculateLoad();
		void processDelay();
		void updateSignals();
		void initAlphaCoefficients();
		void setDispersion(std::vector<std::shared_ptr<yzrilyzr_dsp::DSP>> dispersion, std::shared_ptr<yzrilyzr_dsp::DSP> lowpass, double fracDelay);
	};
}