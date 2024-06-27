#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/DrawGridLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "midifile/include/MidiFile.h"

#include "color_stuff/colors.h"

#include <vector>

using namespace geode::prelude;
using namespace smf;

float halfSpeedUps = 8.37188 * 30;
float oneSpeedUps = 10.3860833 * 30;
float twoSpeedUps = 12.914 * 30;
float threeSpeedUps = 15.6 * 30; 
float fourSpeedUps = 19.2 * 30; 

std::vector<std::vector<double>> getTrackAttacks(std::string filePath) {
    std::vector<std::vector<double>> tracks;
    MidiFile midifile;
    
    midifile.read(filePath);
    if (!midifile.status()) {
        log::debug("Error reading MIDI file");
    }

    int track = 0;
    for (int track=0; track  <midifile.getTrackCount(); track++) {
        std::vector<double> trackAttacksSeconds;
        for (int event=0; event<midifile[track].size(); event++) {
            if (!midifile[track][event].isNoteOn()) {
                continue;
            }
            if (trackAttacksSeconds.empty() || trackAttacksSeconds.back() != midifile[track][event].tick) {
                trackAttacksSeconds.push_back(midifile.getTimeInSeconds(midifile[track][event].tick));
            }
        }
        tracks.push_back(trackAttacksSeconds);
    }
    
    return tracks;
}

bool firstFrame = true;
std::vector<std::vector<double>> tracks;
std::string midiFilePath = "";

class $modify(DrawGridLayer) {	
    void draw() {
        DrawGridLayer::draw();

        std::vector<std::tuple<int, int, int, int>> colors = generateColors(tracks.size());

        for (int i = 0; i < tracks.size(); i++) {
            for (int j = 0; j < tracks[i].size(); j++) {
                float xPos = tracks[i][j] * fourSpeedUps;
                ccDrawColor4B(std::get<0>(colors[i]), std::get<1>(colors[i]), std::get<2>(colors[i]), std::get<3>(colors[i]));
                ccDrawLine(ccp(xPos, 0), ccp(xPos, 30000));
            }
        }

        if (firstFrame && midiFilePath != "") {
            tracks = getTrackAttacks(midiFilePath);
            firstFrame = false;
        }
	}
};

class $modify(mLevelEditorLayer, LevelEditorLayer) {
    void onClick(CCObject* sender) {
        utils::file::FilePickOptions::Filter filter = {
            "MIDI Files",
            { "*.mid", "*.midi" }
        };
        utils::file::FilePickOptions options = {
            .filters = { filter }
        };

        utils::file::pick(utils::file::PickMode::OpenFile, options
        ).listen(
            [](Result<std::filesystem::path>* result) {
                if(!result->isOk()) {
                    return;
                }

                std::filesystem::path path = result->unwrap();
                midiFilePath = path.string();
            }
        );
    }

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
};

