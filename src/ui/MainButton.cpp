#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSettingsLayer.hpp>

#include "MainPopup/MainPopup.hpp"

using namespace geode::prelude;

MainPopup* mainPopup; //extern
CCMenuItemSpriteExtra* btn;

class $modify(mLevelSettingsLayer, LevelSettingsLayer) {
    struct Fields {
        MainPopup* mainPopup;
    };

    $override
    bool init(LevelSettingsObject* levelSettingsObjects, LevelEditorLayer* editorLayer) {
        if (!LevelSettingsLayer::init(levelSettingsObjects, editorLayer)) {
            return false;
        }

		auto spr = ButtonSprite::create("MIDI");
        spr->setScale(.5f);
        btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(mLevelSettingsLayer::onClick)
        );
		this->m_buttonMenu->addChild(btn);
		btn->setPosition(-185.f, -90.f);

		return true;
    }

    void onClick(CCObject* sender) {
        mainPopup = MainPopup::create();
        mainPopup->mLevelSettingsLayer = this;
        mainPopup->show();
    }
};