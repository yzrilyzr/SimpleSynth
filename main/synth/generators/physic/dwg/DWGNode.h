#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	// 数字波导节点（Digital Waveguide Node）
	// 用于模拟声学系统中的连接点，存储波导中的信号和阻抗参数
	EBCLASS(DWGNode){
	public:
		// 信号数组：[0]和[1]分别表示两个方向的波信号
	double signals[2]={0.0, 0.0};
	// 节点阻抗（Impedance），用于能量分配计算
	double impedance=0.0;
	// 负载值（Load），模拟外部负载对节点的影响
	double load=0.0;

	// 初始化节点参数
	// @param z：节点的阻抗值
	void initialize(double z);
	};
}