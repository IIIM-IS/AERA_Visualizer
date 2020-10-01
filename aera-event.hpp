//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020 Icelandic Institute for Intelligent Machines ses
//_/_/ Vitvelastofnun Islands ses, kt. 571209-0390
//_/_/ Author: Jeffrey Thompson <jeff@iiim.is>
//_/_/
//_/_/ -----------------------------------------------------------------------
//_/_/ Released under Open-Source BSD License with CADIA Clause v 1.0
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/   and collaboration notice, this list of conditions and the 
//_/_/   following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/   notice, this list of conditions and the following
//_/_/   disclaimer in the documentation and/or other materials provided 
//_/_/   with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/   contributors may be used to endorse or promote products 
//_/_/   derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause v 1.0: The license granted in and to the software under 
//_/_/   this agreement is a limited-use license. The software may not be used
//_/_/   in furtherance of: 
//_/_/   (i) intentionally causing bodily injury or severe emotional distress 
//_/_/   to any person; 
//_/_/   (ii) invading the personal privacy or violating the human rights of 
//_/_/   any person; or 
//_/_/   (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef AERA_EVENT_HPP
#define AERA_EVENT_HPP

#include <QPointF>
#include "submodules/replicode/r_code/object.h"
#include "submodules/replicode/r_exec/opcodes.h"

namespace aera_visualizer {

class AeraEvent {
public:
  AeraEvent(int eventType, core::Timestamp time, r_code::Code* object)
  : eventType_(eventType),
    time_(time),
    object_(object),
    itemTopLeftPosition_(qQNaN(), qQNaN())
  {}

  /**
   * A helper method to get the first item in the reduction's set of productions.
   * \param reduction The mk.rdx reduction.
   * \return The first production.
   */
  static r_code::Code* getFirstProduction(r_code::Code* reduction)
  {
    return reduction->get_reference(
      reduction->code(reduction->code(MK_RDX_PRODS).asIndex() + 1).asIndex());
  }

  /**
   * A helper method to get the second item in the reduction's set of inputs.
   * \param reduction The mk.rdx reduction.
   * \return The second input, or NULL if the set of inputs size is less than two.
   */
  static r_code::Code* getSecondInput(r_code::Code* reduction)
  {
    uint16 input_set_index = reduction->code(MK_RDX_INPUTS).asIndex();
    if (reduction->code(input_set_index).getAtomCount() < 2)
      return NULL;
    return reduction->get_reference(reduction->code(input_set_index + 2).asIndex());
  }

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

/**
 * ModelImdlPredictionEvent is a reduction event specific to a prediction of an imdl,
 * which doesn't have an associated requirement (as opposed to a mk.val prediction which does).
 */
class ModelImdlPredictionEvent : public AeraEvent {
public:
  /**
   * Create a ModelImdlPredictionEvent.
   * \param time The reduction time.
   * \param factPred The (fact (pred (fact (imdl ...)))).
   * \param predictingModel The model which made the prediction.
   * TODO: Get this from an mk.rdx, which Replicode currently doesn't make.
   * \param cause The input cause for the prediction.
   * TODO: Get this from a mk.rdx, which Replicode currently doesn't make.
   */
  ModelImdlPredictionEvent(core::Timestamp time, r_code::Code* factPred,
    r_code::Code* predictingModel, r_code::Code* cause)
    : AeraEvent(EVENT_TYPE, time, factPred),
    predictingModel_(predictingModel), cause_(cause)
  {}

  static const int EVENT_TYPE = 7;

  r_code::Code* predictingModel_;
  r_code::Code* cause_;
};

/**
 * ModelMkValPredictionReduction is a reduction event specific to a prediction of a mk.val,
 * which has an associated requirement (as opposed to a prediction of an imdl which doesn't).
 */
class ModelMkValPredictionReduction : public AeraEvent {
public:
  /**
   * Create a ModelMkValPredictionReduction, but set object_ to the (fact (pred ...)).
   * (mk.rdx fact_imdl [fact_cause fact_requirement] [fact_pred]) .
   * \param time The reduction time.
   * \param reduction The model reduction which points to the (fact (pred ...)) and the cause.
   * \param imdlPredictionEventIndex The index in the events_ list of the previous prediction whose 
   * object_ is this->getRequirement(), or -1 if this->getRequirement() is NULL.
   * TODO: This should be the debug_oid of an mk.rdx, which Replicode currently doesn't make.
   */
  ModelMkValPredictionReduction(core::Timestamp time, r_code::Code* reduction, int imdlPredictionEventIndex)
    // The prediction is the first item in the set of productions.
    : AeraEvent(EVENT_TYPE, time, getFirstProduction(reduction)),
    reduction_(reduction), imdlPredictionEventIndex_(imdlPredictionEventIndex)
  {}

  static const int EVENT_TYPE = 8;

  r_code::Code* getFactImdl() { return reduction_->get_reference(MK_RDX_IHLP_REF); }

  /**
   * Get the cause from the reduction_, which is the first item in the set of inputs.
   * \return The cause.
   */
  r_code::Code* getCause() {
    return reduction_->get_reference(
      reduction_->code(reduction_->code(MK_RDX_INPUTS).asIndex() + 1).asIndex());
  }

  /**
   * Get the requirement from the reduction_, which is the second item in the set of inputs.
   * \return The requirement, or NULL if the set of inputs has less than two items.
   */
  r_code::Code* getRequirement() { return getSecondInput(reduction_); }

  r_code::Code* getFactPred() { return object_; }

  r_code::Code* reduction_;
  int imdlPredictionEventIndex_;
};

/**
 * ModelGoalReduction is a reduction event of a model abduction to a goal.
 */
class ModelGoalReduction : public AeraEvent {
public:
  /**
   * Create a ModelGoalReduction, but set object_ to the (fact (goal ...)).
   * (mk.rdx fact_imdl [fact_super_goal] [fact_goal]) .
   * \param time The reduction time.
   * \param reduction The model reduction which points to the (fact (goal ...)) and the super goal.
   */
  ModelGoalReduction(core::Timestamp time, r_code::Code* reduction)
    // The goal is the first (only) item in the set of productions.
    : AeraEvent(EVENT_TYPE, time, getFirstProduction(reduction)),
      reduction_(reduction)
  {}

  static const int EVENT_TYPE = 9;

  r_code::Code* getFactImdl() { return reduction_->get_reference(MK_RDX_IHLP_REF); }

  /**
   * Get the fact super goal from the reduction_, which is the first item in the set of inputs.
   * \return The fact super goal.
   */
  r_code::Code* getFactSuperGoal() {
    return reduction_->get_reference(
      reduction_->code(reduction_->code(MK_RDX_INPUTS).asIndex() + 1).asIndex());
  }

  r_code::Code* getFactGoal() { return object_; }

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

  static const int EVENT_TYPE = 10;

  std::vector<r_code::Code*> inputs_;
};

/**
 * Use PredictionResultEvent for both success and failure. Use the method isSuccess to distinguish.
 */
class PredictionResultEvent : public AeraEvent {
public:
  PredictionResultEvent(core::Timestamp time, r_code::Code* factSuccessFactPred)
    : AeraEvent(EVENT_TYPE, time, factSuccessFactPred)
  {}

  /**
   * Check if this is for a prediction success.
   * \return True for prediction success (fact) or false for prediction failure (anti-fact).
   */
  bool isSuccess() { return object_->code(0).asOpcode() == r_exec::Opcodes::Fact;  }

  static const int EVENT_TYPE = 11;
};

class EnvironmentInjectEvent : public AeraEvent {
public:
  EnvironmentInjectEvent(
    core::Timestamp time, r_code::Code* object, core::Timestamp injectionTime)
    : AeraEvent(EVENT_TYPE, time, object),
    injectionTime_(injectionTime)
  {}

  static const int EVENT_TYPE = 12;

  core::Timestamp injectionTime_;
};

class EnvironmentEjectEvent : public AeraEvent {
public:
  EnvironmentEjectEvent(core::Timestamp time, r_code::Code* object, r_code::Code* reduction)
    : AeraEvent(EVENT_TYPE, time, object),
    reduction_(reduction)
  {}

  static const int EVENT_TYPE = 13;

  r_code::Code* reduction_;
};

}

#endif
