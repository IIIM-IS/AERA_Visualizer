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
  NewModelEvent(core::Timestamp time, r_code::Code* model, core::float32 evidenceCount, 
    core::float32 successRate, uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, model),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 1;

  core::float32 evidenceCount_;
  core::float32 successRate_;
  // TODO: Should the model's controller be recorded globally?
  uint64 controllerDegugOid_;
};

class SetModelEvidenceCountAndSuccessRateEvent : public AeraEvent {
public:
  SetModelEvidenceCountAndSuccessRateEvent
  (core::Timestamp time, r_code::Code* model, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time, model),
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
  NewCompositeStateEvent(core::Timestamp time, r_code::Code* compositeState, 
      uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, compositeState),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 3;

  // TODO: Should the composite state's controller be recorded globally?
  uint64 controllerDegugOid_;
};

class ProgramReductionEvent : public AeraEvent {
public:
  // TODO: Get the reduction time from the mk.rdx view's injection time?
  ProgramReductionEvent(core::Timestamp time, r_code::Code* programReduction,
      uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, programReduction),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 4;

  // TODO: Should the program's controller be recorded globally?
  uint64 controllerDegugOid_;
};

class ProgramReductionNewObjectEvent : public AeraEvent {
public:
  ProgramReductionNewObjectEvent(core::Timestamp time, r_code::Code* outputObject,
    r_code::Code* programReduction)
    : AeraEvent(EVENT_TYPE, time, outputObject),
    programReduction_(programReduction)
  {}

  static const int EVENT_TYPE = 5;

  r_code::Code* programReduction_;
  std::string syncMode_;
};

class AutoFocusNewObjectEvent : public AeraEvent {
public:
  AutoFocusNewObjectEvent(core::Timestamp time, r_code::Code* fromObject, 
    r_code::Code* toObject, const std::string& syncMode)
    : AeraEvent(EVENT_TYPE, time, toObject),
    fromObject_(fromObject), syncMode_(syncMode)
  {}

  static const int EVENT_TYPE = 6;

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

  static const int EVENT_TYPE = 7;

  r_code::Code* factImdl_;
  r_code::Code* cause_;
};

class NewInstantiatedCompositeStateEvent : public AeraEvent {
public:
  NewInstantiatedCompositeStateEvent(
      core::Timestamp time, r_code::Code* instantiatedCompositeState,
      const std::vector<r_code::Code*>& inputs)
    : AeraEvent(EVENT_TYPE, time, instantiatedCompositeState),
    inputs_(inputs)
  {}

  static const int EVENT_TYPE = 8;

  std::vector<r_code::Code*> inputs_;
};

}

#endif
