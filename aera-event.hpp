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
    itemTopLeftPosition_(qQNaN(), qQNaN())
  {}

  int eventType_;
  core::Timestamp time_;
  r_code::Code* object_;
  // itemTopLeftPosition_ is used by "New" events to remember the screen position after undoing.
  QPointF itemTopLeftPosition_;
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

class ModelPredictionReduction : public AeraEvent {
public:
  /**
   * Create a ModelPredictionReduction, but set object_ to the (fact (pred ...)).
   * (mk.rdx fact_imdl [fact_cause] [fact_pred]) .
   * \param time The reduction time.
   * \param reduction The model reduction which points to the (fact (pred ...)) and cause
   */
  ModelPredictionReduction(core::Timestamp time, r_code::Code* reduction)
    : AeraEvent(EVENT_TYPE, time, reduction->get_reference(2)),
    reduction_(reduction)
  {}

  static const int EVENT_TYPE = 7;

  r_code::Code* getFactImdl() { return reduction_->get_reference(0); }

  r_code::Code* getCause() { return reduction_->get_reference(1); }

  r_code::Code* getFactPred() { return object_; }

  r_code::Code* reduction_;
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

class NewPredictionSuccessEvent : public AeraEvent {
public:
  NewPredictionSuccessEvent(core::Timestamp time, r_code::Code* factSuccessFactPred)
    : AeraEvent(EVENT_TYPE, time, factSuccessFactPred)
  {}

  static const int EVENT_TYPE = 9;
};

class EnvironmentInjectEvent : public AeraEvent {
public:
  EnvironmentInjectEvent(
    core::Timestamp time, r_code::Code* object, core::Timestamp injectionTime)
    : AeraEvent(EVENT_TYPE, time, object),
    injectionTime_(injectionTime)
  {}

  static const int EVENT_TYPE = 10;

  core::Timestamp injectionTime_;
};

class EnvironmentEjectEvent : public AeraEvent {
public:
  EnvironmentEjectEvent(core::Timestamp time, r_code::Code* object)
    : AeraEvent(EVENT_TYPE, time, object)
  {}

  static const int EVENT_TYPE = 11;
};

}

#endif
