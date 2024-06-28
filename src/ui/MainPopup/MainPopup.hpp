#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class MainPopup : public geode::Popup<std::string const&> {
protected:
    InputNode* m_offsetInput;
    cocos2d::CCLabelBMFont* m_currentFileLabel;

    bool setup(std::string const& value) override;
    void onSelectFile(CCObject* sender);
    void onUpdate(CCObject* sender);
public:
    static MainPopup* create();
};
