#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/LevelSettingsLayer.hpp>

#include "../../main.hpp"

using namespace geode::prelude;

class MainPopup : public geode::Popup<std::string const&> {
protected:
    TextInput* m_offsetInput;
    cocos2d::CCLabelBMFont* m_currentFileLabel;
    LevelMidiData getMidiData(std::string filePath);
    bool setup(std::string const& value) override;

    void onSettings(CCObject* sender);
    void onSelectFile(CCObject* sender);
public:
    void onClose(cocos2d::CCObject*) override;
    static MainPopup* create();
    LevelSettingsLayer* mLevelSettingsLayer;
};