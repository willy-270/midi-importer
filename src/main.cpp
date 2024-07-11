#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/DrawGridLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "midifile/include/MidiFile.h"
#include "color_generator/colors.hpp"
#include "main.hpp"

#include <vector>
#include <map>

using namespace geode::prelude;
using namespace smf;

const float speedMultipliers[] = {
    8.37188 * 30,
    10.3860833 * 30,
    12.914 * 30,
    15.6 * 30,
    19.2 * 30
};

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

// vector of tracks, where each track is a std::pair<bool, std::vector<double>>
// bool represents if track should be visable, the vector<double> contains the times of the notes
std::vector<std::pair<bool, std::vector<double>>> tracks;
float offset; 

class $modify(mDrawGridLayer, DrawGridLayer) {    
    struct Fields {
        std::vector<Ref<GameObject>> speedPortals;
    };

    $override
    void draw() {
        DrawGridLayer::draw();

        std::vector<std::tuple<int, int, int, int>> colors = generateColors(tracks.size());

        for (int i = 0; i < tracks.size(); i++) {
            for (int j = 0; j < tracks[i].second.size(); j++) {
                if (tracks[i].first == false) continue;
                
                float time = tracks[i].second[j] + offset;
                float xPos = getXPosition(time);
                ccDrawColor4B(std::get<0>(colors[i]), std::get<1>(colors[i]), std::get<2>(colors[i]), std::get<3>(colors[i]));
                ccDrawLine(ccp(xPos, 0), ccp(xPos, 30000));
            }
        }
    }

    //shoutout to chatgpt
    float getXPosition(float time) {
        float xPos = 0.0f;
        float lastTime = 0.0f;
        float lastXPos = 0.0f;
        int currentSpeedType = getStartSpeedInt();

        for (const auto& portal : m_fields->speedPortals) {
            float changePos = portal->getPositionX();
            int speedType = getSpeedInt(portal);

            float changeTime = (changePos - lastXPos) / speedMultipliers[currentSpeedType];

            if (time < lastTime + changeTime) {
                xPos = lastXPos + (time - lastTime) * speedMultipliers[currentSpeedType];
                return xPos;
            }

            lastXPos = changePos;
            lastTime += changeTime;
            currentSpeedType = speedType;
        }

        xPos = lastXPos + (time - lastTime) * speedMultipliers[currentSpeedType];
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
};

class $modify(LevelEditorLayer) {
    $override
    void addSpecial(GameObject* obj) {
        LevelEditorLayer::addSpecial(obj);

        if (obj->m_objectID == 200 || obj->m_objectID == 201 || obj->m_objectID == 202 || obj->m_objectID == 203 || obj->m_objectID == 1334) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(m_drawGridLayer)->m_fields->speedPortals;
            speedPortals.push_back(obj);
            
            std::sort(speedPortals.begin(), speedPortals.end(), [](const Ref<GameObject>& obj1, const Ref<GameObject>& obj2) {
                return obj1->getPositionX() < obj2->getPositionX();
            });
        }
    }

    $override
    void removeSpecial(GameObject* obj) {
        LevelEditorLayer::removeSpecial(obj);

        if (obj->m_objectID == 200 || obj->m_objectID == 201 || obj->m_objectID == 202 || obj->m_objectID == 203 || obj->m_objectID == 1334) {
            auto& speedPortals = static_cast<mDrawGridLayer*>(m_drawGridLayer)->m_fields->speedPortals;
            speedPortals.erase(std::remove(speedPortals.begin(), speedPortals.end(), obj), speedPortals.end());

            std::sort(speedPortals.begin(), speedPortals.end(), [](const Ref<GameObject>& obj1, const Ref<GameObject>& obj2) {
                return obj1->getPositionX() < obj2->getPositionX();
            });
        }
    }
};

