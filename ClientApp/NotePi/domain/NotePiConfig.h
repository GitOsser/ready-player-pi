#pragma once

namespace notepi::config {

inline constexpr int BLOCK_SIZE = 256;
inline constexpr int HOP_SIZE = 1024;
inline constexpr int FRAME_SIZE = 2048;
inline constexpr double SAMPLE_RATE_HZ = 48000.0;

inline constexpr const char* MIC_DEVICE = "default";

inline constexpr int SEQUENCE_LENGTH = 5;

inline constexpr float MIN_ENERGY = 1000.0f;

inline constexpr float DOMINANCE_THRESHOLD = 0.30f;

inline constexpr float FUNDAMENTAL_WEIGHT = 1.0f;
inline constexpr float OVERTONE1_WEIGHT = 0.3f;
inline constexpr float OVERTONE2_WEIGHT = 0.1f;

inline constexpr std::size_t RECENT_DETECTIONS_SIZE = 18;

}

