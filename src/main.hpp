#ifndef MAIN_HPP
#define MAIN_HPP

struct LevelMidiData {
    struct Track {
        bool visible;
        int trackNum;
        std::string instrumentName;
        std::vector<double> noteAttacks;
    };
    float offset;
    std::string fileName;
    std::vector<Track> tracks;
};

extern LevelMidiData currentMidiData;

#endif

