#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include "SettingsPopup.hpp"
#include "../../../main.hpp"
#include "../../MainButton.hpp"
#include "../../../color_generator/colors.hpp"

std::vector<CCMenuItemToggler*> btns; //this wont work in the class feilds for some reason caused an hour of confusion !!!!!!!!!

bool SettingsPopup::setup(std::string const& value) {
    btns.clear();

    this->setTitle("Toggle Track Visibility");

    mainPopup->setVisible(false);
    m_bgSprite->setOpacity(0);

    if (tracks.empty()) {
        auto label = CCLabelBMFont::create(" Open a midi file!", "bigFont.fnt");
        auto menu = CCMenu::create();
        menu->addChild(label);
        m_mainLayer->addChild(menu);
        return true;
    }

    auto scroll = ScrollLayer::create(ccp(320, 200));
    scroll->setAnchorPoint(ccp(0, 0));
    scroll->setPosition(CCDirector::get()->getWinSize() / 2 - scroll->getContentSize() / 2 - ccp(0, 10));

    auto bar = Scrollbar::create(scroll);
    bar->setPosition(ccp(180, -10) + CCDirector::sharedDirector()->getWinSize() / 2);
    m_mainLayer->addChild(bar);

    auto content = scroll->m_contentLayer;

    int trackCount = tracks.size();
    content->setContentSize(ccp(320, 30 * (trackCount)));

    std::vector<std::tuple<int, int, int, int>> colors = generateColors(tracks.size());

    for (int i = 0; i < trackCount; i++) {
        auto node = CCLayerColor::create();
        node->setColor({0, 0, 0});
        node->setOpacity((i % 2 == 0) ? 100 : 50);
        node->setContentSize(ccp(320, 30));
        node->setPosition(ccp(0, 30 * trackCount - ((i + 1) * 30)));

        auto menu = CCMenu::create();
        menu->setPosition(ccp(0, 0));
        menu->setContentSize({0, 0});
        menu->ignoreAnchorPointForPosition(false);
        menu->setAnchorPoint(ccp(0, 0));

        auto trackText = CCLabelBMFont::create(("Track " + std::to_string(i + 1)).c_str(), "bigFont.fnt");
        trackText->setAnchorPoint(ccp(0, 0.5f));
        trackText->setScale(0.65f);
        trackText->setPosition(ccp(10, 15));
        trackText->setColor(ccc3(std::get<0>(colors[i]), std::get<1>(colors[i]), std::get<2>(colors[i])));
        menu->addChild(trackText);

        auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

        auto btn = CCMenuItemToggler::create(offSpr, onSpr, menu, menu_selector(SettingsPopup::onToggle));
        btn->toggle(tracks[i].first);
        btn->setPosition(node->getContentSize().width - 20, 15);
        btn->setScale(0.8f);
        menu->addChild(btn);
        btns.push_back(btn);

        node->addChild(menu);
        content->addChild(node);
    }

    scroll->moveToTop();
    m_mainLayer->addChild(scroll);
    handleTouchPriority(m_mainLayer);

    return true;
}

void SettingsPopup::onToggle(CCObject* sender) {
    auto btn = static_cast<CCMenuItemToggler*>(sender);
    
    for (int i = 0; i < btns.size(); i++) {
        if (btns[i] == btn) {
            tracks[i].first = !btn->isToggled();
            break;
        }
    }
}

void SettingsPopup::onClose(cocos2d::CCObject*) {
    mainPopup->setVisible(true);

    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

SettingsPopup* SettingsPopup::create() {
    auto ret = new SettingsPopup();
    if (ret->init(400.f, 250.f, "")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}