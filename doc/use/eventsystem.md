`SimpleSynth`通过事件驱动音频合成，支持两种事件类型，覆盖不同使用场景：
## 队列事件（Scheduled Events）
**适用场景**：预编排的音乐（如播放MIDI文件、自动伴奏），按指定时间触发。



```cpp
// 示例1：延迟3秒播放A4音符（69），力度80，持续2秒
u_time delay = 3.0;
mixer->postEvent(new NoteOn(0, 69, 80), mixer->getCurrentTime() + delay);
mixer->postEvent(new NoteOff(0, 69, 80), mixer->getCurrentTime() + delay + 2.0);

// 示例2：从MIDI文件加载事件序列
#include "MixerSequence.h"
#include "SynthUtil.h"

// 解析MIDI文件（支持.mid/.rmid/.mids格式）
std::shared_ptr<MixerSequence> midiSeq = SynthUtil::parseMIDI(
    FileInputStream("path/to/your/music.mid")
);
if (midiSeq) {
    // 1秒后开始播放，事件组命名为"MyMIDI"（便于后续管理）
    midiSeq->postToMixer(mixer.get(), 1.0, "MyMIDI");
}

// 示例3：从XM文件加载事件序列（Fasttracker 2模块文件）
std::shared_ptr<MixerSequence> xmSeq = SynthUtil::parseXM(
    FileInputStream("path/to/your/tune.xm")
);
if (xmSeq) {
    xmSeq->postToMixer(mixer.get(), 0.0, "MyXM"); // 立即播放
}
```

## 即时事件（Instant Events）
**适用场景**：实时交互（如MIDI键盘输入、UI按钮触发），调用后立即生效。
```cpp
// 示例1：直接发送NoteOn事件（立即播放C5音符）
mixer->sendInstantEvent(new NoteOn(0, 72, 90));

// 示例2：通过MIDI字节快速发送（更符合硬件MIDI交互习惯）
#include "SynthUtil.h"

// 发送控制改变事件（CC 10：声像，值50 → 偏左声道）
SynthUtil::sendMIDIBytes(mixer.get(), 0xB0, 10, 50, "InstantInput");
// 发送程序改变事件（通道0，切换到钢琴音色（程序号0））
SynthUtil::sendMIDIBytes(mixer.get(), 0xC0, 0, 0, "InstantInput");
```

## 事件类型

### NOTE_ON
### NOTE_OFF
### NOTE_PRESSURE
### NOTE_PITCH_BEND
### CHANNEL_CONTROL
### CHANNEL_PITCH_BEND
### CHANNEL_PROGRAM_CHANGE
### CHANNEL_PRESSURE
### TUNING_CHANGE