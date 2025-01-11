#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include "SettingsPopup.hpp"
#include "../../../main.hpp"
#include "../../MainButton.hpp"
#include "../../../color_generator/colors.hpp"

//rly weird behaviour when these are in class fields
std::vector<CCMenuItemToggler*> toggleBtns; 
std::vector<CCMenuItemSpriteExtra*> placeBtns;

bool SettingsPopup::setup(std::string const& value) {
    toggleBtns.clear();
    placeBtns.clear();

    this->setTitle("Toggle Track Visibility");

    mainPopup->setVisible(false);
    mainPopup->mLevelSettingsLayer->setVisible(false);
    m_bgSprite->setOpacity(0);

    if (currentMidiData.tracks.empty()) {
        auto label = CCLabelBMFont::create("Open a midi file!", "bigFont.fnt");
        auto menu = CCMenu::create();
        menu->setPosition(200.f, 140.f);
        menu->addChild(label);
        m_mainLayer->addChild(menu);
        return true;
    }

    //thanks betteredit repo

    auto scroll = ScrollLayer::create(ccp(320, 200));
    scroll->setAnchorPoint(ccp(0, 0));
    scroll->setPosition(40, 10);

    auto bar = Scrollbar::create(scroll);
    bar->setPosition(370, 100);
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
        auto toggleButton = CCMenuItemToggler::create(offSpr, onSpr, menu, menu_selector(SettingsPopup::onToggle));
        toggleButton->toggle(currentMidiData.tracks[i].visible);
        toggleButton->setPosition(node->getContentSize().width - 20, 15);
        toggleButton->setScale(0.8f);
        menu->addChild(toggleButton);
        toggleBtns.push_back(toggleButton);

        auto placeSpr = CCSprite::createWithSpriteFrameName("GJ_plus2Btn_001.png");
        placeSpr->setScale(0.8f);
        auto placeBtn = CCMenuItemSpriteExtra::create(placeSpr, this, menu_selector(SettingsPopup::onPlace));
        placeBtn->setPosition(node->getContentSize().width -50, 15);
        menu->addChild(placeBtn);
        placeBtns.push_back(placeBtn);

        node->addChild(menu);
        content->addChild(node);
    }

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.75f);
    auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(SettingsPopup::onInfo));
    infoBtn->setPosition(-120, 108);

    auto hideAllSpr = ButtonSprite::create("Hide All");
    hideAllSpr->setScale(0.8f);
    auto hideAllBtn = CCMenuItemSpriteExtra::create(hideAllSpr, this, menu_selector(SettingsPopup::onHideAll));
    hideAllBtn->setPosition(-80, -130);

    auto showAllSpr = ButtonSprite::create("Show All");
    showAllSpr->setScale(0.8f);
    auto showAllBtn = CCMenuItemSpriteExtra::create(showAllSpr, this, menu_selector(SettingsPopup::onShowAll));
    showAllBtn->setPosition(80, -130);

    auto menu = CCMenu::create();
    menu->setPosition(200.f, 120.f);
    menu->addChild(infoBtn);
    menu->addChild(hideAllBtn);
    menu->addChild(showAllBtn);

    scroll->moveToTop();
    m_mainLayer->addChild(scroll);
    m_mainLayer->addChild(menu);
    handleTouchPriority(m_mainLayer);

    return true;
}

void SettingsPopup::onHideAll(CCObject* sender) {
    for (auto btn : toggleBtns) {
        btn->toggle(false);
    }
    for (auto& track : currentMidiData.tracks) {
        track.visible = false;
    }
}

void SettingsPopup::onShowAll(CCObject* sender) {
    for (auto btn : toggleBtns) {
        btn->toggle(true);
    }
    for (auto& track : currentMidiData.tracks) {
        track.visible = true;
    }
}

void SettingsPopup::onToggle(CCObject* sender) {
    auto btn = static_cast<CCMenuItemToggler*>(sender);
    
    for (int i = 0; i < toggleBtns.size(); i++) {
        if (toggleBtns[i] == btn) {
            currentMidiData.tracks[i].visible = !btn->isToggled();
            break;
        }
    }
}

void SettingsPopup::onPlace(CCObject* sender) {
    auto* editor = GameManager::sharedState()->getEditorLayer();
    auto* editorUI = editor->m_editorUI;

    if (editorUI->m_selectedObjects->count() != 2) {
        FLAlertLayer::create(
            "Error",
            "You must have exactly <cy>2</c> objects selected! "
            "The first object will then be duplicated along the track guildeines that fall inbetween the first and second selected objects.",
            "OK"
        )->show();
        return;
    }
   
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    int trackIdx;
    for (int i = 0; i < placeBtns.size(); i++) {
        if (placeBtns[i] == btn) {
            trackIdx = i; 
            break;
        }
    }

    auto firstObjXPos = static_cast<GameObject*>(editorUI->m_selectedObjects->objectAtIndex(0))->getPositionX();
    auto secondObjXPos = static_cast<GameObject*>(editorUI->m_selectedObjects->objectAtIndex(1))->getPositionX();
    if (firstObjXPos > secondObjXPos) {
        std::swap(firstObjXPos, secondObjXPos);
    }

    int objsToPlace = 0;
    for (float xPos : xPositions[trackIdx]) {
        if (xPos < firstObjXPos || xPos > secondObjXPos) {
            continue;
        }
        objsToPlace++;
    }

    if (objsToPlace == 0) {
        FLAlertLayer::create(
            "Error",
            "No guidelines of the selected track fall between the selected objects.",
            "OK"
        )->show();
        return;
    }
    
    geode::createQuickPopup(
        "Confirmation",          
        "This will create " + std::to_string(objsToPlace) + " objects. Are you sure?",   
        "No", "Yes",      
        [this, trackIdx](auto, bool btn2) {
            if (btn2) {
                performPlace(trackIdx);
            }
        }
    );
}

void SettingsPopup::performPlace(int trackIdx) {
    auto* editor = GameManager::sharedState()->getEditorLayer();
    auto* editorUI = editor->m_editorUI;

    auto selectedObjs = editorUI->m_selectedObjects;

    auto firstObj = static_cast<GameObject*>(selectedObjs->objectAtIndex(0));
    auto secondObj = static_cast<GameObject*>(selectedObjs->objectAtIndex(1));
    if (firstObj->getPositionX() > secondObj->getPositionX()) {
        std::swap(firstObj, secondObj);
    }

    editorUI->selectObject(firstObj, true);

    auto pastedObjs = CCArray::create();
    for (float xPos : xPositions[trackIdx]) {
        if (xPos < firstObj->getPositionX() || xPos > secondObj->getPositionX()) {
            continue;
        }
        editorUI->onDuplicate(nullptr);
        auto pastedObj = editorUI->m_selectedObject;
        pastedObjs->addObject(pastedObj);
        editorUI->moveObject(pastedObj, ccp(xPos - pastedObj->getPositionX(), 0));
        editor->m_undoObjects->removeLastObject();
    }
    editorUI->selectObject(firstObj, true);
    editorUI->onDeleteSelected(nullptr);
    editorUI->selectObject(secondObj, true);
    editorUI->onDeleteSelected(nullptr);

    editor->m_undoObjects->addObject(UndoObject::createWithArray(pastedObjs, UndoCommand::Paste));
    editor->m_redoObjects->addObject(UndoObject::createWithArray(pastedObjs, UndoCommand::Select));

    editorUI->selectObjects(pastedObjs, true);
    editorUI->updateButtons();
    this->onClose(nullptr);
}

void SettingsPopup::onInfo(CCObject* sender) {
    FLAlertLayer::create(
        "Info",
        "The <cy>toggle boxes</c> toggle the visiblity of the corresponding track.\n\n"
        "To use the <cy>plus buttons</c>, select two objects and then click a plus button. "
        "This will duplicate the left-most selected object along all the corresponding track guidelines that fall between your two selected objects.",
        "OK"
    )->show();
}

void SettingsPopup::onClose(cocos2d::CCObject*) {
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);

    mainPopup->mLevelSettingsLayer->setVisible(true);
    mainPopup->onClose(nullptr);
}

SettingsPopup* SettingsPopup::create() {
    auto ret = new SettingsPopup();
    if (ret->initAnchored(400.f, 250.f, "")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}