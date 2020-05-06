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
  // itemPosition_ is used by "New" events to remember the screen position after undoing.
  QPointF itemPosition_;
};

class NewModelEvent : public AeraEvent {
public:
  NewModelEvent(core::Timestamp time, r_code::Code* object, core::float32 evidenceCount, 
    core::float32 successRate, uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, object),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 1;

  core::float32 evidenceCount_;
  core::float32 successRate_;
  uint64 controllerDegugOid_;
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
  NewCompositeStateEvent(core::Timestamp time, r_code::Code* object, uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, object),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 3;

  uint64 controllerDegugOid_;
};

class AutoFocusNewObjectEvent : public AeraEvent {
public:
  AutoFocusNewObjectEvent(core::Timestamp time, r_code::Code* fromObject, 
    r_code::Code* toObject, const std::string& syncMode)
    : AeraEvent(EVENT_TYPE, time, toObject),
    fromObject_(fromObject), syncMode_(syncMode)
  {}

  static const int EVENT_TYPE = 4;

  r_code::Code* fromObject_;
  std::string syncMode_;
};

class NewMkValPredictionEvent : public AeraEvent {
public:
  NewMkValPredictionEvent(core::Timestamp time, r_code::Code* factPrediction,
    r_code::Code* factImdl, r_code::Code* cause)
    : AeraEvent(EVENT_TYPE, time, factPrediction),
    factImdl_(factImdl), cause_(cause)
  {}

  static const int EVENT_TYPE = 5;

  r_code::Code* factImdl_;
  r_code::Code* cause_;
};

}

#endif
