#include "MFPRenderer.h"

const std::uint8_t defaultVelocity  = 127;
const std::uint8_t defaultChannel   = 1;

typedef std::pair<uint8_t, noteData> noteEvent;

inline noteData makeNote(
  bool on,
  uint8_t pitch,
  uint8_t velocity = defaultVelocity,
  uint8_t channel = defaultChannel
) {
  return { on, pitch, velocity, channel };
}

inline commandData makeCommand(
  bool pressed,
  uint8_t id,
  uint8_t velocity = defaultVelocity,
  uint8_t channel = defaultChannel
) {
  return { pressed, id, velocity, channel };
}

inline void feedRenderer(
  MFPRenderer& renderer,
  const std::vector<noteEvent>& events
) {
  renderer.clear();
  for (auto& event : events) {
    renderer.pushEvent(event.first, event.second);
  }
  renderer.finalize();
};

inline std::vector<std::vector<noteData>> getPerformanceResults(
  MFPRenderer& renderer,
  const std::vector<commandData>& commands
) {
  std::vector<std::vector<noteData>> res;
  for (auto& command : commands) {
    res.push_back(renderer.combine3(command));
  }
  return res;
};

//*
inline bool performanceResultsAreIdentical(
  std::vector<std::vector<noteData>>& res1,
  std::vector<std::vector<noteData>>& res2
) {
  if (res1.size() != res2.size()) return false;

  for (auto i = 0; i < res1.size(); ++i) {
    if (res1[i].size() != res2[i].size()) return false;

    for (auto j = 0; j < res1[i].size(); ++j) {
      if (res1[i][j] != res2[i][j]) return false;
    }
  }

  return true;
}
//*/
