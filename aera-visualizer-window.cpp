//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020-2021 Icelandic Institute for Intelligent Machines ses
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

#include <fstream>
#include <algorithm>
#include "submodules/replicode/r_exec/opcodes.h"
#include "graphics-items/arrow.hpp"
#include "graphics-items/model-item.hpp"
#include "graphics-items/composite-state-item.hpp"
#include "graphics-items/auto-focus-fact-item.hpp"
#include "graphics-items/prediction-item.hpp"
#include "graphics-items/model-goal-item.hpp"
#include "graphics-items/composite-state-goal-item.hpp"
#include "graphics-items/model-prediction-item.hpp"
#include "graphics-items/model-imdl-prediction-item.hpp"
#include "graphics-items/model-prediction-from-requirement-item.hpp"
#include "graphics-items/composite-state-prediction-item.hpp"
#include "graphics-items/prediction-result-item.hpp"
#include "graphics-items/instantiated-composite-state-item.hpp"
#include "graphics-items/io-device-inject-eject-item.hpp"
#include "graphics-items/drive-item.hpp"
#include "graphics-items/simulation-commit-item.hpp"
#include "graphics-items/aera-visualizer-scene.hpp"
#include "aera-visualizer-window.hpp"

#include <QtWidgets>
#include <QProgressDialog>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

/**
 * A MyQGraphicsView extends QGraphicsView so that we can override scrollContentsBy to
 * call scene_->onViewMoved().
 */
class MyQGraphicsView : public QGraphicsView {
public:
  MyQGraphicsView(AeraVisualizerScene* scene, QWidget* parent)
  : QGraphicsView(scene, parent),
    scene_(scene)
  {}

protected:
  void scrollContentsBy(int dx, int dy) override
  {
    QGraphicsView::scrollContentsBy(dx, dy);
    scene_->onViewMoved();
  }

  AeraVisualizerScene* scene_;
};

const set<int> AeraVisulizerWindow::simulationEventTypes_ = {
  DriveInjectEvent::EVENT_TYPE, ModelGoalReduction::EVENT_TYPE, CompositeStateGoalReduction::EVENT_TYPE,
  ModelSimulatedPredictionReduction::EVENT_TYPE, CompositeStateSimulatedPredictionReduction::EVENT_TYPE,
  ModelSimulatedPredictionReductionFromRequirement::EVENT_TYPE, SimulationCommitEvent ::EVENT_TYPE};

const set<int> AeraVisulizerWindow::newItemEventTypes_ = {
  NewModelEvent::EVENT_TYPE,
  NewCompositeStateEvent::EVENT_TYPE,
  AutoFocusNewObjectEvent::EVENT_TYPE,
  ModelMkValPredictionReduction::EVENT_TYPE,
  ModelImdlPredictionEvent::EVENT_TYPE,
  ModelGoalReduction::EVENT_TYPE,
  CompositeStateGoalReduction::EVENT_TYPE,
  ModelSimulatedPredictionReduction::EVENT_TYPE,
  ModelSimulatedPredictionReductionFromRequirement::EVENT_TYPE,
  CompositeStateSimulatedPredictionReduction::EVENT_TYPE,
  PredictionResultEvent::EVENT_TYPE,
  NewInstantiatedCompositeStateEvent::EVENT_TYPE,
  IoDeviceInjectEvent::EVENT_TYPE,
  IoDeviceEjectEvent::EVENT_TYPE,
  DriveInjectEvent::EVENT_TYPE,
  SimulationCommitEvent::EVENT_TYPE };

AeraVisulizerWindow::AeraVisulizerWindow(ReplicodeObjects& replicodeObjects)
: AeraVisulizerWindowBase(0, replicodeObjects),
  iNextEvent_(0), explanationLogWindow_(0),
  essencePropertyObject_(replicodeObjects_.getObject("essence")),
  hoverHighlightItem_(0),
  showRelativeTime_(true),
  playTime_(seconds(0)),
  playTimerId_(0),
  isPlaying_(false),
  itemBorderHighlightPen_(Qt::blue, 3)
{
  createActions();
  createMenus();

  setPlayTime(replicodeObjects_.getTimeReference());

  createToolbars();

  modelsScene_ = new AeraVisualizerScene(replicodeObjects_, this, false,
    [=]() { selectedScene_ = modelsScene_; });
  auto modelsSceneView = new QGraphicsView(modelsScene_, this);
  modelsSceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  modelsSceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  mainScene_ = new AeraVisualizerScene(replicodeObjects_, this, true,
    [=]() { selectedScene_ = mainScene_; });
  // Use a MyQGraphicsView so that we can track movements to the scene view.
  auto mainSceneView = new MyQGraphicsView(mainScene_, this);
  mainSceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  mainSceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // Set a default selected scene.
  selectedScene_ = mainScene_;

  auto splitter = new QSplitter(this);
  splitter->addWidget(modelsSceneView);
  splitter->addWidget(mainSceneView);
  // The splitter sizes are proportional.
  splitter->setSizes(QList<int>() << 100 << 750);

  auto centralLayout = new QVBoxLayout();
  // A stretch factor of 1, vs. the playerControlPanel factor of 0, makes the splitter maximize its space.
  centralLayout->addWidget(splitter, 1);
  centralLayout->addWidget(getPlayerControlPanel());

  auto centralWidget = new QWidget();
  centralWidget->setLayout(centralLayout);
  setCentralWidget(centralWidget);

  setWindowTitle(tr("AERA Visualizer"));
  setUnifiedTitleAndToolBarOnMac(true);
}

bool AeraVisulizerWindow::addEvents(const string& runtimeOutputFilePath)
{
  // load mdl 37, MDLController(113)
  regex loadModelRegex("^load mdl (\\d+), MDLController\\((\\d+)\\) cnt:(\\d+) sr:([\\d\\.]+)$");
  // load cst 36, CSTController(98)
  regex loadCompositeStateRegex("^load cst (\\d+), CSTController\\((\\d+)\\)$");

  // The remaining regex expressions all start with a timestamp.
  regex timestampRegex("^(\\d+)s:(\\d+)ms:(\\d+)us (.+)$");

  // -> mdl 53, MDLController(389)
  regex newModelRegex("^-> mdl (\\d+), MDLController\\((\\d+)\\)$");
  // mdl 53 cnt:2 sr:1
  regex setEvidenceCountAndSuccessRateRegex("^mdl (\\d+) cnt:(\\d+) sr:([\\d\\.]+)$");
  // mdl 53 deleted
  regex deleteModelRegex("^mdl (\\d+) deleted$");
  // -> cst 52, CSTController(375)
  regex newCompositeStateRegex("^-> cst (\\d+), CSTController\\((\\d+)\\)$");
  // A/F -> 35|40 (AXIOM)
  regex autofocusNewObjectRegex("^A/F -> (\\d+)\\|(\\d+) \\((\\w+)\\)$");
  // mdl 61 predict imdl -> mk.rdx 559
  regex modelImdlPredictionReductionRegex("^mdl \\d+ predict imdl -> mk.rdx (\\d+)$");
  // mdl 64: fact (58313) pred fact imdl -> fact 220 simulated pred
  regex modelSimulatedPredictionFromRequirementRegex("^mdl (\\d+): fact \\((\\d+)\\) pred fact imdl -> fact (\\d+) simulated pred$");
  // mdl 63 predict -> mk.rdx 68
  regex modelPredictionReductionRegex("^mdl \\d+ predict -> mk.rdx (\\d+)$");
  // mdl 41 abduce -> mk.rdx 97
  regex modelAbductionReductionRegex("^mdl \\d+ abduce -> mk.rdx (\\d+)$");
  // mdl 64: fact 96 super_goal -> fact 98 simulated goal
  regex modelSimulatedAbductionRegex("^mdl (\\d+): fact (\\d+) super_goal -> fact (\\d+) simulated goal$");
  // cst 64: fact 96 super_goal -> fact 98 simulated goal
  regex compositeStateSimulatedAbductionRegex("^cst (\\d+): fact (\\d+) super_goal -> fact (\\d+) simulated goal$");
  // mdl 57: fact 202 pred -> fact 227 simulated pred
  regex modelSimulatedPredictionRegex("^mdl (\\d+): fact (\\d+) (pred|super_goal) -> fact (\\d+) simulated pred$");
  // mdl 60: fact 181 super_goal -> fact (41817) simulated pred start
  regex modelSimulatedPredictionStartRegex("^mdl (\\d+): fact (\\d+) super_goal -> fact \\((\\d+)\\) simulated pred start, ijt (\\d+)s:(\\d+)ms:(\\d+)us$");
  // cst 60: fact 195 -> fact 218 simulated pred fact icst [ 155 191]
  regex compositeStateSimulatedPredictionRegex("^cst (\\d+): fact (\\d+) -> fact (\\d+) simulated pred fact icst \\[([ \\d]+)\\]$");
  // fact 59 icst[52][ 50 55]
  regex newInstantiatedCompositeStateRegex("^fact (\\d+) icst\\[\\d+\\]\\[([ \\d]+)\\]$");
  // fact 75 -> fact 79 success fact 60 pred
  regex predictionSuccessRegex("^fact (\\d+) -> fact (\\d+) success fact \\d+ pred$");
  // |fact 72 fact 59 pred failure
  regex predictionFailureRegex("^\\|fact (\\d+) fact \\d+ pred failure$");
  // fact 121: 96 goal success (TopLevel)
  regex topLevelGoalSuccessRegex("^fact (\\d+): (\\d+) goal success \\(TopLevel\\)$");
  // I/O device inject 46, ijt 0s:200ms:0us
  regex ioDeviceInjectRegex("^I/O device inject (\\d+), ijt (\\d+)s:(\\d+)ms:(\\d+)us$");
  // mk.rdx(100): I/O device eject 39
  regex ioDeviceEjectWithRdxRegex("^mk.rdx\\((\\d+)\\): I/O device eject (\\d+)$");
  // I/O device eject 39
  regex ioDeviceEjectWithoutRdxRegex("^I/O device eject (\\d+)$");
  // -> drive 158, ijt 0s:310ms:0us
  regex driveInjectRegex("^-> drive (\\d+), ijt (\\d+)s:(\\d+)ms:(\\d+)us$");
  // sim commit: fact 238 pred fact success -> fact (82115) goal
  regex simulationCommitRegex("^sim commit: fact (\\d+) pred fact success -> fact \\((\\d+)\\) goal$");

  QProgressDialog progress("Reading " + QFileInfo(runtimeOutputFilePath.c_str()).fileName() + "...", "Cancel", 0, 100, this);
  progress.setWindowModality(Qt::WindowModal);
  // Remove the '?' in the title.
  progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  progress.setWindowTitle("Initializing");
  QApplication::processEvents();

  // Count the number of lines, to use in the progress dialog.
  int nLines;
  {
    ifstream fileForCount(runtimeOutputFilePath);
    nLines = std::count(istreambuf_iterator<char>(fileForCount), istreambuf_iterator<char>(), '\n');
  }
  progress.setMaximum(nLines);

  std::vector<shared_ptr<AeraEvent> > pendingEvents;
  ifstream runtimeOutputFile(runtimeOutputFilePath);
  int lineNumber = 0;
  string line;
  while (getline(runtimeOutputFile, line)) {
    if (progress.wasCanceled())
      return false;

    ++lineNumber;
    progress.setValue(lineNumber);
    if (lineNumber % 100 == 0)
      QApplication::processEvents();

    smatch matches;

    if (regex_search(line, matches, loadModelRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model) {
        // Restore the initial count and success rate.
        core::float32 evidenceCount = stol(matches[3].str());
        core::float32 successRate = stof(matches[4].str());
        model->code(MDL_CNT) = Atom::Float(evidenceCount);
        model->code(MDL_SR) = Atom::Float(successRate);
        startupEvents_.push_back(make_shared <NewModelEvent>(
          replicodeObjects_.getTimeReference(), model, evidenceCount, successRate, stoll(matches[2].str())));
      }

      continue;
    }
    else if (regex_search(line, matches, loadCompositeStateRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (compositeState)
        startupEvents_.push_back(make_shared <NewCompositeStateEvent>(
          replicodeObjects_.getTimeReference(), compositeState, stoll(matches[2].str())));

      continue;
    }

    // The remaining regex expressions all start with a timestamp.
    if (!regex_search(line, matches, timestampRegex))
      continue;
    core::Timestamp timestamp = getTimestamp(matches);
    string lineAfterTimestamp = matches[4].str();

    while (pendingEvents.size() >= 1 && pendingEvents.front()->time_ <= timestamp) {
      // Insert the pending event before this new event.
      events_.push_back(pendingEvents.front());
      pendingEvents.erase(pendingEvents.begin());
    }

    if (regex_search(lineAfterTimestamp, matches, newModelRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model)
        // Assume the initial success rate is 1.
        events_.push_back(make_shared<NewModelEvent>(
          timestamp, model, 1, 1, stoll(matches[2].str())));
    }
    else if (regex_search(lineAfterTimestamp, matches, setEvidenceCountAndSuccessRateRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model)
        events_.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(
          timestamp, model, stol(matches[2].str()), stof(matches[3].str())));
    }
    else if (regex_search(lineAfterTimestamp, matches, deleteModelRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model)
        events_.push_back(make_shared<DeleteModelEvent>(timestamp, model));
    }
    else if (regex_search(lineAfterTimestamp, matches, newCompositeStateRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (compositeState)
        events_.push_back(make_shared<NewCompositeStateEvent>(
          timestamp, compositeState, stoll(matches[2].str())));
    }
    else if (regex_search(lineAfterTimestamp, matches, autofocusNewObjectRegex)) {
      auto fromObject = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto toObject = replicodeObjects_.getObject(stoul(matches[2].str()));
      // Skip auto-focus of the same fact (such as eject facts).
      // But show auto-focus of the same anti-fact (such as prediction failure).
      if (fromObject && toObject /*debug && !(fromObject == toObject && fromObject->code(0).asOpcode() == Opcodes::Fact) */)
        events_.push_back(make_shared<AutoFocusNewObjectEvent>(
          timestamp, fromObject, toObject, matches[3].str()));
    }
    else if (regex_search(lineAfterTimestamp, matches, modelImdlPredictionReductionRegex)) {
      auto reduction = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (reduction) {
        auto factImdl = reduction->get_reference(MK_RDX_IHLP_REF);
        auto model = factImdl->get_reference(0)->get_reference(0);
        // The super goal is the first (only) item in the set of inputs.
        auto cause = reduction->get_reference(
          reduction->code(reduction->code(MK_RDX_INPUTS).asIndex() + 1).asIndex());
        // The prediction is the first (only) item in the set of productions.
        auto factPred = AeraEvent::getFirstProduction(reduction);

        if (model && cause && factPred) {
          if (((_Fact*)factPred)->get_pred()->is_simulation())
            events_.push_back(make_shared<ModelSimulatedPredictionReduction>(
              timestamp, model, factPred, cause, false));
          else
            events_.push_back(make_shared<ModelImdlPredictionEvent>(
              timestamp, factPred, model, cause));
        }
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, modelSimulatedPredictionFromRequirementRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto input = replicodeObjects_.getObjectByDetailOid(stoul(matches[2].str()));

      if (model && factPred && input)
        events_.push_back(make_shared<ModelSimulatedPredictionReductionFromRequirement>(
          timestamp, model, factPred, input));
    }
    else if (regex_search(lineAfterTimestamp, matches, modelPredictionReductionRegex)) {
      auto reduction = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (reduction) {
        // Check the type of prediction.
        auto factPred = AeraEvent::getFirstProduction(reduction);
        auto pred = factPred->get_reference(0);
        auto factValue = pred->get_reference(0);
        auto value = factValue->get_reference(0);
        auto valueOpcode = value->code(0).asOpcode();

        if (valueOpcode == Opcodes::MkVal) {
          int imdlPredictionEventIndex = -1;
          auto requirement = AeraEvent::getSecondInput(reduction);
          if (requirement) {
            // Search events_ backwards for the previous prediction whose object_ is this->getRequirement().
            for (int i = events_.size() - 1; i >= 0; --i) {
              if (events_[i]->eventType_ == ModelImdlPredictionEvent::EVENT_TYPE &&
                  ((ModelImdlPredictionEvent*)events_[i].get())->object_ == requirement) {
                imdlPredictionEventIndex = i;
                break;
              }
            }
          }

          events_.push_back(make_shared<ModelMkValPredictionReduction>(
            timestamp, reduction, imdlPredictionEventIndex));
        }
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, modelAbductionReductionRegex)) {
      auto reduction = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (reduction) {
        auto factImdl = reduction->get_reference(MK_RDX_IHLP_REF);
        auto model = factImdl->get_reference(0)->get_reference(0);
        // The goal is the first (only) item in the set of productions.
        auto factGoal = AeraEvent::getFirstProduction(reduction);
        // The super goal is the first item in the set of inputs.
        auto factSuperGoal = reduction->get_reference(
          reduction->code(reduction->code(MK_RDX_INPUTS).asIndex() + 1).asIndex());
        events_.push_back(make_shared<ModelGoalReduction>(
          timestamp, model, factGoal, factSuperGoal));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, modelSimulatedAbductionRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factGoal = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto factSuperGoal = replicodeObjects_.getObject(stoul(matches[2].str()));
      if (model && factGoal && factSuperGoal)
        events_.push_back(make_shared<ModelGoalReduction>(
          timestamp, model, factGoal, factSuperGoal));
    }
    else if (regex_search(lineAfterTimestamp, matches, compositeStateSimulatedAbductionRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factGoal = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto factSuperGoal = replicodeObjects_.getObject(stoul(matches[2].str()));
      if (compositeState && factGoal && factSuperGoal)
        events_.push_back(make_shared<CompositeStateGoalReduction>(
          timestamp, compositeState, factGoal, factSuperGoal));
    }
    else if (regex_search(lineAfterTimestamp, matches, modelSimulatedPredictionRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[2].str()));

      if (model && factPred && input)
        events_.push_back(make_shared<ModelSimulatedPredictionReduction>(
          timestamp, model, factPred, input, matches[3] == "super_goal"));
    }
    else if (regex_search(lineAfterTimestamp, matches, modelSimulatedPredictionStartRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factPred = replicodeObjects_.getObjectByDetailOid(stoul(matches[3].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[2].str()));

      if (model && factPred && input) {
        core::Timestamp injectionTime = getTimestamp(matches, 4);
        if (injectionTime < timestamp)
          // We don't expect this, but the runtime would not have injected earlier.
          injectionTime = timestamp;
        // TODO: Use an AeraEvent with the details of starting the simulated forward chaining.
        auto event = make_shared<ModelSimulatedPredictionReduction>(injectionTime, model, factPred, input, true);
        // Put in pendingEvents to be added to events_ later.
        pendingEvents.push_back(event);
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, compositeStateSimulatedPredictionRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[2].str()));

      // Get the matching inputs.
      string inputOids = matches[4].str();
      std::vector<Code*> inputs;
      bool gotAllInputs = true;
      while (regex_search(inputOids, matches, regex("( \\d+)"))) {
        auto input = replicodeObjects_.getObject(stoul(matches[1].str()));
        if (!input) {
          gotAllInputs = false;
          break;
        }
        inputs.push_back(input);

        inputOids = matches.suffix();
      }

      if (compositeState && factPred && input && gotAllInputs)
        events_.push_back(make_shared<CompositeStateSimulatedPredictionReduction>(
          timestamp, compositeState, factPred, input, inputs));
    }
    else if (regex_search(lineAfterTimestamp, matches, newInstantiatedCompositeStateRegex)) {
      auto instantiatedCompositeState = replicodeObjects_.getObject(stoul(matches[1].str()));

      // Get the matching inputs.
      string inputOids = matches[2].str();
      std::vector<Code*> inputs;
      bool gotAllInputs = true;
      while (regex_search(inputOids, matches, regex("( \\d+)"))) {
        auto input = replicodeObjects_.getObject(stoul(matches[1].str()));
        if (!input) {
          gotAllInputs = false;
          break;
        }
        inputs.push_back(input);

        inputOids = matches.suffix();
      }

      if (instantiatedCompositeState && gotAllInputs)
        events_.push_back(make_shared<NewInstantiatedCompositeStateEvent>(
          timestamp, instantiatedCompositeState, inputs));
    }
    else if (regex_search(lineAfterTimestamp, matches, predictionSuccessRegex)) {
      auto factSuccessFactPred = replicodeObjects_.getObject(stoul(matches[2].str()));
      if (factSuccessFactPred)
        events_.push_back(make_shared<PredictionResultEvent>(
          timestamp, factSuccessFactPred));
    }
    else if (regex_search(lineAfterTimestamp, matches, predictionFailureRegex)) {
      auto antiFactSuccessFactPred = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (antiFactSuccessFactPred)
        events_.push_back(make_shared<PredictionResultEvent>(
          timestamp, antiFactSuccessFactPred));
    }
    else if (regex_search(lineAfterTimestamp, matches, topLevelGoalSuccessRegex)) {
      auto factSuccessFactGoal = replicodeObjects_.getObject(stoul(matches[1].str()));
    }
    else if (regex_search(lineAfterTimestamp, matches, ioDeviceInjectRegex)) {
      auto object = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (object)
        events_.push_back(make_shared<IoDeviceInjectEvent>(
          timestamp, object, getTimestamp(matches, 2)));
    }
    else if (regex_search(lineAfterTimestamp, matches, ioDeviceEjectWithRdxRegex)) {
      auto reduction = replicodeObjects_.getObjectByDetailOid(stoul(matches[1].str()));
      auto object = replicodeObjects_.getObject(stoul(matches[2].str()));
      if (object)
        events_.push_back(make_shared<IoDeviceEjectEvent>(
          timestamp, object, reduction));
    }
    else if (regex_search(lineAfterTimestamp, matches, ioDeviceEjectWithoutRdxRegex)) {
      auto object = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (object)
        events_.push_back(make_shared<IoDeviceEjectEvent>(
          timestamp, object, (Code*)NULL));
    }
    else if (regex_search(lineAfterTimestamp, matches, driveInjectRegex)) {
      auto object = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (object)
        events_.push_back(make_shared<DriveInjectEvent>(
          timestamp, object, getTimestamp(matches, 2)));
    }
    else if (regex_search(lineAfterTimestamp, matches, simulationCommitRegex)) {
      auto factGoal = replicodeObjects_.getObjectByDetailOid(stoul(matches[2].str()));
      auto factPredFactSuccess = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (factGoal && factPredFactSuccess)
        events_.push_back(make_shared<SimulationCommitEvent>(
          timestamp, factGoal, factPredFactSuccess));
    }
  }

  // Transfer any remaining pendingEvents to events_.
  for (auto event = pendingEvents.begin(); event != pendingEvents.end(); ++event)
    events_.push_back(*event);
  pendingEvents.clear();

  return true;
}

void AeraVisulizerWindow::addStartupItems()
{
  for (int i = 0; i < startupEvents_.size(); ++i) {
    AeraEvent* event = startupEvents_[i].get();
    if (event->time_ > replicodeObjects_.getTimeReference())
      // Finished scanning the initial events.
      return;

    if (event->eventType_ == NewModelEvent::EVENT_TYPE)
      // TODO: Add arrows.
      modelsScene_->addAeraGraphicsItem(
        new ModelItem((NewModelEvent*)event, replicodeObjects_, modelsScene_));
    else if (event->eventType_ == NewCompositeStateEvent::EVENT_TYPE)
      // TODO: Add arrows.
      modelsScene_->addAeraGraphicsItem(
        new CompositeStateItem((NewCompositeStateEvent*)event, replicodeObjects_, modelsScene_));
  }
}

Timestamp AeraVisulizerWindow::getTimestamp(const smatch& matches, int index)
{
  microseconds us(1000000 * stoll(matches[index].str()) +
                     1000 * stoll(matches[index + 1].str()) +
                            stoll(matches[index + 2].str()));
  return replicodeObjects_.getTimeReference() + us;
}

AeraGraphicsItem* AeraVisulizerWindow::getAeraGraphicsItem(r_code::Code* object, AeraVisualizerScene** scene)
{
  if (scene)
    // Initialize to default NULL.
    *scene = 0;

  auto item = modelsScene_->getAeraGraphicsItem(object);
  if (item) {
    if (scene)
      *scene = modelsScene_;
    return item;
  }

  item = mainScene_->getAeraGraphicsItem(object);
  if (item) {
    if (scene)
      *scene = mainScene_;
    return item;
  }

  return NULL;
}

void AeraVisulizerWindow::zoomToAeraGraphicsItem(r_code::Code* object)
{
  AeraVisualizerScene* scene;
  auto item = getAeraGraphicsItem(object, &scene);
  if (item) {
    if (item == hoverHighlightItem_ && !hoverHighlightItemWasVisible_)
      // The item is temporarily visible while hovering. Make sure it stays visible when we un-hover.
      hoverHighlightItemWasVisible_ = true;

    scene->zoomToItem(item);
  }
}

void AeraVisulizerWindow::textItemHoverMoveEvent(const QTextDocument* document, QPointF position)
{
  auto url = document->documentLayout()->anchorAt(position);
  if (url == "") {
    // The mouse cursor exited the link.
    if (hoverHighlightItem_) {
      // Clear the previous highlighting and restore the visible state.
      hoverHighlightItem_->setPen(hoverHighlightItem_->getBorderNoHighlightPen());
      hoverHighlightItem_->setItemAndArrowsAndHorizontalLinesVisible(hoverHighlightItemWasVisible_);
      hoverHighlightItem_ = 0;
    }

    hoverPreviousUrl_ = "";
    return;
  }
  if (url == hoverPreviousUrl_)
    // Still hovering the same link, so do nothing.
    return;

  hoverPreviousUrl_ = url;
  if (url.startsWith("#detail_oid-")) {
    // Highlight the linked item.
    uint64 detail_oid = url.mid(12).toULongLong();
    auto object = replicodeObjects_.getObjectByDetailOid(detail_oid);
    if (object) {
      if (hoverHighlightItem_) {
        // Unhighlight a previous object.
        hoverHighlightItem_->setPen(hoverHighlightItem_->getBorderNoHighlightPen());
        hoverHighlightItem_->setItemAndArrowsAndHorizontalLinesVisible(hoverHighlightItemWasVisible_);
        hoverHighlightItem_ = 0;
      }

      hoverHighlightItem_ = getAeraGraphicsItem(object);
      if (hoverHighlightItem_) {
        hoverHighlightItemWasVisible_ = hoverHighlightItem_->isVisible();
        if (!hoverHighlightItemWasVisible_)
          // Make the item visible while we hover.
          hoverHighlightItem_->setItemAndArrowsAndHorizontalLinesVisible(true);

        hoverHighlightItem_->setPen(itemBorderHighlightPen_);
      }
    }
  }
}

Timestamp AeraVisulizerWindow::getINextStepEvent
  (Timestamp maximumTime, size_t iNextEventStart, size_t& iNextStepEvent)
{
  // TODO: This has to closely track stepEvent to duplicate its logic, so stepEvent should be
  // rewritten to provide the functionality to show what the next event would be without doing it.
  if (iNextEventStart >= events_.size())
    // Return the value meaning no change.
    return Utils_MaxTime;

  AeraEvent* event = events_[iNextEventStart].get();
  if (event->time_ > maximumTime)
    return Utils_MaxTime;

  // Default to the same initial event.
  iNextStepEvent = iNextEventStart;

  if (newItemEventTypes_.find(event->eventType_) != newItemEventTypes_.end()) {
    if (event->eventType_ == AutoFocusNewObjectEvent::EVENT_TYPE) {
      if (event->time_ == replicodeObjects_.getTimeReference())
        // Debug: For now, skip auto focus events at startup.
        return getINextStepEvent(maximumTime, iNextEventStart + 1, iNextStepEvent);
    }
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE ||
           event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    // We already set the default iNextStepEvent.
  }
  else
    // Skip this event.
    return getINextStepEvent(maximumTime, iNextEventStart + 1, iNextStepEvent);

  return event->time_;
}

Timestamp AeraVisulizerWindow::stepEvent(Timestamp maximumTime)
{
  if (iNextEvent_ >= events_.size())
    // Return the value meaning no change.
    return Utils_MaxTime;

  AeraEvent* event = events_[iNextEvent_].get();
  if (event->time_ > maximumTime)
    return Utils_MaxTime;

  if (newItemEventTypes_.find(event->eventType_) != newItemEventTypes_.end()) {
    AeraGraphicsItem* newItem;
    bool visible = true;

    AeraVisualizerScene* scene;
    if (event->eventType_ == NewModelEvent::EVENT_TYPE ||
      event->eventType_ == NewCompositeStateEvent::EVENT_TYPE)
      scene = modelsScene_;
    else
      scene = mainScene_;

    if (event->eventType_ == NewModelEvent::EVENT_TYPE) {
      auto newModelEvent = (NewModelEvent*)event;

      // Restore the evidence count and success rate in case we did a rewind.
      newModelEvent->object_->code(MDL_CNT) = Atom::Float(newModelEvent->evidenceCount_);
      newModelEvent->object_->code(MDL_SR) = Atom::Float(newModelEvent->successRate_);

      newItem = new ModelItem(newModelEvent, replicodeObjects_, scene);
    }
    else if (event->eventType_ == NewCompositeStateEvent::EVENT_TYPE)
      newItem = new CompositeStateItem((NewCompositeStateEvent*)event, replicodeObjects_, scene);
    else if (event->eventType_ == AutoFocusNewObjectEvent::EVENT_TYPE) {
      auto autoFocusEvent = (AutoFocusNewObjectEvent*)event;
      if (event->time_ == replicodeObjects_.getTimeReference()) {
        // Debug: For now, skip auto focus events at startup.
        ++iNextEvent_;
        return stepEvent(maximumTime);
      }

      newItem = new AutoFocusFactItem(autoFocusEvent, replicodeObjects_, scene);

      // Add an arrow to the "from object".
      auto fromObjectItem = scene->getAeraGraphicsItem(autoFocusEvent->fromObject_);
      if (fromObjectItem)
        scene->addArrow(fromObjectItem, newItem);

      auto mkVal = autoFocusEvent->fromObject_->get_reference(0);
      if (essencePropertyObject_ && mkVal->references_size() >= 2 && mkVal->get_reference(1) == essencePropertyObject_)
        visible = ((nonSimulationsCheckBox_->checkState() == Qt::Checked) && (essenceFactsCheckBox_->checkState() == Qt::Checked));
      else
        visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelMkValPredictionReduction::EVENT_TYPE) {
      auto reductionEvent = (ModelMkValPredictionReduction*)event;
      newItem = new PredictionItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the cause.
      auto causeItem = scene->getAeraGraphicsItem(reductionEvent->getCause());
      if (causeItem)
        scene->addArrow(causeItem, newItem);
      visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelImdlPredictionEvent::EVENT_TYPE) {
      auto reductionEvent = (ModelImdlPredictionEvent*)event;
      newItem = new ImdlPredictionItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the cause.
      auto causeItem = scene->getAeraGraphicsItem(reductionEvent->cause_);
      if (causeItem)
        scene->addArrow(causeItem, newItem);

      visible = ((nonSimulationsCheckBox_->checkState() == Qt::Checked) &&
        (requirementsCheckBox_->checkState() == Qt::Checked));
    }
    else if (event->eventType_ == ModelGoalReduction::EVENT_TYPE) {
      auto reductionEvent = (ModelGoalReduction*)event;
      newItem = new ModelGoalItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the fact super goal.
      auto factSuperGoalItem = scene->getAeraGraphicsItem(reductionEvent->factSuperGoal_);
      if (factSuperGoalItem)
        // The output of the abduction is the LHS.
        scene->addArrow(factSuperGoalItem, newItem, newItem);

      scene->addHorizontalLine(newItem);

      // Show all ModelGoalReduction events along with simulations, even if not simulated.
      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == CompositeStateGoalReduction::EVENT_TYPE) {
      auto reductionEvent = (CompositeStateGoalReduction*)event;
      newItem = new CompositeStateGoalItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the fact super goal.
      auto factSuperGoalItem = scene->getAeraGraphicsItem(reductionEvent->factSuperGoal_);
      if (factSuperGoalItem)
        // The output of the abduction is the LHS.
        scene->addArrow(factSuperGoalItem, newItem, newItem);

      scene->addHorizontalLine(newItem);

      // Show all CompositeStateGoalReduction events along with simulations, even if not simulated.
      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelSimulatedPredictionReduction::EVENT_TYPE) {
      auto reductionEvent = (ModelSimulatedPredictionReduction*)event;
      newItem = new ModelPredictionItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(reductionEvent->input_);
      if (inputItem)
        // If the input is a super goal, then the inputItem is the RHS, otherwose,
        // the input of the prediction is the LHS.
        scene->addArrow(inputItem, newItem, reductionEvent->inputIsSuperGoal_ ? newItem : inputItem);

      scene->addHorizontalLine(newItem);

      if (newItem->is_sim())
        visible = (simulationsCheckBox_->checkState() == Qt::Checked);
      else
        visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelSimulatedPredictionReductionFromRequirement::EVENT_TYPE) {
      auto reductionEvent = (ModelSimulatedPredictionReductionFromRequirement*)event;
      newItem = new ModelPredictionFromRequirementItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(reductionEvent->input_);
      if (inputItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(inputItem, newItem);

      scene->addHorizontalLine(newItem);

      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == CompositeStateSimulatedPredictionReduction::EVENT_TYPE) {
      auto reductionEvent = (CompositeStateSimulatedPredictionReduction*)event;
      newItem = new CompositeStatePredictionItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(reductionEvent->input_);

      // Add arrows to the inputs.
      for (int i = 0; i < reductionEvent->inputs_.size(); ++i) {
        auto referencedItem = scene->getAeraGraphicsItem(reductionEvent->inputs_[i]);
        if (!referencedItem)
          continue;
        if (referencedItem == inputItem)
          // The inputItem of the prediction is the LHS.
          scene->addArrow(referencedItem, newItem, inputItem);
        else
          scene->addArrow(referencedItem, newItem);
      }

      scene->addHorizontalLine(newItem);

      if (newItem->is_sim())
        visible = (simulationsCheckBox_->checkState() == Qt::Checked);
      else
        visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == PredictionResultEvent::EVENT_TYPE) {
      newItem = new PredictionResultItem((PredictionResultEvent*)event, replicodeObjects_, scene);
      visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE) {
      auto newIcstEvent = (NewInstantiatedCompositeStateEvent*)event;
      newItem = new InstantiatedCompositeStateItem(newIcstEvent, replicodeObjects_, scene);

      // Add arrows to inputs.
      for (int i = 0; i < newIcstEvent->inputs_.size(); ++i) {
        auto referencedItem = scene->getAeraGraphicsItem(newIcstEvent->inputs_[i]);
        if (referencedItem)
          scene->addArrow(referencedItem, newItem);
      }

      visible = ((nonSimulationsCheckBox_->checkState() == Qt::Checked) && 
                 (instantiatedCompositeStatesCheckBox_->checkState() == Qt::Checked));
    }
    else if (event->eventType_ == IoDeviceInjectEvent::EVENT_TYPE ||
             event->eventType_ == IoDeviceEjectEvent::EVENT_TYPE)
      // TODO: Position the IoDeviceInjectEvent at its injectionTime?
      newItem = new IoDeviceInjectEjectItem(event, replicodeObjects_, scene);
    else if (event->eventType_ == DriveInjectEvent::EVENT_TYPE) {
      auto driveInject = (DriveInjectEvent*)event;
      newItem = new DriveItem(driveInject, replicodeObjects_, scene);

      scene->addHorizontalLine(newItem);
      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == SimulationCommitEvent::EVENT_TYPE) {
      auto commitEvent = (SimulationCommitEvent*)event;
      newItem = new SimulationCommitItem(commitEvent, replicodeObjects_, scene);

      // Add an arrow to the input Success.
      auto factPredFactSuccessItem = scene->getAeraGraphicsItem(commitEvent->factPredFactSuccess_);
      if (factPredFactSuccessItem)
        // This is not a prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(factPredFactSuccessItem, newItem);

      scene->addHorizontalLine(newItem);

      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }

    // Add the new item.
    scene->addAeraGraphicsItem(newItem);

    // Add arrows to all referenced objects.
    for (int i = 0; i < event->object_->references_size(); ++i) {
      auto referencedItem = scene->getAeraGraphicsItem(event->object_->get_reference(i));
      if (referencedItem)
        scene->addArrow(referencedItem, newItem);
    }
    if (event->object_->code(0).asOpcode() == Opcodes::Fact ||
        event->object_->code(0).asOpcode() == Opcodes::AntiFact) {
      // Add references from the fact value.
      auto value = event->object_->get_reference(0);
      if (!(value->code(0).asOpcode() == Opcodes::IMdl || value->code(0).asOpcode() == Opcodes::ICst)) {
        for (int i = 0; i < value->references_size(); ++i) {
          auto referencedItem = scene->getAeraGraphicsItem(value->get_reference(i));
          if (referencedItem)
            scene->addArrow(referencedItem, newItem);
        }
      }
    }

    // Call setItemAndArrowsAndHorizontalLinesVisible, even if visible is true because we need to hide arrows to non-visible items.
    newItem->setItemAndArrowsAndHorizontalLinesVisible(visible);

    if (visible)
      // Only flash if visible.
      scene->establishFlashTimer();
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE) {
    auto setSuccessRateEvent = (SetModelEvidenceCountAndSuccessRateEvent*)event;

    // Save the current values for a later undo.
    setSuccessRateEvent->oldEvidenceCount_ = setSuccessRateEvent->object_->code(MDL_CNT).asFloat();
    setSuccessRateEvent->oldSuccessRate_ = setSuccessRateEvent->object_->code(MDL_SR).asFloat();

    // Update the model.
    setSuccessRateEvent->object_->code(MDL_CNT) = Atom::Float(setSuccessRateEvent->evidenceCount_);
    setSuccessRateEvent->object_->code(MDL_SR) = Atom::Float(setSuccessRateEvent->successRate_);

    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(setSuccessRateEvent->object_));
    if (modelItem) {
      modelItem->updateFromModel();
      if (setSuccessRateEvent->evidenceCount_ != setSuccessRateEvent->oldEvidenceCount_ &&
          setSuccessRateEvent->successRate_ == setSuccessRateEvent->oldSuccessRate_)
        // Only the evidence count changed.
        modelItem->evidenceCountFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      else if (setSuccessRateEvent->evidenceCount_ == setSuccessRateEvent->oldEvidenceCount_ &&
        setSuccessRateEvent->successRate_ != setSuccessRateEvent->oldSuccessRate_)
        // Only the success rate changed.
        modelItem->successRateFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      else {
        modelItem->evidenceCountFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        modelItem->successRateFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      }
      modelsScene_->establishFlashTimer();
    }
  }
  else if (event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    auto deleteModelEvent = (DeleteModelEvent*)event;

    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(deleteModelEvent->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::gray);
  }
  else {
    // Skip this event.
    ++iNextEvent_;
    return stepEvent(maximumTime);
  }

  ++iNextEvent_;

  return event->time_;
}

Timestamp AeraVisulizerWindow::unstepEvent(Timestamp minimumTime)
{
  if (iNextEvent_ == 0)
    // Return the value meaning no change.
    return Utils_MaxTime;

  if (events_[iNextEvent_ - 1]->time_ < minimumTime)
    // Don't decrement iNextEvent_.
    return Utils_MaxTime;

  --iNextEvent_;

  AeraEvent* event = events_[iNextEvent_].get();
  if (newItemEventTypes_.find(event->eventType_) != newItemEventTypes_.end()) {
    AeraVisualizerScene* scene;
    if (event->eventType_ == NewModelEvent::EVENT_TYPE ||
      event->eventType_ == NewCompositeStateEvent::EVENT_TYPE)
      scene = modelsScene_;
    else
      scene = mainScene_;

    // Find the AeraGraphicsItem for this event and remove it.
    // Note that the event saves the updated item position and will use it when recreating the item.
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(scene->getAeraGraphicsItem(event->object_));
    if (aeraGraphicsItem) {
      aeraGraphicsItem->removeArrowsAndHorizontalLines();
      scene->removeItem(aeraGraphicsItem);
      delete aeraGraphicsItem;
    }
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set to the old evidence count and success rate.
    auto setSuccessRateEvent = (SetModelEvidenceCountAndSuccessRateEvent*)event;

    setSuccessRateEvent->object_->code(MDL_CNT) = Atom::Float(setSuccessRateEvent->oldEvidenceCount_);
    setSuccessRateEvent->object_->code(MDL_SR) = Atom::Float(setSuccessRateEvent->oldSuccessRate_);

    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(setSuccessRateEvent->object_));
    if (modelItem) {
      if (setSuccessRateEvent->evidenceCount_ != setSuccessRateEvent->oldEvidenceCount_ &&
          setSuccessRateEvent->successRate_ == setSuccessRateEvent->oldSuccessRate_)
        // Only the evidence count changed.
        modelItem->evidenceCountFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      else if (setSuccessRateEvent->evidenceCount_ == setSuccessRateEvent->oldEvidenceCount_ &&
               setSuccessRateEvent->successRate_ != setSuccessRateEvent->oldSuccessRate_)
        // Only the success rate changed.
        modelItem->successRateFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      else {
        modelItem->evidenceCountFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        modelItem->successRateFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      }

      modelItem->updateFromModel();
      modelsScene_->establishFlashTimer();
    }
  }
  else if (event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not deleted.
    auto deleteModelEvent = (DeleteModelEvent*)event;

    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(deleteModelEvent->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::white);
  }
  else
    // Skip this event.
    return unstepEvent(minimumTime);

  if (iNextEvent_ > 0)
    return events_[iNextEvent_ - 1]->time_;
  else
    // The caller will use the time reference.
    return Timestamp(seconds(0));
}

void AeraVisulizerWindow::startPlay()
{
  if (isPlaying_)
    // Already playing.
    return;

  playPauseButton_->setIcon(pauseIcon_);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playPauseButton_->setIcon(pauseIcon_);
  isPlaying_ = true;
  if (playTimerId_ == 0)
    playTimerId_ = startTimer(AeraVisulizer_playTimerTick.count());
}

void AeraVisulizerWindow::stopPlay()
{
  if (playTimerId_ != 0) {
    killTimer(playTimerId_);
    playTimerId_ = 0;
  }

  playPauseButton_->setIcon(playIcon_);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playPauseButton_->setIcon(playIcon_);
  isPlaying_ = false;
}

void AeraVisulizerWindow::setPlayTime(Timestamp time)
{
  playTime_ = time;

  uint64 total_us;
  if (showRelativeTime_)
    total_us = duration_cast<microseconds>(time - replicodeObjects_.getTimeReference()).count();
  else
    total_us = duration_cast<microseconds>(time.time_since_epoch()).count();
  uint64 us = total_us % 1000;
  uint64 ms = total_us / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  char buffer[100];
  if (showRelativeTime_)
    sprintf(buffer, "%03ds:%03dms:%03dus", (int)s, (int)ms, (int)us);
  else {
    // Get the UTC time.
    time_t gmtTime = s;
    struct tm* t = gmtime(&gmtTime);
    sprintf(buffer, "%04d-%02d-%02d   UTC\n%02d:%02d:%02d:%03d:%03d",
      t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
      t->tm_hour, t->tm_min, t->tm_sec, (int)ms, (int)us);
  }
  playTimeLabel_->setText(buffer);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playTimeLabel_->setText(buffer);
}

void AeraVisulizerWindow::setSliderToPlayTime()
{
  if (events_.size() == 0) {
    playSlider_->setValue(0);
    for (size_t i = 0; i < children_.size(); ++i)
      children_[i]->playSlider_->setValue(0);
    return;
  }

  auto maximumEventTime = events_.back()->time_;
  int value = playSlider_->maximum() * 
    ((double)duration_cast<microseconds>(playTime_ - replicodeObjects_.getTimeReference()).count() /
     duration_cast<microseconds>(maximumEventTime - replicodeObjects_.getTimeReference()).count());
  playSlider_->setValue(value);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playSlider_->setValue(value);
}

void AeraVisulizerWindow::playPauseButtonClickedImpl()
{
  if (isPlaying_)
    stopPlay();
  else
    startPlay();
}

void AeraVisulizerWindow::stepButtonClickedImpl()
{
  stopPlay();
  size_t iNextStepEvent;
  if (getINextStepEvent(Utils_MaxTime, iNextEvent_, iNextStepEvent) == Utils_MaxTime)
    return;
  auto eventTime = events_[iNextStepEvent]->time_;

  // Keep stepping remaining events in this same frame.
  auto relativeTime = duration_cast<microseconds>(eventTime - replicodeObjects_.getTimeReference());
  auto frameStartTime = eventTime - (relativeTime % replicodeObjects_.getSamplingPeriod());
  auto thisFrameMaxTime = frameStartTime + replicodeObjects_.getSamplingPeriod() - microseconds(1);
  bool isNewFrame = (iNextStepEvent <= 0 || frameStartTime > events_[iNextStepEvent - 1]->time_);
  auto firstEventTime = eventTime;
  bool firstEventIsSimulation = 
    (simulationEventTypes_.find(events_[iNextStepEvent]->eventType_) != simulationEventTypes_.end());

  int iNonSimulation = -1;
  if (isNewFrame) {
    // Remove the simulation items from the previous frame.
    // TODO: We don't expect it, but if the first event in the frame is simulated then this will erase it.
    mainScene_->removeAllItemsByEventType(simulationEventTypes_);
    mainScene_->setFocusSimulationDetailOids(set<int>());
  }
  else {
    if (firstEventIsSimulation) {
      // Not a new frame and the first event is a simulation, so we want to step all the simulations at once.
      // Set iNonSimulation to the next non-simulation event. Also set iCommand to the simulation event showing a
      // non-simulated goal for a command (presumably the simulation's committed goal).
      // TODO: What about multiple committed goals including for mandatory solutions?
      int iCommand = -1;
      for (iNonSimulation = iNextStepEvent; iNonSimulation < events_.size(); ++iNonSimulation) {
        if (simulationEventTypes_.find(events_[iNonSimulation]->eventType_) == simulationEventTypes_.end())
          break;

        if (events_[iNonSimulation]->eventType_ == ModelGoalReduction::EVENT_TYPE) {
          auto value = ((ModelGoalReduction*)events_[iNonSimulation].get())->factGoal_->get_goal()->get_target()->get_reference(0);
          if (value->code(0).asOpcode() == Opcodes::Cmd)
            iCommand = iNonSimulation;
        }
      }

      if (iCommand >= 0) {
        // Start from the committed command and get the chain of inputs.
        std::set<int> focusSimulationDetailOids;
        int i = iCommand;
        while (i >= iNextStepEvent) {
          focusSimulationDetailOids.insert(events_[i]->object_->get_detail_oid());

          auto input = events_[i]->getInput();
          if (!input)
            // The end of the backward links, presumably the drive.
            break;

          // Keep searching backwards (back to the first simulation event) for the event of the input.
          --i;
          for (; i >= iNextStepEvent; --i) {
            if (events_[i]->object_ == input)
              break;
          }
        }

        // This will display the focus simulation items at the top.
        mainScene_->setFocusSimulationDetailOids(focusSimulationDetailOids);
      }
    }
  }

  while (true) {
    if (stepEvent(thisFrameMaxTime) == Utils_MaxTime)
      break;
    eventTime = events_[iNextEvent_ - 1]->time_;

    if (simulationsCheckBox_->isChecked()) {
      if (isNewFrame) {
        // In a new frame, advance until the next item would be a simulation item that is not at the first event time.
        if (iNextEvent_ < events_.size() &&
            simulationEventTypes_.find(events_[iNextEvent_]->eventType_) != simulationEventTypes_.end() &&
            events_[iNextEvent_]->time_ > firstEventTime)
          break;
      }
      else {
        // If not a new frame and the first event is a simulation, keep stepping until the next item would be a non-simulation.
        if (firstEventIsSimulation && iNextEvent_ >= iNonSimulation)
          break;
      }
    }
  }

  setPlayTime(eventTime);
  setSliderToPlayTime();
}

void AeraVisulizerWindow::stepBackButtonClickedImpl()
{
  stopPlay();
  auto newTime = max(unstepEvent(Timestamp(seconds(0))), replicodeObjects_.getTimeReference());
  if (newTime == Utils_MaxTime)
    return;
  // Debug: How to step the children also?

  // Keep unstepping remaining events in this same frame.
  auto relativeTime = duration_cast<microseconds>(newTime - replicodeObjects_.getTimeReference());
  auto frameStartTime = newTime - (relativeTime % replicodeObjects_.getSamplingPeriod());
  while (true) {
    auto localNewTime = unstepEvent(frameStartTime);
    if (localNewTime == Utils_MaxTime)
      break;
    newTime = localNewTime;
  }

  setPlayTime(max(newTime, replicodeObjects_.getTimeReference()));
  setSliderToPlayTime();
}

void AeraVisulizerWindow::playTimeLabelClickedImpl()
{
  showRelativeTime_ = !showRelativeTime_;
  setPlayTime(playTime_);
}

void AeraVisulizerWindow::timerEvent(QTimerEvent* event)
{
  // TODO: Make sure we don't re-enter.

  if (event->timerId() != playTimerId_)
    // This timer event is not for us.
    return;

  if (events_.size() == 0) {
    stopPlay();
    return;
  }

  auto maximumEventTime = events_.back()->time_;
  // TODO: Make this track the passage of real clock time.
  auto playTime = playTime_ + AeraVisulizer_playTimerTick;

  // Step events while events_[iNextEvent_] is less than or equal to the playTime.
  // Debug: How to step the children also?
  while (stepEvent(playTime) != Utils_MaxTime);

  if (iNextEvent_ >= events_.size()) {
    // We have played all events.
    playTime = maximumEventTime;
    stopPlay();
  }

  setPlayTime(playTime);
  setSliderToPlayTime();
}

void AeraVisulizerWindow::zoomIn()
{
  selectedScene_->scaleViewBy(1.09);
}

void AeraVisulizerWindow::zoomOut()
{
  selectedScene_->scaleViewBy(1 / 1.09);
}

void AeraVisulizerWindow::zoomHome()
{
  selectedScene_->zoomViewHome();
}

void AeraVisulizerWindow::createActions()
{
  exitAction_ = new QAction(tr("E&xit"), this);
  exitAction_->setShortcuts(QKeySequence::Quit);
  connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  zoomInAction_ = new QAction(QIcon(":/images/zoom-in.png"), tr("Zoom In"), this);
  zoomInAction_->setStatusTip(tr("Zoom In"));
  connect(zoomInAction_, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAction_ = new QAction(QIcon(":/images/zoom-out.png"), tr("Zoom Out"), this);
  zoomOutAction_->setStatusTip(tr("Zoom Out"));
  connect(zoomOutAction_, SIGNAL(triggered()), this, SLOT(zoomOut()));

  zoomHomeAction_ = new QAction(QIcon(":/images/zoom-home.png"), tr("Zoom Home"), this);
  zoomHomeAction_->setStatusTip(tr("Zoom to show all"));
  connect(zoomHomeAction_, SIGNAL(triggered()), this, SLOT(zoomHome()));
}

void AeraVisulizerWindow::createMenus()
{
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(exitAction_);

  QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAction_);
  viewMenu->addAction(zoomOutAction_);
  viewMenu->addAction(zoomHomeAction_);
}

void AeraVisulizerWindow::createToolbars()
{
  QToolBar* toolbar = addToolBar(tr("Main"));
  toolbar->addAction(zoomInAction_);
  toolbar->addAction(zoomOutAction_);
  toolbar->addAction(zoomHomeAction_);

  toolbar->addSeparator();
  toolbar->addWidget(new QLabel("Show/Hide: ", this));

  simulationsCheckBox_ = new QCheckBox("Simulations", this);
  simulationsCheckBox_->setStyleSheet("background-color:#ffffdc");
  // Show simulations by default.
  simulationsCheckBox_->setCheckState(Qt::Checked);
  connect(simulationsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    for (auto i = simulationEventTypes_.begin(); i != simulationEventTypes_.end(); ++i)
      mainScene_->setItemsVisible(*i, state == Qt::Checked);
    });
  toolbar->addWidget(simulationsCheckBox_);

  // Separate the non-simulations check boxes.
  toolbar->addWidget(new QLabel("    ", this));

  nonSimulationsCheckBox_ = new QCheckBox("Non-Simulations", this);
  nonSimulationsCheckBox_->setStyleSheet("background-color:#ffffff");
  // Show non-simulations by default.
  nonSimulationsCheckBox_->setCheckState(Qt::Checked);
  connect(nonSimulationsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    essenceFactsCheckBox_->setEnabled(state == Qt::Checked);
    instantiatedCompositeStatesCheckBox_->setEnabled(state == Qt::Checked);
    requirementsCheckBox_->setEnabled(state == Qt::Checked);

    // Do the opposite of simulationsCheckBox_ .
    mainScene_->setNonItemsVisible(simulationEventTypes_, state == Qt::Checked);
    if (state == Qt::Checked) {
      // Make specific non-simulation items not visible, if needed.
      mainScene_->setAutoFocusItemsVisible("essence", essenceFactsCheckBox_->checkState() == Qt::Checked);
      mainScene_->setItemsVisible(
        NewInstantiatedCompositeStateEvent::EVENT_TYPE, instantiatedCompositeStatesCheckBox_->checkState() == Qt::Checked);
      mainScene_->setItemsVisible(
        ModelImdlPredictionEvent::EVENT_TYPE, requirementsCheckBox_->checkState() == Qt::Checked);
    }
  });
  toolbar->addWidget(nonSimulationsCheckBox_);

  essenceFactsCheckBox_ = new QCheckBox("Essence Facts", this);
  essenceFactsCheckBox_->setStyleSheet("background-color:#ffffff");
  connect(essenceFactsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setAutoFocusItemsVisible("essence", state == Qt::Checked);  });
  toolbar->addWidget(essenceFactsCheckBox_);

  instantiatedCompositeStatesCheckBox_ = new QCheckBox("Instantiated Comp. States", this);
  instantiatedCompositeStatesCheckBox_->setStyleSheet("background-color:#ffffff");
  connect(instantiatedCompositeStatesCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewInstantiatedCompositeStateEvent::EVENT_TYPE, state == Qt::Checked);  });
  toolbar->addWidget(instantiatedCompositeStatesCheckBox_);

  requirementsCheckBox_ = new QCheckBox("Requirements", this);
  requirementsCheckBox_->setStyleSheet("background-color:#ffffff");
  connect(requirementsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(ModelImdlPredictionEvent::EVENT_TYPE, state == Qt::Checked);  });
  toolbar->addWidget(requirementsCheckBox_);
}

}
