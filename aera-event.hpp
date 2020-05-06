#ifndef AERA_EVENT_HPP
#define AERA_EVENT_HPP

#include <QPointF>
#include "submodules/replicode/r_code/object.h"

namespace aera_visualizer {

class AeraEvent {
public:
  AeraEvent(int eventType, core::Timestamp time, r_code::Code* object)
  : eventType_(eventType),
    time_(time),
    object_(object),
    itemPosition_(qQNaN(), qQNaN())
  {}

  int eventType_;
  core::Timestamp time_;
  r_code::Code* object_;
  // itemPosition_ is used by NewModelEvent.
  QPointF itemPosition_;
};

class NewModelEvent : public AeraEvent {
public:
  NewModelEvent(core::Timestamp time, r_code::Code* object, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time, object),
    evidenceCount_(evidenceCount),
    successRate_(successRate)
  {}

  static const int EVENT_TYPE = 1;

  core::float32 evidenceCount_;
  core::float32 successRate_;
};

class SetModelEvidenceCountAndSuccessRateEvent : public AeraEvent {
public:
  SetModelEvidenceCountAndSuccessRateEvent
  (core::Timestamp time, r_code::Code* object, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time, object),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    oldEvidenceCount_(qQNaN()),
    oldSuccessRate_(qQNaN())
  {}

  static const int EVENT_TYPE = 2;

  core::float32 evidenceCount_;
  core::float32 successRate_;
  core::float32 oldEvidenceCount_;
  core::float32 oldSuccessRate_;
};

class NewCompositeStateEvent : public AeraEvent {
public:
  NewCompositeStateEvent(core::Timestamp time, r_code::Code* object)
    : AeraEvent(EVENT_TYPE, time, object)
  {}

  static const int EVENT_TYPE = 3;
};

}

#endif
