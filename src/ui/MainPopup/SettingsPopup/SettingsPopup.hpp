#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class SettingsPopup : public geode::Popup<std::string const&> {
protected:
    bool setup(std::string const& value) override;
    void onClose(cocos2d::CCObject*) override;
    void onHide(CCObject* sender);
    void onToggle(CCObject* sender);
    void onShowAll(CCObject* sender);
    void onHideAll(CCObject* sender);
    void onPlace(CCObject* sender);
    void onInfo(CCObject* sender);
    void performPlace(int trackIdx);
public:
    static SettingsPopup* create();
};
