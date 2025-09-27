#include "DWGNode.h"

namespace yzrilyzr_simplesynth{
	// 初始化节点：重置信号值并设置阻抗
	void DWGNode::initialize(double z){
		signals[0]=0.0;  // 重置方向0的信号
		signals[1]=0.0;  // 重置方向1的信号
		impedance=z;     // 设置阻抗值
		load=0.0;        // 重置负载
	}
}