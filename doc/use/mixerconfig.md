## 缓冲区大小

缓冲区大小（Buffer Size）是音频处理中最重要的参数之一，直接影响系统的延迟和CPU占用率。

```cpp
// 获取当前缓冲区大小（单位：帧）
u_index bufferSize = mixer->getBufferSize();

// 设置缓冲区大小
mixer->setBufferSize(256);  // 设置为256帧
```

### 参数选择建议

| 缓冲区大小 | 延迟（48kHz） | 适用场景 |
|-----------|--------------|----------|
| 64帧 | 1.33ms | 专业录音/低延迟演奏 |
| 128帧 | 2.67ms | 实时演奏/专业制作 |
| 256帧 | 5.33ms | 通用音频处理（推荐） |
| 512帧 | 10.67ms | 游戏音频/多媒体 |
| 1024帧 | 21.33ms | 渲染/离线处理 |
| 2048帧 | 42.67ms | CPU性能受限环境 |

**重要提示**：
- 修改缓冲区大小会重置所有通道状态
- 实时音频应用中，缓冲区大小需与音频驱动设置匹配
- 过小的缓冲区可能导致音频卡顿，过大的缓冲区会增加延迟

## 设置采样率

采样率决定音频的频率响应范围和音质。

```cpp
// 获取当前采样率
u_sample_rate sr = mixer->getSampleRate();

// 设置采样率（常见值）
mixer->setSampleRate(44100);   // CD音质
mixer->setSampleRate(48000);   // 专业音频标准（推荐）
mixer->setSampleRate(96000);   // 高分辨率音频
mixer->setSampleRate(192000);  // 超高分辨率
```

### 采样率选择指南

1. **44.1kHz** 
   - 音乐CD标准
   - 适合最终发布内容
   - 兼容性最好

2. **48kHz**
   - 影视/游戏行业标准
   - 推荐用于实时处理
   - 平衡音质与性能

3. **96kHz/192kHz**
   - 专业录音/混音
   - 需要高质量音源
   - CPU占用增加2-4倍

**注意**：更改采样率会触发合成器重置，所有音符和效果器状态将被清除。

## 合成器线程模式

IMixer提供三种线程模式以适应不同的性能需求。

### 单线程模式 (MODE_SINGLE_THREAD)

```cpp
mixer->setSynthMode(IMixer::MODE_SINGLE_THREAD, 0);
```

**特点**：
- 所有处理在调用线程完成
- 无线程同步开销
- 适合简单应用或性能测试

### 线程池模式 (MODE_THREAD_POOL)

```cpp
// 自动分配CPU核心数
mixer->setSynthMode(IMixer::MODE_THREAD_POOL, -1);

// 指定线程数（如4个线程）
mixer->setSynthMode(IMixer::MODE_THREAD_PREAD_POOL, 4);
```

**特点**：
- 通道级并行处理
- 自动负载均衡
- 适合多核CPU和复杂音色

### 异步任务模式 (MODE_FUTURE)

```cpp
mixer->setSynthMode(IMixer::MODE_FUTURE, 0);
```

**特点**：
- 使用std::future异步执行
- 非阻塞调用
- 适合与GUI框架集成

### 异步合成控制

```cpp
// 启动异步合成
mixer->asyncMix();

// 等待合成完成
mixer->awaitMix();

// 检查是否在异步合成中
auto& future = mixer->mixFuture;
if (future.has_value() && future->valid()) {
    // 异步操作进行中
}
```

## 重置合成器

重置功能用于清除所有状态，恢复到初始配置。

```cpp
// 完全重置合成器
mixer->reset();
```

**重置操作包括**：
1. 停止所有发音的音符
2. 清除所有事件队列
3. 重置所有DSP效果器状态
4. 恢复通道默认配置
5. 清除时间计数器

**使用场景**：
- 切换工程/音色时
- 处理音频异常时
- 需要完全静音时

## 使用均衡器

全局均衡器可调整整体音色平衡。

```cpp
// 启用/禁用全局EQ
mixer->setUseEQ(true);   // 启用
mixer->setUseEQ(false);  // 禁用

// 检查EQ状态
bool eqEnabled = mixer->isUseEQ();

// 获取EQ处理链（可添加自定义滤波器）
std::shared_ptr<yzrilyzr_dsp::DSPChain>* eqChain = mixer->getEQ();
```

## 使用主输出限制器

限制器防止音频过载和削波失真。

```cpp
// 启用/禁用限制器
mixer->setUseLimiter(true);   // 启用（默认）
mixer->setUseLimiter(false);  // 禁用

// 检查限制器状态
bool limiterEnabled = mixer->isUseLimiter();

// 重置限制器状态（释放压缩）
mixer->resetLimiter();
```

**限制器特性**：
- 自动增益控制
- 峰值保护（-0.1dB阈值）
- 软拐点处理
- 实时响应急剧瞬变

## 获取处理时间占用

监控CPU使用情况，优化性能。

```cpp
// 获取上一次mix()调用的实际处理时间（秒）
u_time_f actualTime = mixer->getProcessTime();

// 获取理论最大处理时间（基于缓冲区大小）
u_time_f maxTime = mixer->getProcessStandardTime();

// 计算CPU占用率
float cpuUsage = (actualTime / maxTime) * 100.0f;

std::cout << "实际处理: " << actualTime * 1000 << "ms" << std::endl;
std::cout << "理论最大: " << maxTime * 1000 << "ms" << std::endl;
std::cout << "CPU占用: " << cpuUsage << "%" << std::endl;
```

**性能调优建议**：
- 保持CPU占用率<70%以确保稳定性
- 若占用过高，考虑：
  - 增加缓冲区大小
  - 减少复音数
  - 简化DSP效果
  - 启用线程池模式

## 获取事件队列排队的个数

监控事件系统负载。

```cpp
// 获取待处理事件数量
u_index pendingEvents = mixer->getPostedEventCount();

// 实时监控
while (isPlaying) {
    mixer->mix();
    u_index events = mixer->getPostedEventCount();
    updateEventCounterUI(events);
}
```

## 获取当前处理音符数量

监控合成器负载。

```cpp
// 获取当前正在发音的音符数量
u_index activeNotes = mixer->getCurrentProcessingNoteCount();

// 根据复音数调整策略
u_index maxPolyphony = 64;  // 硬件/软件限制

if (activeNotes >= maxPolyphony) {
    // 触发音符窃取或警告
    std::cout << "达到最大复音数: " << activeNotes << std::endl;
}

// 动态显示
void updatePolyphonyDisplay() {
    u_index notes = mixer->getCurrentProcessingNoteCount();
    polyphonyLabel->setText("复音数: " + std::to_string(notes));
}
```

## 判断是否有正在合成的数据

检查音频活动状态。

```cpp
// 检查是否有音频数据正在合成
bool hasAudioData = mixer->hasData();

// 应用场景1：自动静音检测
if (!mixer->hasData()) {
    // 无音频活动超过阈值
    idleTimer++;
    if (idleTimer > 600) {  // 10秒无活动
        enterSleepMode();
    }
} else {
    idleTimer = 0;
}

// 应用场景2：节能优化
void audioCallback() {
    if (mixer->hasData()) {
        mixer->mix();  // 正常处理
    } else {
        // 输出静音，降低CPU频率
        outputSilence();
    }
}
```

## 通道DSP控制

控制单个通道是否使用DSP效果器。

```cpp
// 全局启用/禁用通道DSP
mixer->setChannelUseDSP(true);   // 启用（默认）
mixer->setChannelUseDSP(false);  // 禁用所有通道DSP

// 检查状态
bool dspEnabled = mixer->isChannelUseDSP();

// 获取DSP锁（线程安全操作）
{
    std::shared_lock lock(mixer->getDSPLock());
    // 安全地读取DSP配置
}

{
    std::unique_lock lock(mixer->getDSPLock());
    // 安全地修改DSP配置
}
```

## 全局配置管理

访问和修改全局通道配置。

```cpp
// 获取全局配置引用
ChannelConfig& config = mixer->getGlobalConfig();

// 修改全局参数
config.Volume = 0.8f;      // 主音量80%
config.Pan = 0.0f;         // 居中声像
config.ChannelPitchBend = 0.0f;   // 无弯音
config.Modulation = 0.1f;  // 轻度调制
```

## 线程安全锁

IMixer提供细粒度锁机制，支持并发访问。

```cpp
// 1. 事件系统锁
std::shared_mutex& eventLock = mixer->getEventLock();
{
    std::unique_lock lock(eventLock);
    mixer->postEvent(new NoteOn(0, 60, 100), currentTime);
    // 事件操作线程安全
}

// 2. 通道管理锁
std::shared_mutex& channelLock = mixer->getChannelLock();
{
    std::shared_lock lock(channelLock);  // 读锁
    auto channels = mixer->getAllChannels();
    // 安全读取通道信息
}

// 3. DSP配置锁
std::shared_mutex& dspLock = mixer->getDSPLock();
{
    std::unique_lock lock(dspLock);
    // 安全修改DSP链
}
```

## 故障排除

### 常见问题

1. **音频卡顿/爆音**
   ```cpp
   // 增大缓冲区
   mixer->setBufferSize(512);
   // 或减少复音数
   ```

2. **延迟过高**
   ```cpp
   // 减小缓冲区，启用线程池
   mixer->setBufferSize(128);
   mixer->setSynthMode(IMixer::MODE_THREAD_POOL, 4);
   ```

3. **CPU占用过高**
   ```cpp
   // 检查处理时间
   u_time_f time = mixer->getProcessTime();
   u_time_f maxTime = mixer->getProcessStandardTime();
   if (time > maxTime * 0.8f) {
       // 考虑禁用部分DSP或EQ
       mixer->setUseEQ(false);
       mixer->setChannelUseDSP(false);
   }
   ```

4. **内存泄漏检查**
   ```cpp
   // 定期检查事件队列积压
   if (mixer->getPostedEventCount() > 5000) {
       // 可能事件未及时处理
       mixer->reset();  // 紧急重置
   }
   ```

