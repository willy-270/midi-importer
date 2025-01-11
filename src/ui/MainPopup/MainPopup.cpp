#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

#include "MainPopup.hpp"
#include "SettingsPopup/SettingsPopup.hpp"
#include "../../external/midifile/include/MidiFile.h"
#include "../../main.hpp"

using namespace geode::prelude;
using namespace smf;

std::filesystem::path midiFilePath;

LevelMidiData MainPopup::getMidiData(std::string filePath) {
    LevelMidiData levelMidiData;
    MidiFile midiFile;
    
    midiFile.read(filePath);
    if (!midiFile.status()) {
        FLAlertLayer::create(
            "Unable to read file",    
            "what the frick",  
            "OK"        
        )->show();
        return levelMidiData;
    }

    int track = 0;
    for (int track = 0; track < midiFile.getTrackCount(); track++) {
        std::vector<double> trackAttacksSeconds;
        std::string instrumentName;
        bool instrumentNameStored = false;
        for (int event = 0; event < midiFile[track].size(); event++) {
            if (!instrumentNameStored && midiFile[track][event].isMeta() && midiFile[track][event].getMetaType() == 0x03) {
                instrumentName = midiFile[track][event].getMetaContent();
                instrumentNameStored = true;
            }
            if (!midiFile[track][event].isNoteOn()) {
                continue;
            }
            if (trackAttacksSeconds.empty() || trackAttacksSeconds.back() != midiFile[track][event].tick) {
                trackAttacksSeconds.push_back(midiFile.getTimeInSeconds(midiFile[track][event].tick));
            }
        }
        if (trackAttacksSeconds.empty()) {
            continue;
        }
        LevelMidiData::Track trackObj;
        trackObj.noteAttacks = trackAttacksSeconds;
        trackObj.visible = true;
        if (instrumentName.empty()) trackObj.instrumentName = "Unknown";
        else trackObj.instrumentName = instrumentName;

        levelMidiData.tracks.push_back(trackObj);
    }
    
    return levelMidiData;
}

bool MainPopup::setup(std::string const& value) {
    this->setTitle("Import MIDI File");

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsSpr->setScale(.8f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsSpr,
        this,
        menu_selector(MainPopup::onSettings)
    );
    settingsBtn->setPosition(145, 95);

    if (currentMidiData.fileName.empty()) {
        currentMidiData.fileName = "None";
    }
    m_currentFileLabel = CCLabelBMFont::create(("Current file: " + currentMidiData.fileName).c_str(), "bigFont.fnt");
    m_currentFileLabel->setPosition(0, 40);
    m_currentFileLabel->setScale(.25f);
    
    auto openFileSpr = ButtonSprite::create("Open File");
    auto openFileBtn = CCMenuItemSpriteExtra::create(
        openFileSpr,
        this,
        menu_selector(MainPopup::onSelectFile)
    );
    openFileBtn->setPosition(0, -10);

    m_offsetInput = TextInput::create(150, "Offset", "bigFont.fnt");
    m_offsetInput->setPosition(0, -50);
    m_offsetInput->setEnabled(true);
    m_offsetInput->setCommonFilter(CommonFilter::Float);
    m_offsetInput->setMaxCharCount(10);
    std::string offsetStr = std::to_string(currentMidiData.offset);
    offsetStr.erase(offsetStr.find_last_not_of('0') + 1, std::string::npos);
    offsetStr.erase(offsetStr.find_last_not_of('.') + 1, std::string::npos);
    if (offsetStr != "0") {
        m_offsetInput->setString(offsetStr);
    }

    auto menu = CCMenu::create();
    menu->setPosition(150.f, 100.f); //whole menu offset after update
    menu->addChild(m_currentFileLabel);
    menu->addChild(settingsBtn);
    menu->addChild(openFileBtn);
    menu->addChild(m_offsetInput);
    m_mainLayer->addChild(menu);

    return true;
}

void MainPopup::onSettings(CCObject* sender) {
    SettingsPopup::create()->show();
}

void MainPopup::onSelectFile(CCObject* sender) {
    utils::file::pick(
        utils::file::PickMode::OpenFile, 
        utils::file::FilePickOptions({.filters = {{"MIDI Files", { "*.mid", "*.midi" }}}})
    ).listen(
        [this](Result<std::filesystem::path>* result) {
            if(!result->isOk()) {
                return;
            }
    
            midiFilePath = result->unwrap();
            
            currentMidiData = getMidiData(midiFilePath.string());
            currentMidiData.offset = std::stof(m_offsetInput->getString());

            currentMidiData.fileName = midiFilePath.filename().string();
            m_currentFileLabel->setString(("Current file: " + currentMidiData.fileName).c_str());
        }
    );
}

void MainPopup::onClose(cocos2d::CCObject*) {
    std::string offsetStr = m_offsetInput->getString();
    if (offsetStr.empty()) {
        currentMidiData.offset = 0.f;
    } else {
        currentMidiData.offset = std::stof(offsetStr);
    }

    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

MainPopup* MainPopup::create() {
    auto ret = new MainPopup();
    if (ret->initAnchored(300.f, 200.f, "")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}