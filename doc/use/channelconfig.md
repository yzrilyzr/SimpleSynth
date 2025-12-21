`ChannelConfig` 是合成器通道的核心配置类，管理着单个MIDI通道的所有状态、参数和效果。每个MIDI通道（0-15）都有独立的 `ChannelConfig` 实例，用于控制音色、效果、演奏技巧等各方面参数。

## 基础配置

### 上下文设置

通道配置需要与混音器和通道对象关联：

```cpp
// 设置上下文（必须在创建后调用）
void setContext(IMixer* mixer, IChannel* channel);

// 使用示例
auto channel = mixer->getMIDIChannel(0);  // 获取通道0
auto& config = channel->getConfig();
config.setContext(mixer.get(), channel.get());
```

### 默认配置

```cpp
// 获取默认配置
std::shared_ptr<ChannelConfig> config = ChannelConfig::DefaultConfig();

// 默认配置包含：
// - 十二平均律调音
// - 简单MIDI乐器
// - 2次幂速度映射曲线
// - 通道音量70%
```

## 通道基础参数

### 音量与声像控制

| 参数 | 类型 | 范围 | 默认值 | 描述 |
|------|------|------|--------|------|
| `Volume` | `float` | 0.0-1.0 | 0.7 | 通道音量 |
| `Pan` | `float` | -1.0-1.0 | 0.0 | 声像位置（-1=左，0=中，1=右） |
| `Expression` | `float` | 0.0-1.0 | 1.0 | 表情控制（实时音量微调） |
| `Breath` | `float` | 0.0-1.0 | 1.0 | 呼吸控制器（常用于木管乐器） |
| `Foot` | `float` | 0.0-1.0 | 1.0 | 脚踏控制器 |

### 音高控制

| 参数 | 类型 | 范围 | 默认值 | 描述 |
|------|------|------|--------|------|
| `ChannelPitchBend` | `float` | -1.0-1.0 | 0.0 | 弯音轮值 |
| `PitchBendRange` | `float` | 0.0+ | 2.0 | 弯音范围（半音数） |
| `FineTune` | `float` | ±100 | 0.0 | 精细调音（音分） |
| `CoarseTune` | `float` | ±48 | 0.0 | 粗调音（半音） |
| `Detune` | `float` | 0.0-1.0 | 0.0 | 失谐效果强度 |
| `NoteShift` | `float` | ±48 | 0.0 | 全局音符偏移 |

### 调制控制

| 参数 | 类型 | 范围 | 默认值 | 描述 |
|------|------|------|--------|------|
| `Modulation` | `float` | 0.0-1.0 | 0.0 | 调制轮总效果量 |
| `ModRate` | `float` | 0.0+ | 5.0 | 调制速率（Hz） |
| `ModDelay` | `float` | 0.0+ | 0.3 | 调制延迟（秒） |
| `ModDepth` | `float` | 0.0+ | 0.5 | 调制深度 |

## 演奏模式控制

### 踏板与延音

```cpp
// 启用/禁用延音踏板
config.Sustain = true;   // 启用延音（CC#64）
config.Sustain = false;  // 禁用延音

// 选择性延音（Sostenuto）
config.Sostenuto = true;   // 锁定当前音符
config.Sostenuto = false;  // 释放锁定

// 弱音踏板（Soft Pedal）
config.SoftPedal = true;   // 启用弱音（CC#67）
config.SoftPedal = false;  // 禁用弱音

// 更新延音锁定状态
config.sostenutoChange();  // 应用Sostenuto状态变化
```

### 单音/复音模式

```cpp
// 单音模式（Mono Mode）
config.MonoMode = true;    // 单音模式，同时只能发一个音
config.MonoMode = false;   // 复音模式（默认）

// 滑音（Portamento）
config.Portamento = true;          // 启用滑音
config.PortamentoTime = 0.5f;      // 滑音时间（秒）
config.Portamento = false;         // 禁用滑音

// 连奏（Legato）
config.Legato = true;    // 连奏模式（音符重叠时无重新触发）
config.Legato = false;   // 非连奏模式
```

## 音符管理

### 音符状态跟踪

```cpp
// 音符按下/释放状态（128个音符）
bool isNoteHeld = config.noteHoldMap[60];  // 中央C是否按下
bool isSostenuto = config.sostenutoLock[60];  // 中央C是否被Sostenuto锁定

// 当前演奏的音符
Note* currentNote = config.lastNote;  // 最后一次NoteOn的音符

// 释放所有音符
config.allNotesOff();  // 相当于发送All Notes Off消息
```

### 速度映射

```cpp
// 设置速度响应曲线
#include "interpolator/PowInterpolator.h"

// 创建2次幂曲线（力度响应更强）
auto velocityMap = std::make_shared<PowInterpolator>(2.0f);
config.setNoteVelocityMap(velocityMap);

// 创建线性曲线
auto linearMap = std::make_shared<PowInterpolator>(1.0f);
config.setNoteVelocityMap(linearMap);

// 创建平方根曲线（柔和响应）
auto softMap = std::make_shared<PowInterpolator>(0.5f);
config.setNoteVelocityMap(softMap);
```

## 音源与调音

### 乐器提供器

```cpp
// 设置乐器提供器（控制音色）
#include "instrument/SimpleMIDIInstrument.h"
#include "instrument/SF2FormatInstrument.h"

// 使用内置简单乐器
auto simpleInstr = std::make_shared<SimpleMIDIInstrument>();
config.setInstrumentProvider(simpleInstr);

// 加载SoundFont音色库
auto sf2Instr = std::make_shared<SF2FormatInstrument>(
    yzrilyzr_io::FileInputStream("piano.sf2")
);
config.setInstrumentProvider(sf2Instr);

// 获取当前乐器
InstrumentProvider* instr = config.instrument;
```

### 调音系统

```cpp
// 设置调音系统
#include "tuning/EqualTemperament.h"
#include "tuning/JustIntonation.h"

// 使用十二平均律（默认）
auto equalTemp = std::make_shared<EqualTemperament>();
config.setNoteTuning(equalTemp);

// 使用纯律
auto justIntonation = std::make_shared<JustIntonation>();
config.setNoteTuning(justIntonation);

// 获取当前调音系统
NoteTuning* tuning = config.tuning;

// 设置音色库
config.Bank = 1;  // 选择音色库1
```

### 音符处理器

```cpp
// 设置自定义音符处理器
class MyNoteProcessor : public NoteProcessor {
public:
    void process(Note* note, ChannelConfig& config) override {
        // 自定义处理逻辑
    }
};

auto processor = std::make_shared<MyNoteProcessor>();
config.setNoteProcessor(processor);
```

## 3D音频效果

### 启用3D定位

```cpp
#include "dsp/DSP3D.h"

// 创建3D音频处理器
auto dsp3d = std::make_shared<yzrilyzr_dsp::DSP3D>();

// 设置3D参数
dsp3d->setPosition(0.0f, 0.0f, 5.0f);  // 声源位置
dsp3d->setListenerPosition(0.0f, 0.0f, 0.0f);  // 听者位置
dsp3d->setRoomSize(10.0f, 8.0f, 6.0f);  // 房间尺寸

// 应用到通道
config.set3DEffect(dsp3d);

// 获取3D处理器
yzrilyzr_dsp::DSP3D* dsp = config.dsp3d;
```

### 动态更新3D位置

```cpp
// 在游戏/VR中实时更新3D位置
void update3DPosition(ChannelConfig& config, float x, float y, float z) {
    if (config.dsp3d) {
        config.dsp3d->setPosition(x, y, z);
    }
}

// 更新听者位置（通常全局一个听者）
void updateListener(ChannelConfig& config, float x, float y, float z, 
                    float yaw, float pitch, float roll) {
    if (config.dsp3d) {
        config.dsp3d->setListenerPosition(x, y, z);
        config.dsp3d->setListenerOrientation(yaw, pitch, roll);
    }
}
```

## RPN/NRPN参数控制

### RPN标准参数

```cpp
// RPN（注册参数编号）控制
PNData& rpn = config.rpn;

// 常用RPN操作
rpn.setParamMSB(0, 2);   // 弯音灵敏度（半音数）
rpn.setParamLSB(0, 0);
rpn.setValueMSB(0, 2);   // 2个半音
rpn.setValueLSB(0, 0);

// 精细调音
rpn.setParamMSB(1, 0);   // 精细调音
rpn.setParamLSB(1, 1);
rpn.setValueMSB(1, 64);  // 中性值（8192）
rpn.setValueLSB(1, 0);

// 粗调音
rpn.setParamMSB(2, 0);   // 粗调音
rpn.setParamLSB(2, 2);
rpn.setValueMSB(2, 64);  // 中性值
rpn.setValueLSB(2, 0);
```

### NRPN扩展参数

```cpp
// NRPN（非注册参数编号）控制
PNData& nrpn = config.nrpn;

// 设置自定义参数
nrpn.setParamMSB(1, 0);   // 参数1000（混音器限制器）
nrpn.setParamLSB(1, 0);
nrpn.setValueMSB(1, 64);  // 默认值
nrpn.setValueLSB(1, 0);

// 重置所有RPN/NRPN
config.rpn.reset();
config.nrpn.reset();
```

## 效果器控制

### 通过IChannel接口控制效果器

```cpp
// 获取通道对象
std::shared_ptr<IChannel> channel = mixer->getMIDIChannel(0);

// 设置效果器参数
channel->setChorus(0.3f);    // 合唱深度30%
channel->setReverb(0.4f);    // 混响强度40%
channel->setPhaser(0.2f);    // 移相器强度20%
channel->setDetune(0.1f);    // 失谐强度10%

// 获取效果器实例
yzrilyzr_dsp::Chorus& chorus = channel->getChorus(0);     // 左声道合唱
yzrilyzr_dsp::Freeverb& reverb = channel->getReverb(0);   // 左声道混响
yzrilyzr_dsp::Phaser& phaser = channel->getPhaser(0);     // 左声道移相器
```

### 直接效果器配置

```cpp
// 获取配置
ChannelConfig& config = channel->getConfig();

// 通过配置获取混音器
IMixer* mixer = config.mixer;
u_sample_rate sampleRate = config.sampleRate;
u_index channelCount = mixer->getOutputChannelCount();

// 配置各声道效果器
for (u_index i = 0; i < channelCount; i++) {
    yzrilyzr_dsp::Chorus& chorus = channel->getChorus(i);
    chorus.depthMs = 50.0f * 0.3f;  // 30%深度
    chorus.rateHz = 1.5f;
    chorus.init(sampleRate);
    
    yzrilyzr_dsp::Freeverb& reverb = channel->getReverb(i);
    reverb.roomSize = 0.4f * 0.95f;
    reverb.damper = 0.18f;
    reverb.wetRatio = 0.2f;
    reverb.init(sampleRate);
}
```

## 事件系统集成

### 发送事件

```cpp
// 通过配置发送即时事件
void ChannelConfig::postInstantEvent(ChannelEvent* event);

// 使用示例
config.postInstantEvent(new NoteOn(config.channel->getChannelID(), 60, 100));
config.postInstantEvent(new ControlChange(config.channel->getChannelID(), 7, 120)); // 音量
config.postInstantEvent(new PitchBend(config.channel->getChannelID(), 0.5f)); // 向上弯音
```

### MIDI控制启用配置

```cpp
// 通过IChannel启用/禁用MIDI控制
channel->ENABLE_MIDI_CHANNEL_CONTROL = true;     // 启用MIDI CC控制
channel->ENABLE_MIDI_PROGRAM_CHANGE = true;      // 启用音色切换
channel->ENABLE_MIDI_CC_ADSR = false;            // 禁用CC控制ADSR
channel->ENABLE_MIDI_CC_EFFECT = false;          // 禁用CC控制效果器

// 检查是否为鼓组通道
bool isDrumChannel = channel->isDrumSetChannel();  // 通道9为鼓组
```

## 配置管理

### 复制配置

```cpp
// 复制通道配置（不包括共享指针）
void setOnlyChannelConfig(const ChannelConfig& other);

// 完整复制配置（包括共享指针）
void set(const ChannelConfig& other);

// 使用示例
ChannelConfig newConfig;
newConfig.setOnlyChannelConfig(oldConfig);  // 只复制值
newConfig.set(oldConfig);                   // 完整复制

// 批量应用到多个通道
void applyConfigToAllChannels(IMixer* mixer, const ChannelConfig& config) {
    auto channels = mixer->getAllChannels();
    for (auto& channel : channels) {
        channel->getConfig().set(config);
    }
}
```

### 重置配置

```cpp
// 重置到默认值
void reset();

// 重置操作包括：
// - 声像居中，音量70%
// - 所有控制器归零
// - 关闭所有踏板
// - 重置RPN/NRPN
// - 清除音符状态

// 使用示例
config.reset();  // 恢复到初始状态
```

### 动态采样率设置

```cpp
// 采样率相关参数会自动更新
config.sampleRate = 48000;    // 设置采样率
config.currentTime = 0.0;     // 当前时间（秒）
config.deltaTime = 1.0/48000; // 每帧时间

// 注意：通常不需要手动设置，混音器会自动更新
```

## 实用示例

### 完整的通道配置流程

```cpp
#include "Mixer2.h"
#include "ChannelConfig.h"
#include "tuning/EqualTemperament.h"
#include "instrument/SF2FormatInstrument.h"

using namespace yzrilyzr_simplesynth;

void setupPianoChannel(std::shared_ptr<IMixer> mixer, uint8_t channelId) {
    // 获取通道
    auto channel = mixer->getMIDIChannel(channelId);
    auto& config = channel->getConfig();
    
    // 设置上下文
    config.setContext(mixer.get(), channel.get());
    
    // 加载钢琴音色
    auto pianoInstr = std::make_shared<SF2FormatInstrument>(
        yzrilyzr_io::FileInputStream("grand_piano.sf2")
    );
    config.setInstrumentProvider(pianoInstr);
    
    // 设置调音
    auto tuning = std::make_shared<EqualTemperament>();
    tuning->setConcertPitch(442.0f);  // A4=442Hz
    config.setNoteTuning(tuning);
    
    // 设置力度曲线（柔和响应）
    auto velocityMap = std::make_shared<PowInterpolator>(0.7f);
    config.setNoteVelocityMap(velocityMap);
    
    // 配置通道参数
    config.Volume = 0.8f;           // 80%音量
    config.Pan = -0.2f;             // 略微偏左
    config.Expression = 1.0f;       // 最大表情
    config.PitchBendRange = 2.0f;   // 2个半音弯音
    
    // 启用效果器
    channel->setReverb(0.3f);       // 30%混响（大厅效果）
    channel->setChorus(0.1f);       // 轻微合唱
    channel->setDetune(0.05f);      // 轻微失谐（更丰满）
    
    // 启用MIDI控制
    channel->ENABLE_MIDI_CHANNEL_CONTROL = true;
    channel->ENABLE_MIDI_PROGRAM_CHANGE = true;
    
    // 设置通道名称
    channel->setName("Grand Piano");
}

void setupStringsSection(std::shared_ptr<IMixer> mixer) {
    // 弦乐组配置（多个通道）
    auto violinConfig = ChannelConfig::DefaultConfig();
    violinConfig.Volume = 0.7f;
    violinConfig.Pan = -0.5f;  // 左侧
    
    auto violaConfig = ChannelConfig::DefaultConfig();
    violaConfig.Volume = 0.6f;
    violaConfig.Pan = -0.2f;   // 中左
    
    auto celloConfig = ChannelConfig::DefaultConfig();
    celloConfig.Volume = 0.8f;
    celloConfig.Pan = 0.3f;    // 中右
    
    auto bassConfig = ChannelConfig::DefaultConfig();
    bassConfig.Volume = 0.9f;
    bassConfig.Pan = 0.6f;     // 右侧
    
    // 应用到通道
    mixer->getMIDIChannel(0)->getConfig().set(violinConfig);
    mixer->getMIDIChannel(1)->getConfig().set(violaConfig);
    mixer->getMIDIChannel(2)->getConfig().set(celloConfig);
    mixer->getMIDIChannel(3)->getConfig().set(bassConfig);
    
    // 统一设置效果
    for (int i = 0; i < 4; i++) {
        auto channel = mixer->getMIDIChannel(i);
        channel->setReverb(0.4f);    // 弦乐需要较多混响
        channel->setChorus(0.2f);    // 合唱增强融合度
    }
}
```

### 实时控制器映射

```cpp
class MidiControllerMapper {
private:
    std::map<uint8_t, std::function<void(float)>> ccHandlers;
    ChannelConfig& config;
    
public:
    MidiControllerMapper(ChannelConfig& cfg) : config(cfg) {
        setupDefaultMappings();
    }
    
    void setupDefaultMappings() {
        // CC#1: 调制轮 → 颤音深度
        ccHandlers[1] = [this](float value) {
            config.Modulation = value / 127.0f;
        };
        
        // CC#7: 通道音量
        ccHandlers[7] = [this](float value) {
            config.Volume = value / 127.0f;
        };
        
        // CC#10: 声像
        ccHandlers[10] = [this](float value) {
            config.Pan = (value / 127.0f) * 2.0f - 1.0f;
        };
        
        // CC#11: 表情
        ccHandlers[11] = [this](float value) {
            config.Expression = value / 127.0f;
        };
        
        // CC#64: 延音踏板
        ccHandlers[64] = [this](float value) {
            config.Sustain = value >= 64;
            if (config.Sostenuto) {
                config.sostenutoChange();
            }
        };
        
        // CC#91: 混响发送
        ccHandlers[91] = [this](float value) {
            if (config.channel) {
                config.channel->setReverb(value / 127.0f);
            }
        };
    }
    
    void handleControlChange(uint8_t cc, uint8_t value) {
        if (ccHandlers.find(cc) != ccHandlers.end()) {
            ccHandlers[cc](value);
        }
    }
    
    void addCustomMapping(uint8_t cc, std::function<void(float)> handler) {
        ccHandlers[cc] = handler;
    }
};
```

## 高级应用

### 自动化参数插值

```cpp
class ParameterAutomation {
private:
    struct AutomationPoint {
        u_time time;
        float value;
    };
    
    std::map<std::string, std::vector<AutomationPoint>> automations;
    ChannelConfig& config;
    
public:
    ParameterAutomation(ChannelConfig& cfg) : config(cfg) {}
    
    void addAutomationPoint(const std::string& param, u_time time, float value) {
        automations[param].push_back({time, value});
        std::sort(automations[param].begin(), automations[param].end(),
                 [](const auto& a, const auto& b) { return a.time < b.time; });
    }
    
    void update(u_time currentTime) {
        config.currentTime = currentTime;
        
        for (auto& [param, points] : automations) {
            if (points.empty()) continue;
            
            // 查找当前时间点
            auto it = std::upper_bound(points.begin(), points.end(), currentTime,
                                      [](u_time t, const AutomationPoint& p) {
                                          return t < p.time;
                                      });
            
            if (it == points.begin()) {
                // 在第一个点之前，使用第一个点的值
                setParameter(param, points.front().value);
            } else if (it == points.end()) {
                // 在最后一个点之后，使用最后一个点的值
                setParameter(param, points.back().value);
            } else {
                // 在两个点之间，线性插值
                auto prev = it - 1;
                auto next = it;
                float t = (currentTime - prev->time) / (next->time - prev->time);
                float value = prev->value + (next->value - prev->value) * t;
                setParameter(param, value);
            }
        }
    }
    
private:
    void setParameter(const std::string& param, float value) {
        if (param == "volume") config.Volume = value;
        else if (param == "pan") config.Pan = value;
        else if (param == "expression") config.Expression = value;
        else if (param == "modulation") config.Modulation = value;
        // ... 其他参数
    }
};
```

### 多通道配置管理

```cpp
class ChannelPresetManager {
private:
    struct ChannelPreset {
        std::string name;
        ChannelConfig config;
        std::shared_ptr<InstrumentProvider> instrument;
        std::shared_ptr<NoteTuning> tuning;
        float reverbAmount = 0.0f;
        float chorusAmount = 0.0f;
        float phaserAmount = 0.0f;
    };
    
    std::map<std::string, ChannelPreset> presets;
    
public:
    void savePreset(const std::string& name, 
                   const IChannel& channel,
                   const std::string& instrumentPath = "") {
        ChannelPreset preset;
        preset.name = name;
        preset.config.set(channel.getConfig());
        
        if (!instrumentPath.empty()) {
            preset.instrument = std::make_shared<SF2FormatInstrument>(
                yzrilyzr_io::FileInputStream(instrumentPath)
            );
        }
        
        presets[name] = preset;
    }
    
    void loadPreset(const std::string& name, IChannel& channel) {
        if (presets.find(name) == presets.end()) {
            throw std::runtime_error("Preset not found: " + name);
        }
        
        auto& preset = presets[name];
        channel.getConfig().set(preset.config);
        
        if (preset.instrument) {
            channel.getConfig().setInstrumentProvider(preset.instrument);
        }
        
        if (preset.tuning) {
            channel.getConfig().setNoteTuning(preset.tuning);
        }
        
        channel.setReverb(preset.reverbAmount);
        channel.setChorus(preset.chorusAmount);
        channel.setPhaser(preset.phaserAmount);
        channel.setName(preset.name);
    }
    
    std::vector<std::string> getPresetNames() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : presets) {
            names.push_back(name);
        }
        return names;
    }
};
```

## 故障排除

### 常见问题

1. **配置不生效**
   ```cpp
   // 确保设置了上下文
   config.setContext(mixer, channel);
   
   // 确保采样率已设置
   config.sampleRate = mixer->getSampleRate();
   ```

2. **音符状态异常**
   ```cpp
   // 检查音符映射
   for (int i = 0; i < 128; i++) {
       if (config.noteHoldMap[i]) {
           // 音符i仍在按下状态
       }
   }
   
   // 强制释放所有音符
   config.allNotesOff();
   ```

3. **效果器无声音**
   ```cpp
   // 检查效果器是否初始化
   if (config.dsp3d) {
       config.dsp3d->init(config.sampleRate);
   }
   
   // 检查效果器发送量
   config.Expression = 1.0f;  // 确保表情控制器打开
   ```

4. **RPN/NRPN不响应**
   ```cpp
   // 检查是否启用了MIDI控制
   channel->ENABLE_MIDI_CHANNEL_CONTROL = true;
   
   // 重置RPN/NRPN状态
   config.rpn.reset();
   config.nrpn.reset();
   ```

### 调试输出

```cpp
void debugChannelConfig(const ChannelConfig& config) {
    std::cout << "=== Channel Config Debug ===" << std::endl;
    std::cout << "Volume: " << config.Volume << std::endl;
    std::cout << "Pan: " << config.Pan << std::endl;
    std::cout << "Expression: " << config.Expression << std::endl;
    std::cout << "Modulation: " << config.Modulation << std::endl;
    std::cout << "PitchBend: " << config.ChannelPitchBend << std::endl;
    std::cout << "Sustain: " << config.Sustain << std::endl;
    std::cout << "MonoMode: " << config.MonoMode << std::endl;
    std::cout << "Active Notes: ";
    
    int activeCount = 0;
    for (int i = 0; i < 128; i++) {
        if (config.noteHoldMap[i]) {
            std::cout << i << " ";
            activeCount++;
        }
    }
    std::cout << "(" << activeCount << " total)" << std::endl;
    
    std::cout << "Instrument: " << (config.instrument ? "Loaded" : "None") << std::endl;
    std::cout << "Tuning: " << (config.tuning ? "Loaded" : "None") << std::endl;
    std::cout << "3D Effect: " << (config.dsp3d ? "Enabled" : "Disabled") << std::endl;
}
```
