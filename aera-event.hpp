#ifndef AERA_EVENT_HPP
#define AERA_EVENT_HPP

#include <QPointF>
#include "submodules/replicode/r_code/object.h"

namespace aera_visualizer {

class AeraEvent {
public:
  AeraEvent(int eventType, core::Timestamp time)
    : eventType_(eventType),
    time_(time)
  {}

  int eventType_;
  core::Timestamp time_;
};

class NewModelEvent : public AeraEvent {
public:
  NewModelEvent(core::Timestamp time, r_code::Code* model, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time),
    model_(model),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    itemPosition_(qQNaN(), qQNaN())
  {}

  static const int EVENT_TYPE = 1;

  r_code::Code* model_;
  core::float32 evidenceCount_;
  core::float32 successRate_;
  QPointF itemPosition_;
};

class SetModelEvidenceCountAndSuccessRateEvent : public AeraEvent {
public:
  SetModelEvidenceCountAndSuccessRateEvent
  (core::Timestamp time, r_code::Code* model, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time),
    model_(model),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    oldEvidenceCount_(qQNaN()),
    oldSuccessRate_(qQNaN())
  {}

  static const int EVENT_TYPE = 2;

  r_code::Code* model_;
  core::float32 evidenceCount_;
  core::float32 successRate_;
  core::float32 oldEvidenceCount_;
  core::float32 oldSuccessRate_;
};

}

#endif
