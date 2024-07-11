#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class MainPopup : public geode::Popup<std::string const&> {
protected:
    InputNode* m_offsetInput;
    cocos2d::CCLabelBMFont* m_currentFileLabel;

    std::vector<std::pair<bool, std::vector<double>>> getTrackAttacks(std::string filePath);
    bool setup(std::string const& value) override;
    void onClose(cocos2d::CCObject*) override;
    void onSettings(CCObject* sender);
    void onSelectFile(CCObject* sender);
public:
    static MainPopup* create();
};
