#include "ChordVelocityMapping.h"

namespace ChordVelocityMapping {

// EXTENDED BASE STRATEGY CLASS ////////////////////////////////////////////////

class ExtendedStrategy : public Strategy {
public:
  virtual ~ExtendedStrategy() {}

  virtual bool hasStart(std::vector<noteData> const& notes) {
    return Events::hasStart<noteData>(notes);
  }

  virtual float computeMeanVelocity(std::vector<noteData> const& notes) {
    if (notes.empty()) return 0;
    uint8_t sum = 0;
    for (auto& note : notes) {
        sum += note.velocity;
    }
    return sum / notes.size();
  }

  virtual std::tuple<float, uint8_t, uint8_t>
  computeVelocityStats(std::vector<noteData> const& notes) {
    uint8_t cnt = 0, sum = 0, max = 0, min = 127;
    for (auto& note : notes) {
        if (note.on && note.velocity != 0) {
            cnt++;
            sum += note.velocity;
            if (note.velocity > max) max = note.velocity;
            if (note.velocity < min) min = note.velocity;
        }
    }
    float mean = (cnt == 0) ? 0 : static_cast<float>(sum) / cnt;
    return std::make_tuple(mean, min, max);
  }

  virtual void
  adjustToCommandVelocity(std::vector<noteData>& notes,
                          uint8_t cmd_velocity) = 0;
};

// STRATEGY IMPLEMENTATIONS ////////////////////////////////////////////////////

class SameForAll :
public ExtendedStrategy {
  virtual void
  adjustToCommandVelocity(std::vector<noteData>& notes,
                          uint8_t cmd_velocity) {
    for (auto& note : notes) {
      if (note.on && note.velocity != 0) {
        note.velocity = cmd_velocity;
      }
    }
  }
};

class ClippedScaledFromMean :
public ExtendedStrategy {
  virtual void
  adjustToCommandVelocity(std::vector<noteData>& notes,
                          uint8_t cmd_velocity) {
    float mean = computeMeanVelocity(notes);
    if (mean == 0.f) return;

    const int maxVelocity = 127, minVelocity = 1;

    for (auto& note : notes) {
      if (note.on && note.velocity != 0) {
        float proportion = note.velocity / mean;
        int adjustedVelocity = proportion * cmd_velocity;
        note.velocity = uint8_t(
          std::min(maxVelocity, std::max(minVelocity, adjustedVelocity))
        );
      }
    }

  }
};

class AdjustedScaledFromMean :
public ExtendedStrategy {
  virtual void
  adjustToCommandVelocity(std::vector<noteData>& notes,
                          uint8_t cmd_velocity) {
    float mean;
    uint8_t min, max;
    std::tie(mean, min, max) = computeVelocityStats(notes);
    if (mean == 0.f) return;

    const uint8_t maxVelocity = 127, minVelocity = 1;
    float ratio = static_cast<float>(cmd_velocity) / mean;

    float maxDeltaVelocity = (maxVelocity - cmd_velocity);
    float minDeltaVelocity = (maxVelocity - cmd_velocity);
    // if maxRatio is +inf, scale will be 0 => OK
    float maxRatio = (max * ratio - cmd_velocity) / (maxVelocity - cmd_velocity);
    // if minRatio is std::abs(-inf), scale will still be 0 => OK
    float minRatio = std::abs(min * ratio - cmd_velocity) / (minVelocity - cmd_velocity);
    float scale = 1.f / std::max(1.f, std::max(maxRatio, minRatio));
    
    for (auto& note : notes) {
      if (note.on && note.velocity != 0) {
        float velocity = cmd_velocity + (note.velocity * ratio - cmd_velocity) * scale;
        note.velocity = uint8_t(velocity);
      }
    }
  }
};

class ClippedScaledFromMax :
public ExtendedStrategy {
  virtual void
  adjustToCommandVelocity(std::vector<noteData>& notes,
                          uint8_t cmd_velocity) {
    uint8_t max = 0;
    for (auto& note: notes) {
      if (note.velocity > max) max = note.velocity;
    }
    if (max == 0) return;

    const int maxVelocity = 127, minVelocity = 1;
    float maxRatio = static_cast<float>(cmd_velocity) / max;

    for (auto& note: notes) {
      float ratio = static_cast<float>(note.velocity) / max;
      int adjustedVelocity = note.velocity * maxRatio;
      note.velocity = uint8_t(
        std::min(maxVelocity, std::max(minVelocity, adjustedVelocity))
      );
    }
  }
};

// STRATEGY FACTORY FUNCTION ///////////////////////////////////////////////////

std::shared_ptr<Strategy> createStrategy(StrategyType s) {
  Strategy* strategy;
  switch (s) {
    case StrategyType::SameForAll:
      strategy = new SameForAll();
      break;
    case StrategyType::ClippedScaledFromMean:
      strategy = new ClippedScaledFromMean();
      break;
    case StrategyType::AdjustedScaledFromMean:
      strategy = new AdjustedScaledFromMean();
      break;
    case StrategyType::ClippedScaledFromMax:
      strategy = new ClippedScaledFromMax();
      break;
    default:
      strategy = nullptr;
      break;
  }
  return std::shared_ptr<Strategy>(strategy);
}

} /* END NAMESPACE ChordVelocityMapping */
