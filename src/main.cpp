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
#include <array>
#include <map>
#include <unordered_map>

using namespace geode::prelude;
using namespace smf;
using json = nlohmann::json;

LevelMidiData currentMidiData; //extern, used in lots of files
std::vector<std::vector<float>> xPositions; //extern, for obj placement in SettignsPopup

std::unordered_map<double, float> xPosCache;


class $modify(mDrawGridLayer, DrawGridLayer) {    
    struct Fields {
        std::vector<Ref<GameObject>> m_speedPortals;
        bool m_firstFrame = true;
        int cachedStartSpeed;
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

            for (const auto& track : currentMidiData.tracks) {
                xPositions.push_back(std::vector<float>());
                for (double noteAttack : track.noteAttacks) {
                    xPositions.back().push_back(getXPosition(noteAttack + currentMidiData.offset));
                }
            }

            m_fields->m_firstFrame = false;
        }

        drawLines();
    }

    void loadDataFromJson() {
        currentMidiData = LevelMidiData();
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
            for (double noteAttack : currentMidiData.tracks[i].noteAttacks) {
                float time = noteAttack + currentMidiData.offset;
                float xPos = getXPosition(time);

                if (!currentMidiData.tracks[i].visible) continue;

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

        auto cacheIt = xPosCache.find(time);
        if (cacheIt != xPosCache.end() && currentSpeedType == m_fields->cachedStartSpeed) {
            return cacheIt->second;
        }

        m_fields->cachedStartSpeed = currentSpeedType;
        float lastTime = 0.0f;
        float lastXPos = 0.0f;

        for (const auto& portal : m_fields->m_speedPortals) {
            int speedType = getSpeedInt(portal);
            float changePos = getNearestXPosition(portal);

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

    float getNearestXPosition(GameObject* portal) {
        cocos2d::CCPoint center = portal->getPosition();

        float height = portal->getScaledContentHeight();
        float width = portal->getScaledContentWidth();

        std::array<std::pair<float, float>, 4> corners = {
            std::make_pair(center.x - width / 2, center.y - height / 2), // bottom left
            std::make_pair(center.x + width / 2, center.y - height / 2), // bottom right
            std::make_pair(center.x - width / 2, center.y + height / 2), // top left
            std::make_pair(center.x + width / 2, center.y + height / 2) // top right
        };

        if (portal->getRotation() != 0.0f) {
            corners = rotateCorners(corners, center, portal->getRotation());
        }

        float minX = corners[0].first;
        for (const auto& corner : corners) {
            if (corner.first < minX) minX = corner.first;
        }

        return minX;
    }

    std::array<std::pair<float, float>, 4> rotateCorners(std::array<std::pair<float, float>, 4> corners, cocos2d::CCPoint center, float rotation) {
        float angle = CC_DEGREES_TO_RADIANS(rotation);
        float s = sin(angle);
        float c = cos(angle);

        for (auto& corner : corners) {
            float x = corner.first - center.x;
            float y = corner.second - center.y;

            corner.first = x * c - y * s + center.x;
            corner.second = x * s + y * c + center.y;
        }

        return corners;
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

void sortSpeedPortalsByXPos(std::vector<Ref<GameObject>>& portals) {
    std::sort(portals.begin(), portals.end(), [](const Ref<GameObject>& obj1, const Ref<GameObject>& obj2) {
        return obj1->getPositionX() < obj2->getPositionX();
    });
}

class $modify(LevelEditorLayer) {
    $override
    void addSpecial(GameObject* obj) {
        LevelEditorLayer::addSpecial(obj);

        if (isSpeedPortal(obj)) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(m_drawGridLayer)->m_fields->m_speedPortals;
            speedPortals.push_back(obj);
            sortSpeedPortalsByXPos(speedPortals);

            xPosCache.clear();
        }
    }

    $override
    void removeSpecial(GameObject* obj) {
        LevelEditorLayer::removeSpecial(obj);

        if (isSpeedPortal(obj)) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(m_drawGridLayer)->m_fields->m_speedPortals;
            speedPortals.erase(std::remove(speedPortals.begin(), speedPortals.end(), obj), speedPortals.end());
            sortSpeedPortalsByXPos(speedPortals);

            xPosCache.clear();
        }
    }
};

class $modify(EditorUI) {
    $override
    void moveObject(GameObject* p0, CCPoint p1) {
        EditorUI::moveObject(p0, p1);

        if (isSpeedPortal(p0)) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(this->m_editorLayer->m_drawGridLayer)->m_fields->m_speedPortals;
            sortSpeedPortalsByXPos(speedPortals);

            xPosCache.clear();    
        }
    }

    $override
    void deselectAll() {
        EditorUI::deselectAll();

        auto& speedPortals = static_cast<mDrawGridLayer*>(this->m_editorLayer->m_drawGridLayer)->m_fields->m_speedPortals;
        sortSpeedPortalsByXPos(speedPortals);

        xPosCache.clear();
    }
};