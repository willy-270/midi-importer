#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "MainPopup/MainPopup.hpp"

using namespace geode::prelude;

MainPopup* mainPopup; //extern
CCMenuItemSpriteExtra* btn;

//want this to be in LevelSettingsLayer but no init :(:(:(:(:((((
class $modify(mEditorUI, EditorUI) {
    struct Fields {
        MainPopup* mainPopup;
    };

    $override
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

		auto menu = CCMenu::create();
		auto spr = ButtonSprite::create("MIDI");
        spr->setScale(.4f);
        btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(mEditorUI::onClick)
        );
		menu->addChild(btn);
		this->addChild(menu, 1000);
		btn->setPosition(-260.f, -58.f);

		return true;
    }

    void onClick(CCObject* sender) {
        mainPopup = MainPopup::create();
        mainPopup->show();
    }

    void onPlaytest(CCObject* sender) {
        EditorUI::onPlaytest(sender);
        btn->setVisible(!btn->isVisible());
    }

    //the onStopPlaytest in EditorUI doesnt get called on enter key 
};

class $modify(LevelEditorLayer) {
    $override
    void onStopPlaytest() {
        LevelEditorLayer::onStopPlaytest();
        btn->setVisible(true);
    }
};