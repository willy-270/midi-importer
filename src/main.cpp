#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/DrawGridLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "external/midifile/include/MidiFile.h"
#include "external/json.hpp"
#include "color_generator/colors.hpp"
#include "main.hpp"

#include <vector>
#include <map>

using namespace geode::prelude;
using namespace smf;
using json = nlohmann::json;

LevelMidiData currentMidiData; //extern

std::unordered_map<double, float> xPosCache;
int cachedStartSpeed;

class $modify(mDrawGridLayer, DrawGridLayer) {    
    struct Fields {
        std::vector<Ref<GameObject>> m_speedPortals;
        bool m_firstFrame = true;
        const float m_speedMultipliers[5] = {
            8.37188 * 30,
            10.3860833 * 30,
            12.914 * 30,
            15.6 * 30,
            19.2 * 30
        };
    };

    $override
    void draw() {
        DrawGridLayer::draw();

        if (m_fields->m_firstFrame) {

            loadDataFromJson();
            m_fields->m_firstFrame = false;
        }

        drawLines();
    }

    void loadDataFromJson() {
        currentMidiData = LevelMidiData();
        currentMidiData.offset = 0.0f;
        currentMidiData.fileName = "";
        xPosCache.clear();

        std::filesystem::path levelJsonPath = Mod::get()->getSaveDir() / (std::to_string(EditorIDs::getID(this->m_editorLayer->m_level)) + ".json");

        if (std::filesystem::exists(levelJsonPath)) {
            std::ifstream file(levelJsonPath);
            json levelJson;
            file >> levelJson;

            currentMidiData.offset = levelJson["offset"];
            currentMidiData.fileName = levelJson["file-name"];
            for (const auto& track : levelJson["tracks"]) {
                LevelMidiData::Track newTrack;
                newTrack.visible = track["visable"];
                newTrack.trackNum = track["track-num"];
                newTrack.instrumentName = track["instrument-name"];
                newTrack.noteAttacks = std::vector<double>(track["note-attacks"].begin(), track["note-attacks"].end());
                currentMidiData.tracks.push_back(newTrack);
            }
        }
    }

    void drawLines() {
        std::vector<std::tuple<int, int, int, int>> colors = generateColors(currentMidiData.tracks.size());

        for (int i = 0; i < currentMidiData.tracks.size(); i++) {
            for (int j = 0; j < currentMidiData.tracks[i].noteAttacks.size(); j++) {
                if (currentMidiData.tracks[i].visible == false) continue;
                
                float time = currentMidiData.tracks[i].noteAttacks[j] + currentMidiData.offset;
                float xPos = getXPosition(time);
                ccDrawColor4B(
                    std::get<0>(colors[i]), 
                    std::get<1>(colors[i]), 
                    std::get<2>(colors[i]),
                    std::get<3>(colors[i])
                );
                ccDrawLine(ccp(xPos, 0), ccp(xPos, 30000));
            }
        }
    }

    float getXPosition(float time) {
        int currentSpeedType = getStartSpeedInt();

        if (xPosCache.find(time) != xPosCache.end() && currentSpeedType == cachedStartSpeed) {
            return xPosCache[time];
        }

        xPosCache.clear();

        cachedStartSpeed = currentSpeedType;
        float lastTime = 0.0f;
        float lastXPos = 0.0f;

        for (const auto& portal : m_fields->m_speedPortals) {
            float changePos = portal->getPositionX();
            int speedType = getSpeedInt(portal);

            float changeTime = (changePos - lastXPos) / m_fields->m_speedMultipliers[currentSpeedType];
            if (time < lastTime + changeTime) {
                float xPos = lastXPos + (time - lastTime) * m_fields->m_speedMultipliers[currentSpeedType];
                xPosCache[time] = xPos;
                return xPos;
            }

            lastXPos = changePos;
            lastTime += changeTime;
            currentSpeedType = speedType;
        }

        float xPos = lastXPos + (time - lastTime) * m_fields->m_speedMultipliers[currentSpeedType];
        xPosCache[time] = xPos;
        return xPos;
    }

    int getStartSpeedInt() {
        Speed speed = this->m_editorLayer->m_levelSettings->m_startSpeed;
        switch (speed) {
            case Speed::Slow: return 0;
            case Speed::Normal: return 1;
            case Speed::Fast: return 2;
            case Speed::Faster: return 3;
            case Speed::Fastest: return 4;
        }
        return -1;
    }

    int getSpeedInt(GameObject* obj) {
        switch (obj->m_objectID) {
            case 200: return 0;
            case 201: return 1;
            case 202: return 2;
            case 203: return 3;
            case 1334: return 4;
        }
        return -1;
    }
};

bool isSpeedPortal(GameObject* obj) {
    return obj->m_objectID == 200 || obj->m_objectID == 201 || obj->m_objectID == 202 || obj->m_objectID == 203 || obj->m_objectID == 1334;
}

class $modify(LevelEditorLayer) {
    $override
    void addSpecial(GameObject* obj) {
        LevelEditorLayer::addSpecial(obj);

        if (isSpeedPortal(obj)) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(m_drawGridLayer)->m_fields->m_speedPortals;
            speedPortals.push_back(obj);
            sortSpeedPortals(speedPortals);
            xPosCache.clear();
        }
    }

    $override
    void removeSpecial(GameObject* obj) {
        LevelEditorLayer::removeSpecial(obj);

        if (isSpeedPortal(obj)) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(m_drawGridLayer)->m_fields->m_speedPortals;
            speedPortals.erase(std::remove(speedPortals.begin(), speedPortals.end(), obj), speedPortals.end());
            sortSpeedPortals(speedPortals);
            xPosCache.clear();
        }
    }

    void sortSpeedPortals(std::vector<Ref<GameObject>>& portals) {
        std::sort(portals.begin(), portals.end(), [](const Ref<GameObject>& obj1, const Ref<GameObject>& obj2) {
            return obj1->getPositionX() < obj2->getPositionX();
        });
    }
};

class $modify(EditorUI) {
    $override
    void moveObject(GameObject* p0, CCPoint p1) {
        EditorUI::moveObject(p0, p1);

        if (isSpeedPortal(p0)) {
            xPosCache.clear();    
        }
    }
};
