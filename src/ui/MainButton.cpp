#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

#include "MainPopup/MainPopup.hpp"

using namespace geode::prelude;

MainPopup* mainPopup;

//want this to be in LevelSettingsLayer but no init :(:(:(:(:((((
class $modify(mEditorUI, EditorUI) {
    struct Fields {
        MainPopup* mainPopup;
        CCMenuItemSpriteExtra* btn;
    };

    $override
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

		auto menu = CCMenu::create();
		auto spr = ButtonSprite::create("MIDI");
        spr->setScale(.4f);
        m_fields->btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(mEditorUI::onClick)
        );
		menu->addChild(m_fields->btn);
		this->addChild(menu, 1000);
		m_fields->btn->setPosition(-260.f, -58.f);

		return true;
    }

    $override
    void onPlaytest(CCObject* sender) {
        EditorUI::onPlaytest(sender);
        m_fields->btn->setVisible(!m_fields->btn->isVisible());
    }

    $override
    void onStopPlaytest(CCObject* sender) {
        EditorUI::onStopPlaytest(sender);
        m_fields->btn->setVisible(true);
    }

    void onClick(CCObject* sender) {
        log::debug("Clicked MIDI button");
        mainPopup = MainPopup::create();
        mainPopup->show();
    }
};