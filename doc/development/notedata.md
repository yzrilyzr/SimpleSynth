`NoteData` 提供了为音符对象`Note`存储更复杂数据的便捷管理

对于需要向音符存储自定义数据的复杂处理器：

## 定义音符数据类
```cpp
EBCLASS(MyKeyData){
public:
    //在这里存储数据
    yzrilyzr_dsp::RingBufferSample ringBuffer;
    std::shared_ptr<yzrilyzr_dsp::IIR> filter = nullptr;
};
```

## 继承NoteData模板并实现初始化
```cpp
ECLASS(MyProcessor, public Osc, NoteData<MyKeyData>){
    // 实现init方法初始化音符数据
    MyKeyData* init(MyKeyData* data, Note& note) override{
        if(data == nullptr){
            //创建新的数据并初始化
            data = new MyKeyData();
            data->filter = std::make_shared<yzrilyzr_dsp::BiquadIIR>();
        }
        //在这里可根据note初始化data内部数据
        yzrilyzr_dsp::IIRUtil::biquad(*data -> filter,1000.0, note.cfg->sampleRate, 0.707, yzrilyzr_dsp::FilterPassType::LOWPASS);
        return data;
    }
};
```

## 获取音符数据

```cpp
u_sample getAmp(Note & note) override{
    MyKeyData &data=*getData(note);
}
```