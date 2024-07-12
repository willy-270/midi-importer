#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "main.hpp"
#include "external/json.hpp"

using json = nlohmann::json;
using namespace geode::prelude;

std::filesystem::path getLevelJsonPath(GJGameLevel* level) {
    int levelId = EditorIDs::getID(level);
    std::filesystem::path saveDir = Mod::get()->getSaveDir();
    std::filesystem::path levelJsonPath = saveDir / (std::to_string(levelId) + ".json");

    return levelJsonPath;
}

class $modify(EditorPauseLayer) {
    $override
    void saveLevel() {
        EditorPauseLayer::saveLevel();

        if (currentMidiData.tracks.empty()) {
            return;
        }
        
        json levelJson = makeLevelJson();

        std::filesystem::path levelJsonPath = getLevelJsonPath(this->m_editorLayer->m_level);

        if (!std::filesystem::exists(levelJsonPath)) {
            std::ofstream file(levelJsonPath);
            file << levelJson.dump(4);
        } else {
            if (!compareJsonFiles(levelJsonPath, levelJson)) {
                std::ofstream file(levelJsonPath);
                file << levelJson.dump(4);
            }
        }
    }

    bool compareJsonFiles(const std::filesystem::path& filePath, const json& newJson) {
        std::ifstream file(filePath);
        nlohmann::json existingJson;
        file >> existingJson;

        return existingJson == newJson;
    }

    json makeLevelJson() {
        json levelJson;

        levelJson["offset"] = currentMidiData.offset;

        if (currentMidiData.fileName.empty()) {
            levelJson["file-name"] = "None";
        } else {
            levelJson["file-name"] = currentMidiData.fileName;
        }

        json tracksJson;
        for (size_t i = 0; i < currentMidiData.tracks.size(); ++i) {
            const auto& track = currentMidiData.tracks[i];
            json trackJson;
            trackJson["visable"] = track.visible;
            trackJson["track-num"] = i + 1;
            trackJson["instrument-name"] = track.instrumentName;
            trackJson["note-attacks"] = track.noteAttacks;
            tracksJson.push_back(trackJson);
        }
        levelJson["tracks"] = tracksJson;

        return levelJson;
    }
};

class $modify(EditLevelLayer) {
    $override 
    void confirmDelete(CCObject* p0) {
        EditLevelLayer::confirmDelete(p0);

        std::filesystem::path levelJsonPath = getLevelJsonPath(this->m_level);

        if (std::filesystem::exists(levelJsonPath)) {
            std::filesystem::remove(levelJsonPath);
        }
    }
};

