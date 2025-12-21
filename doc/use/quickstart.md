## 快速开始：基础合成流程

本节通过一个完整示例展示音频合成的核心流程：**初始化 → 加载音源 → 发送事件 → 合成输出**。

### 完整示例代码

```cpp
#include "Mixer2.h"
#include "SimpleMidiInstrument.h"
#include "ChannelEvent.h"
using namespace yzrilyzr_simplesynth;

int main() {
    // 1. 创建混音器（缓冲区大小=256帧）
    auto mixer = std::make_shared<Mixer2>(256);
    
    // 2. 配置合成参数
    mixer->setSynthMode(IMixer::MODE_THREAD_POOL, -1); // 启用多线程，自动分配CPU核心
    mixer->setSampleRate(48000);                      // 设置采样率（48kHz）
    
    // 3. 加载音源（使用内置测试音色）
    auto instr = std::make_shared<SimpleMidiInstrument>();
    mixer->setInstrumentProvider(instr);
    
    // 4. 安排MIDI事件
    u_time currentTime = mixer->getCurrentTime();  // 获取当前时间戳
    mixer->postEvent(new NoteOn(0, 60, 100), currentTime + 1.0);  // 1秒后：中央C按下
    mixer->postEvent(new NoteOff(0, 60, 100), currentTime + 2.0); // 2秒后：中央C释放
    
    // 5. 执行音频合成
    mixer->mix();  // 合成256帧音频数据
    
    // 6. 获取合成结果
    u_sample* leftCh = mixer->getOutput(0);   // 左声道数据
    u_sample* rightCh = mixer->getOutput(1);  // 右声道数据
    
    return 0;
}
```

---

## 关键步骤详解

### 1. 初始化混音器

混音器是音频合成的核心控制器，缓冲区大小决定每次合成的音频帧数：

```cpp
auto mixer = std::make_shared<Mixer2>(256);  // 256帧缓冲区
```

**参数说明：**
- **缓冲区大小**：影响实时性和延迟，典型值为128-1024
- **值越小**：延迟越低，但CPU占用越高
- **值越大**：CPU占用越低，但延迟越高

### 2. 设置采样率

采样率决定音频的质量和频率响应范围：

```cpp
// 常用采样率设置
mixer->setSampleRate(44100);  // CD音质（兼容性好）
mixer->setSampleRate(48000);  // 专业音频标准（推荐）
mixer->setSampleRate(96000);  // 高分辨率音频（需要高质量音源）
```

### 3. 加载音源

支持多种音源格式，从简单测试音色到专业音色库：

```cpp
// 选项1：使用内置测试音色（快速验证）
auto simpleInstr = std::make_shared<SimpleMidiInstrument>();

// 选项2：加载SF2音色库
auto sf2Instr = std::make_shared<SF2FormatInstrument>(
    yzrilyzr_io::FileInputStream("piano.sf2")
);

// 选项3：加载DLS音色库
auto dlsInstr = std::make_shared<DLSFormatInstrument>(
    yzrilyzr_io::FileInputStream("strings.dls")
);

mixer->setInstrumentProvider(instr);  // 设置当前音源
```

### 4. 发送MIDI事件

事件系统控制音符播放，支持定时和即时两种方式：

```cpp
// 队列事件（定时执行）
u_time now = mixer->getCurrentTime();
mixer->postEvent(new NoteOn(0, 60, 100), now + 0.5);   // 0.5秒后触发
mixer->postEvent(new NoteOff(0, 60, 0), now + 1.5);    // 1.5秒后触发

// 即时事件（立即执行）
mixer->sendEventImmediate(new NoteOn(0, 64, 90));  // 立即播放E4音符
```

### 5. 执行合成与获取输出

合成流程通常在音频回调中循环执行：

```cpp
// 音频驱动回调函数示例
void audioCallback(float* outputBuffer, int frameCount) {
    // 执行合成
    mixer->mix();
    
    // 复制到输出缓冲区
    u_sample* left = mixer->getOutput(0);
    u_sample* right = mixer->getOutput(1);
    
    for(int i = 0; i < frameCount; i++) {
        outputBuffer[2*i] = left[i];      // 左声道
        outputBuffer[2*i + 1] = right[i]; // 右声道
    }
}
```

---

## 平台对接指南

### Windows (WASAPI)
```cpp
// 需包含相应音频API头文件
// 在音频线程中循环调用 mixer->mix()
```

### macOS/iOS (Core Audio)
```cpp
// 在AudioUnit回调中调用合成函数
OSStatus renderCallback(void* userData,
                       AudioUnitRenderActionFlags* flags,
                       const AudioTimeStamp* timeStamp,
                       UInt32 busNumber,
                       UInt32 frameCount,
                       AudioBufferList* data) {
    auto mixer = static_cast<Mixer2*>(userData);
    mixer->mix();
    // ... 复制数据到data->mBuffers
    return noErr;
}
```

### Linux (ALSA/JACK)
```cpp
// 在音频处理线程中定期调用
while(running) {
    mixer->mix();
    // ... 写入音频设备
}
```
