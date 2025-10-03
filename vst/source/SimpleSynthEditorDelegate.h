// 在 controller.h 或单独的头文件中
#include "vstgui/plugin-bindings/vst3editor.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/cviewcontainer.h"
#include "vstgui/uidescription/uiattributes.h"
#include "util/Lang.h"
#include "MultiColumnProgramSelector.h"
using namespace VSTGUI;

class SimpleSynthEditorDelegate : public VST3EditorDelegate{
    private:
    Steinberg::Vst::EditController * controller;
    yzrilyzr_util::Lang & langManager;

    public:
    SimpleSynthEditorDelegate(Steinberg::Vst::EditController * ctrl, yzrilyzr_util::Lang & lang)
        : controller(ctrl), langManager(lang){}

    // 创建自定义视图
    CView * createCustomView(UTF8StringPtr name, const UIAttributes & attributes,
                             const IUIDescription * description, VST3Editor * editor) override{

        if(strcmp(name, "MultiColumnProgramSelector") == 0){
            // 从属性中获取尺寸
            CRect size;
            if(attributes.getRectAttribute("size", size)){
                // 从属性中获取基础标签
                int32_t baseTag=10000; // 默认值
                attributes.getIntegerAttribute("base-tag", baseTag);

                // 创建多列程序选择器
                return new MultiColumnProgramSelector(size, (VSTGUI::IControlListener * )controller,
                                                      static_cast<Steinberg::Vst::ParamID>(baseTag), langManager);
            }
        }
        return nullptr;
    }

    CView * verifyView(CView * view, const UIAttributes & attributes,
                       const IUIDescription * description, VST3Editor * editor) override{
        return view;
    }

    bool findParameter(const CPoint & pos, Steinberg::Vst::ParamID & paramID,
                       VST3Editor * editor) override{
        return false;
    }

    bool isPrivateParameter(const Steinberg::Vst::ParamID paramID) override{
        return false;
    }

    void didOpen(VST3Editor * editor) override{}
    void willClose(VST3Editor * editor) override{}

    COptionMenu * createContextMenu(const CPoint & pos, VST3Editor * editor) override{
        return nullptr;
    }

    IController * createSubController(UTF8StringPtr name, const IUIDescription * description,
                                      VST3Editor * editor) override{
        return nullptr;
    }

    void onZoomChanged(VST3Editor * editor, double newZoom) override{}
};