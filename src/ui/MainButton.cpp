#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "MainPopup/MainPopup.hpp"

using namespace geode::prelude;

class $modify(mLevelEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelEditorLayer::init(p0, p1)) {
            return false;
        }

		auto menu = CCMenu::create();
		auto spr = ButtonSprite::create("MIDI");
        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(mLevelEditorLayer::onClick)
        );
		menu->addChild(btn);
		this->addChild(menu, 1000);
		btn->setPosition(-195.f, -70.f);

		return true;
    }

    void onClick(CCObject* sender) {
        auto popup = MainPopup::create();
        popup->show();
    }
};