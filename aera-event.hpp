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
#include "submodules/AERA/r_code/object.h"
#include "submodules/AERA/r_exec/opcodes.h"
#include "submodules/AERA/r_exec/factory.h"

namespace aera_visualizer {

class AeraEvent {
public:
  AeraEvent(int eventType, core::Timestamp time, r_code::Code* object)
  : eventType_(eventType),
    time_(time),
    object_(object),
    itemInitialTopLeftPosition_(qQNaN(), qQNaN()),
    itemTopLeftPosition_(qQNaN(), qQNaN())
  {}

  virtual AeraEvent::~AeraEvent() {}

  /**
   * A class can override this to specify the primary input of a reduction. 
   * \return The primary input, or null if not specified.
   */
  virtual r_code::Code* getInput() { return 0;  }

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
  // itemOriginalTopLeftPosition_ is used by "Reset Position" to restore the initial placement.
  QPointF itemInitialTopLeftPosition_;
  // itemTopLeftPosition_ is used by "New" events to remember the screen position after undoing.
  QPointF itemTopLeftPosition_;
};

class NewModelEvent : public AeraEvent {
public:
  NewModelEvent(core::Timestamp time, r_code::Code* model, core::float32 strength,
    core::float32 evidenceCount, core::float32 successRate, uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, model),
    strength_(strength),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 1;

  core::float32 strength_;
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

class SetModelStrengthEvent : public AeraEvent {
public:
  SetModelStrengthEvent
  (core::Timestamp time, r_code::Code* model, core::float32 strength)
    : AeraEvent(EVENT_TYPE, time, model),
    strength_(strength),
    oldStrength_(qQNaN())
  {}

  static const int EVENT_TYPE = 3;

  core::float32 strength_;
  core::float32 oldStrength_;
};

class PhaseOutModelEvent : public AeraEvent {
public:
  PhaseOutModelEvent(core::Timestamp time, r_code::Code* model)
    : AeraEvent(EVENT_TYPE, time, model)
  {}

  static const int EVENT_TYPE = 4;
};

class PhaseInModelEvent : public AeraEvent {
public:
  PhaseInModelEvent(core::Timestamp time, r_code::Code* model)
    : AeraEvent(EVENT_TYPE, time, model)
  {}

  static const int EVENT_TYPE = 5;
};

// TODO: Record whether it was phased out before deletion so that this can be restored during unstep.
class DeleteModelEvent : public AeraEvent {
public:
  DeleteModelEvent(core::Timestamp time, r_code::Code* model)
    : AeraEvent(EVENT_TYPE, time, model)
  {}

  static const int EVENT_TYPE = 6;
};

class NewCompositeStateEvent : public AeraEvent {
public:
  NewCompositeStateEvent(core::Timestamp time, r_code::Code* compositeState, 
      uint64 controllerDegugOid)
    : AeraEvent(EVENT_TYPE, time, compositeState),
    controllerDegugOid_(controllerDegugOid)
  {}

  static const int EVENT_TYPE = 7;

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

  static const int EVENT_TYPE = 8;

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

  static const int EVENT_TYPE = 9;

  r_code::Code* programReduction_;
};

class AutoFocusNewObjectEvent : public AeraEvent {
public:
  AutoFocusNewObjectEvent(core::Timestamp time, r_code::Code* fromObject, 
    r_code::Code* toObject, const std::string& syncMode)
    : AeraEvent(EVENT_TYPE, time, toObject),
    fromObject_(fromObject), syncMode_(syncMode)
  {}

  static const int EVENT_TYPE = 10;

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

  static const int EVENT_TYPE = 11;

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
   * TODO: This should be the detail_oid of an mk.rdx, which Replicode currently doesn't make.
   */
  ModelMkValPredictionReduction(core::Timestamp time, r_code::Code* reduction, int imdlPredictionEventIndex)
    // The prediction is the first item in the set of productions.
    : AeraEvent(EVENT_TYPE, time, getFirstProduction(reduction)),
    reduction_(reduction), imdlPredictionEventIndex_(imdlPredictionEventIndex)
  {}

  static const int EVENT_TYPE = 12;

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
   * Create a ModelGoalReduction, but set object_ to the (fact (goal...)).
   * \param time The reduction time.
   * \param model The model which did the reduction.
   * \param factGoal The (fact (goal...)) (production).
   * \param factSuperGoal The (fact (goal...)) super goal (input).
   */
  ModelGoalReduction(core::Timestamp time, r_code::Code* model, 
      r_code::Code* factGoal, r_code::Code* factSuperGoal)
    : AeraEvent(EVENT_TYPE, time, factGoal),
      model_(model), factGoal_((r_exec::_Fact*)factGoal), 
      factSuperGoal_((r_exec::_Fact*)factSuperGoal)
  {}

  r_code::Code* getInput() override { return factSuperGoal_; }

  static const int EVENT_TYPE = 13;

  r_code::Code* model_;
  r_exec::_Fact* factGoal_;
  r_exec::_Fact* factSuperGoal_;
};

/**
 * CompositeStateGoalReduction is a reduction event of a composite statue abduction to a goal.
 */
class CompositeStateGoalReduction : public AeraEvent {
public:
  /**
   * Create a CompositeStateGoalReduction, but set object_ to the (fact (goal...)).
   * \param time The reduction time.
   * \param compositeState The composite state which did the reduction.
   * \param factGoal The (fact (goal...)) (production).
   * \param factSuperGoal The (fact (goal...)) super goal (input).
   */
  CompositeStateGoalReduction(core::Timestamp time, r_code::Code* compositeState,
    r_code::Code* factGoal, r_code::Code* factSuperGoal)
    : AeraEvent(EVENT_TYPE, time, factGoal),
    compositeState_(compositeState), factGoal_((r_exec::_Fact*)factGoal),
    factSuperGoal_((r_exec::_Fact*)factSuperGoal)
  {}

  r_code::Code* getInput() override { return factSuperGoal_; }

  static const int EVENT_TYPE = 14;

  r_code::Code* compositeState_;
  r_exec::_Fact* factGoal_;
  r_exec::_Fact* factSuperGoal_;
};

/**
 * ModelSimulatedPredictionReduction is a reduction event of a model to a simulated prediction.
 */
class ModelSimulatedPredictionReduction : public AeraEvent {
public:
  /**
   * Create a ModelSimulatedPredictionReduction.
   * \param time The reduction time.
   * \param model The model which did the reduction.
   * \param factPred The (fact (pred...)) (production).
   * \param input The input fact triggering the reduction.
   * \param requirement The requirement used by the reduction (may be NULL).
   * \param inputIsSuperGoal True if the input is a super goal, meaning that this event
   * is the last step of simulated backward chaining to start forward chaining.
   * \param factPredIsImdl True if the prediction is of the imdl, not of the RHS.
   */
  ModelSimulatedPredictionReduction(core::Timestamp time, r_code::Code* model,
      r_code::Code* factPred, r_code::Code* input, r_code::Code* requirement, bool inputIsSuperGoal,
      bool factPredIsImdl)
    : AeraEvent(EVENT_TYPE, time, factPred),
      model_(model), factPred_((r_exec::_Fact*)factPred),
      input_((r_exec::_Fact*)input), requirement_((r_exec::_Fact*)requirement),
      inputIsSuperGoal_(inputIsSuperGoal), factPredIsImdl_(factPredIsImdl)
  {}

  r_code::Code* getInput() override { return input_; }

  static const int EVENT_TYPE = 15;

  r_code::Code* model_;
  r_exec::_Fact* factPred_;
  r_exec::_Fact* input_;
  r_exec::_Fact* requirement_;
  bool inputIsSuperGoal_;
  bool factPredIsImdl_;
};

/**
 * CompositeStateSimulatedPredictionReduction is a reduction event of a composite state to a simulated prediction.
 */
class CompositeStateSimulatedPredictionReduction : public AeraEvent {
public:
  /**
   * Create a CompositeStateSimulatedPredictionReduction.
   * \param time The reduction time.
   * \param compositeState The composite state which did the reduction.
   * \param factPred The (fact (pred...)) (production).
   * \param input The input fact triggering the reduction.
   * \param inputs The set of inputs which made the icst (one of which is input).
   */
  CompositeStateSimulatedPredictionReduction(core::Timestamp time, r_code::Code* compositeState,
    r_code::Code* factPred, r_code::Code* input, const std::vector<r_code::Code*>& inputs)
    : AeraEvent(EVENT_TYPE, time, factPred),
    compositeState_(compositeState), factPred_((r_exec::_Fact*)factPred),
    input_((r_exec::_Fact*)input), inputs_(inputs)
  {}

  r_code::Code* getInput() override { return input_; }

  static const int EVENT_TYPE = 16;

  r_code::Code* compositeState_;
  r_exec::_Fact* factPred_;
  r_exec::_Fact* input_;
  std::vector<r_code::Code*> inputs_;
};

class NewInstantiatedCompositeStateEvent : public AeraEvent {
public:
  NewInstantiatedCompositeStateEvent(
      core::Timestamp time, r_code::Code* instantiatedCompositeState,
      const std::vector<r_code::Code*>& inputs)
    : AeraEvent(EVENT_TYPE, time, instantiatedCompositeState),
    inputs_(inputs)
  {}

  static const int EVENT_TYPE = 17;

  std::vector<r_code::Code*> inputs_;
};

/* Use NewPredictedInstantiatedCompositeStateEvent for a new predicted (non-simulated) icst.
 */
class NewPredictedInstantiatedCompositeStateEvent : public AeraEvent {
public:
  /**
   * Create a NewPredictedInstantiatedCompositeStateEvent.
   * \param time The reduction time.
   * \param f_p_f_icst The new (fact (pred (fact (icst ...))))
   * \param inputs The list of inputs. Note that some may be predictions.
   */
  NewPredictedInstantiatedCompositeStateEvent(
    core::Timestamp time, r_code::Code* f_p_f_icst,
    const std::vector<r_code::Code*>& inputs)
    : AeraEvent(EVENT_TYPE, time, f_p_f_icst),
      inputs_(inputs)
  {}

  static const int EVENT_TYPE = 18;

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

  static const int EVENT_TYPE = 19;
};

class IoDeviceInjectEvent : public AeraEvent {
public:
  IoDeviceInjectEvent(
    core::Timestamp time, r_code::Code* object, core::Timestamp injectionTime)
    : AeraEvent(EVENT_TYPE, time, object),
    injectionTime_(injectionTime)
  {}

  static const int EVENT_TYPE = 20;

  core::Timestamp injectionTime_;
};

class IoDeviceEjectEvent : public AeraEvent {
public:
  IoDeviceEjectEvent(core::Timestamp time, r_code::Code* object, r_code::Code* reduction)
    : AeraEvent(EVENT_TYPE, time, object),
    reduction_(reduction)
  {}

  static const int EVENT_TYPE = 21;

  r_code::Code* reduction_;
};

class DriveInjectEvent : public AeraEvent {
public:
  DriveInjectEvent(
    core::Timestamp time, r_code::Code* object, core::Timestamp injectionTime)
    : AeraEvent(EVENT_TYPE, time, object),
    injectionTime_(injectionTime)
  {}

  static const int EVENT_TYPE = 22;

  core::Timestamp injectionTime_;
};

/**
 * The input of a SimulationCommitEvent is a simulated (fact (pred (fact (success...)))),
 * and the output is a commited non-simulated (fact (goal...)) which is the RHS of a 
 * model whose LHS is the committed action.
 */
class SimulationCommitEvent : public AeraEvent {
public:
  /**
   * Create a SimulationCommitEvent
   * \param time The event time.
   * \param factGoal The (fact (goal...)) (committed RHS).
   * \param factPredFactSuccess The (fact (pred (fact (success...)))).
   */
  SimulationCommitEvent(core::Timestamp time,
    r_code::Code* factGoal, r_code::Code* factPredFactSuccess)
    : AeraEvent(EVENT_TYPE, time, factGoal),
    factPredFactSuccess_((r_exec::_Fact*)factPredFactSuccess)
  {}

  r_code::Code* getInput() override { return factPredFactSuccess_; }

  static const int EVENT_TYPE = 23;

  r_exec::_Fact* factPredFactSuccess_;
};

/**
 * ModelSimulatedPredictionReductionFromRequirement is a reduction event of a model from a predicted requirement
 * to a simulated prediction of the LHS.
 */
class ModelSimulatedPredictionReductionFromRequirement : public AeraEvent {
public:
  /**
   * Create a ModelSimulatedPredictionReductionFromRequirement.
   * \param time The reduction time.
   * \param model The model which did the reduction.
   * \param factPred The (fact (pred...)) (production).
   * \param input The input fact triggering the reduction.
   * \param goal_requirement The original goal requirement (the target of the signalled SRMonitor).
   */
  ModelSimulatedPredictionReductionFromRequirement(core::Timestamp time, r_code::Code* model,
    r_code::Code* factPred, r_code::Code* input, r_code::Code* goal_requirement)
    : AeraEvent(EVENT_TYPE, time, factPred),
    model_(model), factPred_((r_exec::_Fact*)factPred),
    input_((r_exec::_Fact*)input), goal_requirement_((r_exec::_Fact*)goal_requirement)
  {}

  r_code::Code* getInput() override { return input_; }

  static const int EVENT_TYPE = 24;

  r_code::Code* model_;
  r_exec::_Fact* factPred_;
  r_exec::_Fact* input_;
  r_exec::_Fact* goal_requirement_;
};

/**
 * ModelSimulatedPredictionFromRequirementDisabledEvent is the event when a weak requirement is disabled by
 * a strong requirement.
 */
class ModelSimulatedPredictionFromRequirementDisabledEvent : public AeraEvent {
public:
  /**
   * Create a ModelSimulatedPredictionFromRequirementDisabledEvent. This sets object_ to NULL, since this is
   * an event for disabling the production of an output.
   * \param time The time of the disable event.
   * \param model The model which did the reduction.
   * \param input The input fact (weak requirement) triggering the event.
   * \param goal_requirement The original goal requirement (the target of the signalled SRMonitor).
   * \param strong_requirement The strong requirement which disabled the input (weak requirement).
   */
  ModelSimulatedPredictionFromRequirementDisabledEvent(core::Timestamp time, r_code::Code* model,
    r_code::Code* input, r_code::Code* goal_requirement, r_code::Code* strong_requirement)
    : AeraEvent(EVENT_TYPE, time, NULL),
    model_(model), input_((r_exec::_Fact*)input),
    goal_requirement_((r_exec::_Fact*)goal_requirement),
    strong_requirement_((r_exec::_Fact*)strong_requirement)
  {}

  r_code::Code* getInput() override { return input_; }

  static const int EVENT_TYPE = 25;

  r_code::Code* model_;
  r_exec::_Fact* input_;
  r_exec::_Fact* goal_requirement_;
  r_exec::_Fact* strong_requirement_;
};

class PromotedSimulatedPredictionEvent : public AeraEvent {
public:
  /**
   * Create a PromotedSimulatedPredictionEvent
   * \param time The event time.
   * \param promotedFact The promoted fact.
   * \param promotedFromFact The fact from which the promoted fact was made.
   * \param timingsFact The fact whose timings are used for the promoted fact (treated as the input).
   */
  PromotedSimulatedPredictionEvent(core::Timestamp time,
    r_code::Code* promotedFact, r_code::Code* promotedFromFact, r_code::Code* timingsFact)
    : AeraEvent(EVENT_TYPE, time, promotedFact),
    promotedFromFact_((r_exec::_Fact*)promotedFromFact),
    timingsFact_((r_exec::_Fact*)timingsFact)
  {}

  r_code::Code* getInput() override { return timingsFact_; }

  static const int EVENT_TYPE = 26;

  r_exec::_Fact* promotedFromFact_;
  r_exec::_Fact* timingsFact_;
};

}

#endif
