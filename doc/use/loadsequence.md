`[PlatformName]` 支持MIDI以及XM格式的序列加载

## MIDI

* 支持MID MIDS RIFF等格式的加载
```cpp
// 解析MIDI文件（支持.mid/.rmid/.mids格式）
std::shared_ptr<MixerSequence> midiSeq = SynthUtil::parseMIDI(
    FileInputStream("path/to/your/music.mid")
);
if (midiSeq) {
    // 1秒后开始播放，事件组命名为"MyMIDI"（便于后续管理）
    midiSeq->postToMixer(mixer.get(), 1.0, "MyMIDI");
}
```


## XM (External Module)
```cpp
// 示例3：从XM文件加载事件序列（Fasttracker 2模块文件）
std::shared_ptr<MixerSequence> xmSeq = SynthUtil::parseXM(
    FileInputStream("path/to/your/tune.xm")
);
if (xmSeq) {
    xmSeq->postToMixer(mixer.get(), 0.0, "MyXM"); // 立即播放
}
```
