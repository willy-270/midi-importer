#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

#include "MainPopup.hpp"
#include "SettingsPopup/SettingsPopup.hpp"
#include "../../midifile/include/MidiFile.h"
#include "../../main.hpp"

using namespace geode::prelude;
using namespace smf;

std::filesystem::path midiFilePath;

std::vector<std::pair<bool, std::vector<double>>>  MainPopup::getTrackAttacks(std::string filePath) {
    std::vector<std::pair<bool, std::vector<double>>>tracks;
    MidiFile midifile;
    
    midifile.read(filePath);
    if (!midifile.status()) {
        FLAlertLayer::create(
            "Unable to read file",    
            "what the frick",  
            "OK"        
        )->show();
        return tracks;
    }

    int track = 0;
    for (int track = 0; track < midifile.getTrackCount(); track++) {
        std::vector<double> trackAttacksSeconds;
        for (int event=0; event<midifile[track].size(); event++) {
            if (!midifile[track][event].isNoteOn()) {
                continue;
            }
            if (trackAttacksSeconds.empty() || trackAttacksSeconds.back() != midifile[track][event].tick) {
                trackAttacksSeconds.push_back(midifile.getTimeInSeconds(midifile[track][event].tick));
            }
        }
        std::pair<bool, std::vector<double>> trackPair = {true, trackAttacksSeconds};
        tracks.push_back(trackPair);
    }
    
    return tracks;
}

bool MainPopup::setup(std::string const& value) {
    this->setTitle("Import MIDI File");

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    std::string fileString;
    if (midiFilePath.empty()) {
        fileString = "None";
    } else {
        fileString = midiFilePath.filename().string();
    }

    auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsSpr,
        this,
        menu_selector(MainPopup::onSettings)
    );
    settingsBtn->setPosition(145, 95);
    settingsBtn->setScale(.8f);

    m_currentFileLabel = CCLabelBMFont::create(("Current file: " + fileString).c_str(), "bigFont.fnt");
    m_currentFileLabel->setPosition(0, 40);
    m_currentFileLabel->setScale(.25f);
    
    auto openFileSpr = ButtonSprite::create("Open File");
    auto openFileBtn = CCMenuItemSpriteExtra::create(
        openFileSpr,
        this,
        menu_selector(MainPopup::onSelectFile)
    );
    openFileBtn->setPosition(0, -10);

    m_offsetInput = InputNode::create(150, "Offset", "bigFont.fnt", "1234567890.", 6);
    std::string offsetStr = std::to_string(offset);
    offsetStr.erase(offsetStr.find_last_not_of('0') + 1, std::string::npos);
    offsetStr.erase(offsetStr.find_last_not_of('.') + 1, std::string::npos);
    m_offsetInput->setString(offsetStr);
    m_offsetInput->setPosition(0, -50);
    m_offsetInput->setEnabled(true);

    auto menu = CCMenu::create();
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
    utils::file::pick(utils::file::PickMode::OpenFile, utils::file::FilePickOptions({.filters = {{"MIDI Files", { "*.mid", "*.midi" }}}})
    ).listen(
        [this](Result<std::filesystem::path>* result) {
            if(!result->isOk()) {
                return;
            }
    
            midiFilePath = result->unwrap();
            
            tracks = getTrackAttacks(midiFilePath.string());
            offset = std::stof(m_offsetInput->getString());

            std::string fileString;
            if (midiFilePath.empty()) {
                fileString = "None";
            } else {
                fileString = midiFilePath.filename().string();
            }

            m_currentFileLabel->setString(("Current file: " + fileString).c_str());
        }
    );
}

void MainPopup::onClose(cocos2d::CCObject*) {
    std::string offsetStr = m_offsetInput->getString();
    if (offsetStr.empty()) {
        offset = 0.f;
    } else {
        offset = std::stof(offsetStr);
    }

    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

MainPopup* MainPopup::create() {
    auto ret = new MainPopup();
    if (ret->init(300.f, 200.f, "")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}