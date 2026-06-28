#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

inline bool writeScore(const std::string &filePath, const std::string &playerNames, int score) {
    int fileDescriptor = open(filePath.c_str(), O_RDWR | O_CREAT, 0644);
    if (fileDescriptor < 0) return false;

    flock(fileDescriptor, LOCK_EX);

    std::vector<std::pair<std::string, int>> scores;
    FILE *readHandle = fdopen(dup(fileDescriptor), "r");
    if (readHandle) {
        char   nameBuffer[256];
        int    entryScore;
        while (fscanf(readHandle, "%255[^:]:%d\n", nameBuffer, &entryScore) == 2)
            scores.push_back({nameBuffer, entryScore});
        fclose(readHandle);
    }

    scores.push_back({playerNames, score});

    std::sort(scores.begin(), scores.end(),
              [](const auto &leftEntry, const auto &rightEntry) {
                  return leftEntry.second < rightEntry.second;
              });
    if (scores.size() > 50) scores.resize(50);

    std::string tempFilePath = filePath + ".tmp";
    FILE *writeHandle = fopen(tempFilePath.c_str(), "w");
    if (writeHandle) {
        for (auto &[name, entryScore] : scores)
            fprintf(writeHandle, "%s:%d\n", name.c_str(), entryScore);
        fclose(writeHandle);
        rename(tempFilePath.c_str(), filePath.c_str());
    }

    flock(fileDescriptor, LOCK_UN);
    close(fileDescriptor);
    return true;
}

inline json readScoreboard(const std::string &filePath) {
    json scoreboardArray = json::array();
    int  fileDescriptor   = open(filePath.c_str(), O_RDONLY);
    if (fileDescriptor < 0) return scoreboardArray;

    flock(fileDescriptor, LOCK_SH);
    FILE *readHandle = fdopen(dup(fileDescriptor), "r");
    if (readHandle) {
        char nameBuffer[256];
        int  entryScore;
        while (fscanf(readHandle, "%255[^:]:%d\n", nameBuffer, &entryScore) == 2)
            scoreboardArray.push_back({{"names", nameBuffer}, {"score", entryScore}});
        fclose(readHandle);
    }
    flock(fileDescriptor, LOCK_UN);
    close(fileDescriptor);
    return scoreboardArray;
}

