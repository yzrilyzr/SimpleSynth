## 加法器 

`AmpAdder` 提供了将输出振幅相加的功能，适合实现**加法合成**

参数说明
|参数名|类型|描述|
|--- |---|---|
|a |`NoteProcPtr` | 加数a |
|b |`NoteProcPtr` | 加数b |

输出：$a+b$

## 乘法器 

`AmpMultiplier` 提供了将输出振幅相乘的功能，适合实现**将音频振幅放大或缩小**或者**环形调制**

参数说明
|参数名|类型|描述|
|--- |---|---|
|a |`NoteProcPtr` | 乘数a |
|b |`NoteProcPtr` | 乘数b |

输出：$a*b$

## 量化器 

`AmpQuantization` 提供了将输出振幅量化至某一整数的功能，适合实现**量化失真器**

参数说明
|参数名|类型|描述|
|--- |---|---|
|a |`NoteProcPtr` | 被量化 |
|q |`uint32_t` | 量化数 |

输出：$\frac{\operatorname{round}(x \cdot q)}{q}$


## 比例混合器 

`AmpRatioMixer` 提供了按比例混合两个输入的功能

参数说明
|参数名|类型|描述|
|--- |---|---|
|a |`NoteProcPtr` | 输入a |
|b |`NoteProcPtr` | 输入b |
|r |`u_normal_11` | 比例值，1代表全是a，-1代表全是b |

输出：$ \frac{1}{2} \big[ a (1 + r) + b (1 - r) \big]$

## 自带CC参数

`AmpWithCC` 提供了自带CC参数控制功能

参数说明
|参数名|类型|描述|
|--- |---|---|
|a |`NoteProcPtr` | 输入a |
|cc |`IntArray` | CC参数组 |


## 自动调制

`AutoMod` 提供了自动调制功能，适合实现**颤音**、**震音**等生动效果

参数说明
|参数名|类型|描述|
|--- |---|---|
|a |`NoteProcPtr` | 输入a |
|modFreqDepth |`u_normal_01` | 调制频率深度|
|modAmpDepth |`u_normal_01` | 调制振幅深度|
|modRate |`u_normal_01` | 调制频率速度|
|modDelay |`u_normal_01` |调制开始延迟 |
|modShape |`u_normal_01` |调制形状 |


## 频率调制

`FreqModAmp` 提供了频率调制功能，适合实现**FM合成**

参数说明
|参数名|类型|描述|
|--- |---|---|
|dst |`NoteProcPtr` | 载波 |
|src |`NoteProcPtr` | 调制波形 |
|depth |`u_sample` | 调制深度 |

## 硬同步

`HardSync` 提供了硬同步功能

参数说明
|参数名|类型|描述|
|--- |---|---|
|slave |`NoteProcPtr` | 从振荡器 |
|slaveFreqRatio |`float` | 从振荡器频率比 |

## 6x6矩阵调制

`Matrix6x6Modulation` 提供了6x6矩阵调制功能，支持**频率调制**和**环形调制**

建议使用`Matrix6x6ModulationBuilder`搭建


`setOp`参数说明

|参数名|类型|描述|
|--- |---|---|
|index |`u_index` | 第几个算子 |
|osc |`NoteProcPtr` | 振荡器波形 |
|freqMul |`double` | 振荡器频率倍乘 |
|freqOff |`u_freq` | 振荡器频率偏移 |
|initPhase |`s_phase` | 振荡器初始相位 |
|input |`u_sample` | 振荡器输入振幅 |
|output |`u_sample` | 振荡器最终输出混合比 |

`setMatrix`参数说明

|参数名|类型|描述|
|--- |---|---|
|type |`int` | 调制类型：TYPE_FM、TYPE_RM |
|from |`u_index` | 第几个算子作为调制源 |
|to |`u_index` | 调制到第几个算子 |
|depth |`u_sample` | 调制深度 |



## 多键触发

`MultiKeyTrigger` 提供了一个音符触发多键功能

## 最邻近表

`NearestAmpSet` 提供了选择最邻近音符处理器输出的功能

输出：选择最邻近的一个音符处理器输出

## 邻近插值表

`NeighbourMixAmpSet` 提供了选择两个邻近音符处理器输出混合的功能

输出：选择两个邻近的音符处理器，获取输出并按比例混合

## 非插值表

`NonInterpolateAmpSet` 提供了选择单个音符处理器输出的功能

输出：选择单个特定的音符处理器输出，没有则输出0

## 音符数字音频处理

`NoteDSP` 提供了对单个音符输出执行数字音频处理的功能

每个音符都是独立的DSP

## 音符音高偏移

`NoteShift` 提供了对音符音高偏移的功能


## 音符力度混合

`NoteVelocityMix` 提供了对音符原始力度和指定力度进行比例混合的功能


## 音符后处理数字音频处理

`PostProcessDSP` 提供了对音符混合后的数字音频处理的功能

## 区域图

`RegionAmp` 提供了将音符处理器按照音高和力度划分为不同区域并根据输入音符选择指定区域的音符处理器进行处理的功能

## 失谐器

`SimpleDetuner` 提供了将音符复制并失谐的功能，适合实现**失谐效果**


## 软同步

`SoftSync` 提供了软同步功能




