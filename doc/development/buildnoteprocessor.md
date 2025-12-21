SimpleSynth提供了强大的音符处理器（NoteProcessor）接口，允许开发者创建自定义的声音合成算法。本文将详细介绍如何从零开始，创建一个属于你自己的音符处理器。

## 核心概念：NoteProcessor

`NoteProcessor` 是SimpleSynth中所有声音生成器和效果器的基类。它的核心职责是在每个音频样本点上，计算出一个音符（Note）应产生的振幅（Amplitude）。

一个完整的音符处理器开发流程通常包括：
1.  **继承 `NoteProcessor` 类**
2.  **重写核心方法**，主要是 `getAmp`
3.  **（可选）重写音符生命周期管理方法**，如 `init`, `noMoreData`
4.  **（可选）重写音符后处理方法** `postProcess`
5.  **（可选）重写事件处理方法处理方法** `noteOn`, `noteOff`, `cc`
6.  **实现克隆方法** `clone`

---

## 创建处理器类

首先，你需要创建一个新的C++类，并让它继承自 `yzrilyzr_simplesynth::NoteProcessor`。

**示例：创建一个名为 `MyCustomProcessor` 的头文件 (`MyCustomProcessor.h`)**

```cpp
#pragma once
#include "interface/NoteProcessor.h" // 引入NoteProcessor基类

// 确保使用正确的命名空间
namespace yzrilyzr_simplesynth{

    // 使用ECLASS宏来定义你的类，这有助于SimpleSynth的内部管理
    EBCLASS(MyCustomProcessor, public NoteProcessor){
    public:
        // 构造函数
        MyCustomProcessor();

        // 【核心】重写getAmp方法，这是声音合成的入口
        u_sample getAmp(Note & note) override;

        // 【重要】重写clone方法，用于多线程安全和状态复制
        NoteProcPtr clone() override;

        // 【可选】重写初始化方法
        void init(ChannelConfig & cfg) override;
        
        // 【可选】重写后处理方法
        u_sample postProcess(u_sample output) override;

        // 【可选】重写生命周期判断方法
        bool noMoreData(Note & note) override;

        // 【可选】重写音符开始事件
        void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel) override;

        // 【可选】重写音符关闭事件
        void noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel) override;

        // 【可选】重写通道控制事件
        void cc(ChannelConfig & cfg, ChannelControl & cc) override;
    };
}
```

---

## 实现核心方法 `getAmp`

`getAmp` 方法是整个处理器的灵魂。它接收一个 `Note` 对象作为参数，并返回一个 `u_sample` 类型的振幅值（通常在 -1.0 到 1.0 之间）。

`Note` 对象包含了合成该音符所需的所有动态信息，例如：
* `note.id`: **音符触发的音** (note)。由事件设置，是音符的基础音。
* `note.velocity`: **音符触发的力度** (0.0 到 1.0)。由事件设置，是音符的基础力度。
* `note.idSynth*`: **当前音符的实际音** (note)。这是计算频率的关键。
* `note.freqSynth`: **当前音符的实际频率** (Hz)。这是计算周期、相位的关键。
* `note.velocitySynth*`: **当前音符的实际力度** (0.0 到 1.0)。由音符更新器设置，用于控制合成的力度。
* `note.passedTime`: **音符已经持续的时间** (秒)。用于实现包络线（ADSR）、颤音等时间相关效果。
* `note.phaseSynth`: **合成累积相位** ，即频率对时间的积分。由合成器自动更新，方便快速生成周期性波形。
  
> 提示：带*符号的会被LFO调制直接影响
> 
> 详情参考 `NoteUpdater.cpp`


**示例：实现一个简单的正弦波**

在你的 `.cpp` 文件 (`MyCustomProcessor.cpp`) 中：

```cpp
#include "MyCustomProcessor.h"
#include "dsp/FastSin.h" // 引入快速正弦函数

namespace yzrilyzr_simplesynth{

    MyCustomProcessor::MyCustomProcessor() : NoteProcessor() {
        // 构造函数中可以进行一些基础的初始化
    }

    u_sample MyCustomProcessor::getAmp(Note & note){
        // 使用FastSin库和合成器提供的相位来生成正弦波
        // getPhase(note) 返回一个0到1之间的归一化相位
        // _2PI 是一个常量 (2 * PI)
        u_sample sine_wave = fast_sin(getPhase(note) * _2PI, note.freqSynth);
        
        // 将生成的波形乘以力度，控制音量
        return sine_wave * note.velocitySynth;
    }
    
    // ... 其他方法的实现将在后续步骤中添加 ...
}
```
> **提示**：`getPhase(note)` 是 `Osc` (振荡器) 类提供的一个便捷方法。如果你的处理器是一个振荡器，建议直接继承 `Osc` 类，它已经为你处理好了相位的累加和管理。

---

## 实现克隆方法 `clone`

`clone` 方法至关重要，因为合成器在多线程环境下工作时，可能会为每个音符或每个声道复制一个处理器实例，以避免状态冲突。

**示例：实现 `clone` 方法**

```cpp
// 在 MyCustomProcessor.cpp 中

NoteProcPtr MyCustomProcessor::clone(){
    // 创建一个当前类的新实例，并返回其智能指针
    // 确保复制所有必要的成员变量
    return std::make_shared<MyCustomProcessor>();
}
```
> **高级用法**：如果你的处理器包含一些可配置的参数（如滤波器 cutoff 频率），你的 `clone` 方法需要将这些参数值也复制到新实例中。参考 `BowedString` 的实现。

---

## 实现初始化方法 `init` (可选)

`init` 方法在处理器被分配给一个声道（Channel）时调用。这是设置与采样率（sample rate）相关参数或初始化DSP效果器的理想位置。

**示例：在 `init` 中设置采样率并初始化滤波器**

```cpp
// 在 MyCustomProcessor.h 中添加成员变量
private:
    u_sample_rate sampleRate = 0;
    std::shared_ptr<yzrilyzr_dsp::BiquadIIR> myFilter;

// 在 MyCustomProcessor.cpp 中实现 init
void MyCustomProcessor::init(ChannelConfig & cfg){
    // 调用基类的init方法
    NoteProcessor::init(cfg);

    // 保存采样率，以备后用
    this->sampleRate = cfg.sampleRate;

    // 初始化一个低通滤波器
    myFilter = std::make_shared<yzrilyzr_dsp::BiquadIIR>();
    // 假设我们要创建一个1000Hz的低通滤波器
    yzrilyzr_dsp::IIRUtil::biquad(*myFilter, 1000.0, sampleRate, 0.707, yzrilyzr_dsp::FilterPassType::LOWPASS);
}
```

---

## 实现后处理方法 `postProcess` (可选)

`postProcess` 方法用于对音符的最终输出进行效果处理。它会在 **当前采样点或缓冲块下** 所有 `Note` 的 `getAmp` 调用完成并混音之后被调用（即所有 `Note` 的输出之和）。这非常适合实现混响、延迟、均衡器、物理建模的箱体等全局效果。

**示例：使用 `init` 中创建的滤波器进行后处理**

```cpp
// 在 MyCustomProcessor.cpp 中实现 postProcess
u_sample MyCustomProcessor::postProcess(u_sample output){
    // 将 getAmp 的输出通过我们的低通滤波器
    if(myFilter){
        return myFilter->procDsp(output);
    }
    return output; // 如果滤波器未初始化，则直接返回原始输出
}
```

---

## 实现生命周期管理 `noMoreData` (可选)

`noMoreData` 方法告诉合成器一个音符是否已经“没有更多数据”可以产生了。当一个音符被释放（NoteOff）后，它可能还会在释放（Release）阶段持续发声一段时间。当声音完全消失后，`noMoreData` 应该返回 `true`，这样合成器就可以归还这个音符实例到对象池，节省资源。

默认实现为：音符关闭即没有数据

**示例：简单的释放时间判断**

```cpp
// 在 MyCustomProcessor.cpp 中实现 noMoreData
bool MyCustomProcessor::noMoreData(Note & note){
    // 如果音符已经关闭（NoteOff已触发）并且已经过去2秒，则认为它已结束
    if(note.closed(*note.cfg) && note.closedPassedTime > 2.0){
        return true;
    }
    return false;
}
```

## NoteProcessor 生命周期
当 `NoteProcessor` 附加到 `Channel` 时，调用 `NoteProcessor` 的 `init(ChannelConfig & cfg)` 方法。

合成流程：
1. 框架内部调用：NoteUpdater::preUpdateNote(note, cfg)
2. 调用 `getAmp`，传入当前通道的每个音符，返回该音符的采样振幅
3. 框架内部调用：NoteUpdater::postUpdateNote(note, cfg);
4. 调用 `noMoreData`，传入当前通道的每个音符，返回该音符是否还有数据
5. 所有当前通道的音符混音后，调用`postProcess`，传入混音结果，返回后处理结果

## 总结与最佳实践

1.  **从简单开始**：先实现一个基础的 `getAmp`，例如正弦波或方波。
2.  **利用继承**：如果你的处理器是一个振荡器，继承 `Osc` 类可以省去你管理相位的麻烦。
3.  **状态隔离**：确保每个音符的状态是独立的。如果你的处理器需要为每个音符保存数据（如 `BowedString` 中的环形缓冲区），请使用 `NoteData<T>` 模板或类似机制。不要使用全局或静态变量来存储音符的状态！
4.  **线程安全**：`getAmp` 可能会被多线程调用，确保使用无锁数据。
5.  **资源管理**：在析构函数 `~MyCustomProcessor()` 中释放所有动态分配的资源（如 `new` 出来的对象）。

通过遵循以上步骤，你就可以开始构建从简单到复杂的各种声音处理器，极大地扩展SimpleSynth的声音合成能力。