#pragma once
#include "DWGNode.h"
#include "SimpleSynth.h"
#include "yzrutil.h"
#include "array/Array.hpp"
#include "collection/ArrayList.hpp"
#include "dsp/RingBuffer.h"
#include "dsp/DSP.h"
#include <vector>
/**
*
* 每个 DWGNode（波导节点）包含两个信号值 signals[0] 和 signals[1]，分别代表两个方向的行波。在典型的波导网络中，方向定义如下（以一段波导为例）：
* 左节点（leftNode）
* signals[0]：进入左节点的波（来自右侧，即向右传播的波到达左节点）
* signals[1]：离开左节点的波（向左传播，从左侧发出）
*
* 右节点（rightNode）
* signals[0]：离开右节点的波（向左传播，从右侧发出）
* signals[1]：进入右节点的波（来自左侧，即向左传播的波到达右节点）
*
* 在 processDelay() 中，延迟线将右节点的 signals[0] 延迟后写入左节点的 signals[0]，将左节点的 signals[1] 延迟后写入右节点的 signals[1]，直观体现了波的传播方向。
*/
namespace yzrilyzr_simplesynth{
	class PianoString;
	// 数字波导（物理建模合成中的核心组件，用于模拟声波传播）
	EBCLASS(DigitalWaveGuide){
		public:
			// 左侧节点的极性（true/false表示正负方向）
		bool leftNodePolarities[2];
		// 右侧节点的极性
		bool rightNodePolarities[2];
		// 左侧连接的节点数组引用
		DWGNode * leftConnectedNodes[2];
		// 右侧连接的节点数组引用
		DWGNode * rightConnectedNodes[2];
		// 左侧节点的alpha系数（用于能量分配计算）
		u_sample leftNodeAlphas[2];
		// 右侧节点的alpha系数
		u_sample rightNodeAlphas[2];
		// 延迟缓冲区,存储声波延迟样本
		yzrilyzr_dsp::RingBufferSample delayBuffers[2];
		// 第一个延迟线的长度（采样数）
		u_sample delayLength1;
		// 第二个延迟线的长度（采样数）
		u_sample delayLength2;
		// 左侧已连接的节点数量
		int leftNodeCount=0;
		// 右侧已连接的节点数量
		int rightNodeCount=0;
		// 左侧核心节点（波导端点）
		DWGNode leftNode;
		// 右侧核心节点（波导端点）
		DWGNode rightNode;
		// 左侧负载值（模拟声学负载）
		u_sample leftLoad;
		// 右侧负载值
		u_sample rightLoad;
		// 左侧alpha系数（核心节点的能量分配权重）
		u_sample leftAlpha;
		// 右侧alpha系数
		u_sample rightAlpha;
		// 交换标志（控制信号处理流程是否交换）
		bool commuteFlag=false;
		u_sample damper=1;
		yzrilyzr_collection::ArrayList<yzrilyzr_dsp::DSPPtr> dispersion;
		yzrilyzr_dsp::DSPPtr lowpass=nullptr;
		yzrilyzr_dsp::RingBufferSample fracDelay;
		u_sample fracDelayLen;
		~DigitalWaveGuide();
		DigitalWaveGuide();
		DigitalWaveGuide(u_sample z, u_sample delayLen1, u_sample delayLen2);
		void init(u_sample z, u_sample delayLen1, u_sample delayLen2);
		void initNode(u_sample z);
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
		void resetConnection();
		void resetMemory();
		void calculateLoad();
		void processDelay();
		void updateSignals();
		void initAlphaCoefficients();
		void setDispersion(const yzrilyzr_collection::ArrayList<yzrilyzr_dsp::DSPPtr> &dispersion, yzrilyzr_dsp::DSPPtr lowpass, u_sample fracDelay);
	};
}