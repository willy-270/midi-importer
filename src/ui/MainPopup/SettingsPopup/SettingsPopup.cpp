#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include "SettingsPopup.hpp"
#include "../../../main.hpp"
#include "../../MainButton.hpp"
#include "../../../color_generator/colors.hpp"

std::vector<CCMenuItemToggler*> btns; //rly weird behaviour when in class fields

bool SettingsPopup::setup(std::string const& value) {
    btns.clear();

    this->setTitle("Toggle Track Visibility");

    mainPopup->setVisible(false);
    m_bgSprite->setOpacity(0);

    if (currentMidiData.tracks.empty()) {
        auto label = CCLabelBMFont::create(" Open a midi file!", "bigFont.fnt");
        auto menu = CCMenu::create();
        menu->addChild(label);
        m_mainLayer->addChild(menu);
        return true;
    }

    //thanks betteredit repo

    auto scroll = ScrollLayer::create(ccp(320, 200));
    scroll->setAnchorPoint(ccp(0, 0));
    scroll->setPosition(CCDirector::get()->getWinSize() / 2 - scroll->getContentSize() / 2 - ccp(0, 10));

    auto bar = Scrollbar::create(scroll);
    bar->setPosition(ccp(180, -10) + CCDirector::sharedDirector()->getWinSize() / 2);
    m_mainLayer->addChild(bar);

    auto content = scroll->m_contentLayer;

    int trackCount = currentMidiData.tracks.size();
    content->setContentSize(ccp(320, 30 * (trackCount)));

    std::vector<std::tuple<int, int, int, int>> colors = generateColors(currentMidiData.tracks.size());

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

        auto trackText = CCLabelBMFont::create(("Track " + std::to_string(i + 1) + ": " + currentMidiData.tracks[i].instrumentName).c_str(), "bigFont.fnt");
        trackText->setAnchorPoint(ccp(0, 0.5f));
        trackText->setScale(0.4f);
        trackText->setPosition(ccp(10, 15));
        trackText->setColor(ccc3(std::get<0>(colors[i]), std::get<1>(colors[i]), std::get<2>(colors[i])));
        menu->addChild(trackText);

        auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

        auto btn = CCMenuItemToggler::create(offSpr, onSpr, menu, menu_selector(SettingsPopup::onToggle));
        btn->toggle(currentMidiData.tracks[i].visible);
        btn->setPosition(node->getContentSize().width - 20, 15);
        btn->setScale(0.8f);
        menu->addChild(btn);
        btns.push_back(btn);

        node->addChild(menu);
        content->addChild(node);
    }

    auto hideAllSpr = ButtonSprite::create("Hide All");
    hideAllSpr->setScale(0.8f);
    auto hideAllBtn = CCMenuItemSpriteExtra::create(
        hideAllSpr,
        this,
        menu_selector(SettingsPopup::onHideAll)
    );
    hideAllBtn->setPosition(-80, -130);

    auto showAllSpr = ButtonSprite::create("Show All");
    showAllSpr->setScale(0.8f);
    auto showAllBtn = CCMenuItemSpriteExtra::create(
        showAllSpr,
        this,
        menu_selector(SettingsPopup::onShowAll)
    );
    showAllBtn->setPosition(80, -130);

    auto menu = CCMenu::create();
    menu->addChild(hideAllBtn);
    menu->addChild(showAllBtn);

    scroll->moveToTop();
    m_mainLayer->addChild(scroll);
    m_mainLayer->addChild(menu);
    handleTouchPriority(m_mainLayer);

    return true;
}

void SettingsPopup::onHideAll(CCObject* sender) {
    for (auto btn : btns) {
        btn->toggle(false);
    }

    for (auto& track : currentMidiData.tracks) {
        track.visible = false;
    }
}

void SettingsPopup::onShowAll(CCObject* sender) {
    for (auto btn : btns) {
        btn->toggle(true);
    }

    for (auto& track : currentMidiData.tracks) {
        track.visible = true;
    }
}

void SettingsPopup::onToggle(CCObject* sender) {
    auto btn = static_cast<CCMenuItemToggler*>(sender);
    
    for (int i = 0; i < btns.size(); i++) {
        if (btns[i] == btn) {
            currentMidiData.tracks[i].visible = !btn->isToggled();
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