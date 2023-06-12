//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2023 Jeff Thompson
//_/_/ Copyright (c) 2018-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2023 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
//_/_/ http://www.iiim.is
//_/_/
//_/_/ --- Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without
//_/_/ modification, is permitted provided that the following conditions
//_/_/ are met:
//_/_/ - Redistributions of source code must retain the above copyright
//_/_/   and collaboration notice, this list of conditions and the
//_/_/   following disclaimer.
//_/_/ - Redistributions in binary form must reproduce the above copyright
//_/_/   notice, this list of conditions and the following disclaimer 
//_/_/   in the documentation and/or other materials provided with 
//_/_/   the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its
//_/_/   contributors may be used to endorse or promote products
//_/_/   derived from this software without specific prior 
//_/_/   written permission.
//_/_/   
//_/_/ - CADIA Clause: The license granted in and to the software 
//_/_/   under this agreement is a limited-use license. 
//_/_/   The software may not be used in furtherance of:
//_/_/    (i)   intentionally causing bodily injury or severe emotional 
//_/_/          distress to any person;
//_/_/    (ii)  invading the personal privacy or violating the human 
//_/_/          rights of any person; or
//_/_/    (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
//_/_/ CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
//_/_/ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//_/_/ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//_/_/ DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//_/_/ CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//_/_/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//_/_/ SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//_/_/ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//_/_/ WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//_/_/ NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//_/_/ OF SUCH DAMAGE.
//_/_/ 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <fstream>
#include <algorithm>
#include "aera-checkbox.h"
#include "graphics-items/aba-sentence-item.hpp"
#include "graphics-items/aera-graphics-item-group.hpp"
#include "graphics-items/aera-visualizer-scene.hpp"
#include "graphics-items/arrow.hpp"
#include "graphics-items/auto-focus-fact-item.hpp"
#include "graphics-items/composite-state-goal-item.hpp"
#include "graphics-items/composite-state-item.hpp"
#include "graphics-items/composite-state-prediction-item.hpp"
#include "graphics-items/drive-item.hpp"
#include "graphics-items/instantiated-composite-state-item.hpp"
#include "graphics-items/instantiated-model-item.hpp"
#include "graphics-items/io-device-inject-eject-item.hpp"
#include "graphics-items/model-goal-item.hpp"
#include "graphics-items/model-imdl-prediction-item.hpp"
#include "graphics-items/model-item.hpp"
#include "graphics-items/model-prediction-from-requirement-item.hpp"
#include "graphics-items/model-prediction-from-requirement-disabled-item.hpp"
#include "graphics-items/model-prediction-item.hpp"
#include "graphics-items/predicted-instantiated-composite-state-item.hpp"
#include "graphics-items/prediction-item.hpp"
#include "graphics-items/prediction-result-item.hpp"
#include "graphics-items/promoted-prediction-defeated-item.hpp"
#include "graphics-items/promoted-prediction-item.hpp"
#include "graphics-items/simulation-commit-item.hpp"
#include "submodules/AERA/r_exec/opcodes.h"

#include "aera-visualizer-window.hpp"
#include "find-dialog.hpp"

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

const set<int> AeraVisualizerWindow::simulationEventTypes_ = {
  AbaAddSentence::EVENT_TYPE,
  AbaMarkSentence::EVENT_TYPE,
  AbaMarkedSentenceToParent::EVENT_TYPE,
  CompositeStateGoalReduction::EVENT_TYPE,
  CompositeStateSimulatedPredictionReduction::EVENT_TYPE,
  DriveInjectEvent::EVENT_TYPE,
  ModelGoalReduction::EVENT_TYPE,
  ModelPredictionFromRequirementDisabledEvent::EVENT_TYPE,
  ModelSimulatedPredictionReduction::EVENT_TYPE,
  ModelSimulatedPredictionReductionFromGoalRequirement::EVENT_TYPE,
  PromotedSimulatedPredictionDefeatEvent::EVENT_TYPE,
  PromotedSimulatedPredictionEvent::EVENT_TYPE,
  SimulationCommitEvent::EVENT_TYPE };

const set<int> AeraVisualizerWindow::newItemEventTypes_ = {
  AbaAddSentence::EVENT_TYPE,
  AutoFocusNewObjectEvent::EVENT_TYPE,
  CompositeStateGoalReduction::EVENT_TYPE,
  CompositeStateSimulatedPredictionReduction::EVENT_TYPE,
  DriveInjectEvent::EVENT_TYPE,
  IoDeviceEjectEvent::EVENT_TYPE,
  IoDeviceInjectEvent::EVENT_TYPE,
  ModelGoalReduction::EVENT_TYPE,
  ModelImdlPredictionEvent::EVENT_TYPE,
  ModelMkValPredictionReduction::EVENT_TYPE,
  ModelPredictionFromRequirementDisabledEvent::EVENT_TYPE,
  ModelSimulatedPredictionReduction::EVENT_TYPE,
  ModelSimulatedPredictionReductionFromGoalRequirement::EVENT_TYPE,
  NewCompositeStateEvent::EVENT_TYPE,
  NewInstantiatedCompositeStateEvent::EVENT_TYPE,
  NewModelEvent::EVENT_TYPE,
  NewInstantiatedModelEvent::EVENT_TYPE,
  NewPredictedInstantiatedCompositeStateEvent::EVENT_TYPE,
  PredictionResultEvent::EVENT_TYPE,
  PromotedSimulatedPredictionDefeatEvent::EVENT_TYPE,
  PromotedSimulatedPredictionEvent::EVENT_TYPE,
  SimulationCommitEvent::EVENT_TYPE };

const QString AeraVisualizerWindow::SettingsKeyAutoScroll = "AutoScroll";
const QString AeraVisualizerWindow::SettingsKeySimulationsVisible = "simulationsVisible";
const QString AeraVisualizerWindow::SettingsKeyAllSimulationInputsVisible = "allSimulationInputsVisible";
const QString AeraVisualizerWindow::SettingsKeySingleStepSimulationVisible = "singleStepSimulationVisible";
const QString AeraVisualizerWindow::SettingsKeyNonSimulationsVisible = "nonSimulationsVisible";
const QString AeraVisualizerWindow::SettingsKeyEssenceFactsVisible = "essenceFactsVisible";
const QString AeraVisualizerWindow::SettingsKeyInstantiatedCompositeStatesVisible = "instantiatedCompositeStatesVisible";
const QString AeraVisualizerWindow::SettingsKeyInstantiatedModelsVisible = "instantiatedModelsVisible";
const QString AeraVisualizerWindow::SettingsKeyPredictedInstantiatedCompositeStatesVisible = "predictedInstantiatedCompositeStatesVisible";
const QString AeraVisualizerWindow::SettingsKeyRequirementsVisible = "requirementsVisible";

AeraVisualizerWindow::AeraVisualizerWindow(ReplicodeObjects& replicodeObjects)
: AeraVisualizerWindowBase(0, replicodeObjects),
  iNextEvent_(0), explanationLogWindow_(0),
  essencePropertyObject_(replicodeObjects_.getObject("essence")),
  hoverHighlightItem_(0),
  phasedOutModelColor_(255, 192, 192),
  showRelativeTime_(true),
  playTime_(seconds(0)),
  playTimerId_(0),
  isPlaying_(false),
  itemBorderHighlightPen_(Qt::blue, 3)
{
  createActions();
  createMenus();

  // Set mainScene_ to null so that setPlayTime will not try to auto-scroll it.
  mainScene_ = 0;
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

bool AeraVisualizerWindow::addEvents(const string& runtimeOutputFilePath, QProgressDialog& progress)
{
  // load mdl 37, MDLController(113)
  regex loadModelRegex("^load mdl (\\d+), MDLController\\((\\d+)\\) strength:([\\d\\.]+) cnt:(\\d+) sr:([\\d\\.]+)$");
  // load cst 36, CSTController(98)
  regex loadCompositeStateRegex("^load cst (\\d+), CSTController\\((\\d+)\\)$");

  // The remaining regex expressions all start with a timestamp.
  regex timestampRegex("^(\\d+)s:(\\d+)ms:(\\d+)us (.+)$");

  // -> mdl 194 strength:0 cnt:1 sr:1, MDLController(314)
  regex newModelRegex("^-> mdl (\\d+) strength:([\\d\\.]+) cnt:(\\d+) sr:([\\d\\.]+), MDLController\\((\\d+)\\)$");
  // mdl 53 cnt:2 sr:1
  regex setEvidenceCountAndSuccessRateRegex("^mdl (\\d+) cnt:(\\d+) sr:([\\d\\.]+)$");
  // mdl 75 strength:1
  regex setStrengthRegex("^mdl (\\d+) strength:([\\d\\.]+)$");
  // mdl 53 deleted
  // mdl 53 phased in
  // mdl 53 phased out
  regex deleteOrPhaseInOrOutModelRegex("^mdl (\\d+) (deleted|phased in|phased out)$");
  // -> cst 52, CSTController(375)
  regex newCompositeStateRegex("^-> cst (\\d+), CSTController\\((\\d+)\\)$");
  // A/F -> 35|40 (AXIOM)
  regex autofocusNewObjectRegex("^A/F -> (\\d+)\\|(\\d+) \\((\\w+)\\)$");
  // mdl 61 predict imdl -> mk.rdx 559
  regex modelImdlPredictionReductionRegex("^mdl \\d+ predict imdl -> mk.rdx (\\d+)$");
  // mdl 67: fact (352225) pred fact imdl -> fact 588 simulated pred, from goal req 533
  regex modelSimulatedPredictionFromGoalRequirementRegex("^mdl (\\d+): fact \\((\\d+)\\) pred fact imdl -> fact (\\d+) simulated pred, from goal req (\\d+)$");
  // mdl 67: fact (697996) pred fact imdl, from goal req 1250, simulated pred disabled by fact (696754) pred |fact imdl
  regex modelPredictionDisabledByStrongRequirementRegex("^mdl (\\d+): fact \\((\\d+)\\) pred fact imdl(, from goal req (\\d+))?, (simulated )?pred disabled by fact \\((\\d+)\\) pred \\|fact imdl$");
  // mdl 63 predict -> mk.rdx 68
  regex modelPredictionReductionRegex("^mdl \\d+ predict -> mk.rdx (\\d+)$");
  // mdl 41 abduce -> mk.rdx 97
  regex modelAbductionReductionRegex("^mdl \\d+ abduce -> mk.rdx (\\d+)$");
  // mdl 64: fact 96 super_goal -> fact 98 simulated goal
  regex modelSimulatedAbductionRegex("^mdl (\\d+): fact (\\d+) super_goal -> fact (\\d+) simulated goal$");
  // cst 64: fact 96 super_goal -> fact 98 simulated goal
  regex compositeStateSimulatedAbductionRegex("^cst (\\d+): fact (\\d+) super_goal -> fact (\\d+) simulated goal$");
  // mdl 57: fact 202 pred -> fact 227 simulated pred
  // mdl 57: fact 202 pred -> fact 227 simulated pred, using req (745971)
  // mdl 57: fact 202 pred -> fact 227 simulated pred fact imdl, using req (745971)
  regex modelSimulatedPredictionRegex("^mdl (\\d+): fact (\\d+) pred -> fact (\\d+) simulated pred( fact imdl)?(?:, using req \\((\\d+)\\))?$");
  // mdl 63: fact 531 super_goal -> fact (332278) simulated pred start, using req (323845), ijt 0s:535ms:0us
  regex modelSimulatedPredictionStartRegex("^mdl (\\d+): fact (\\d+) super_goal -> fact \\((\\d+)\\) simulated pred start(?:, using req \\((\\d+)\\))?, ijt (\\d+)s:(\\d+)ms:(\\d+)us$");
  // cst 60: fact 195 -> fact 218 simulated pred fact icst [ 155 191]
  regex compositeStateSimulatedPredictionRegex("^cst (\\d+): fact (\\d+) -> fact (\\d+) simulated pred fact icst \\[([ \\d]+)\\]$");
  // fact 59 icst[52][ 50 55]
  regex newInstantiatedCompositeStateRegex("^fact (\\d+) icst\\[\\d+\\]\\[([ \\d]+)\\]$");
  // fact 59 pred fact (193775) icst[52][ 50 55]
  regex newPredictedInstantiatedCompositeStateRegex("^fact (\\d+) pred fact \\(\\d+\\) icst\\[\\d+\\]\\[([ \\d]+)\\]$");
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
  // fact 182 -> promoted simulated pred fact 250 w/ fact 247 timings
  regex simulationPromotedSimulatedPredictionRegex("^fact (\\d+) -> promoted simulated pred fact (\\d+) w/ fact (\\d+) timings$");
  // promoted simulated fact 251 with DefeasibleValidity(200773) defeated by fact 253
  regex simulationPromotedSimulatedPredictionDefeatedRegex("^promoted simulated fact (\\d+) with DefeasibleValidity\\((\\d+)\\) defeated by fact (\\d+)");
  // Step 0: Case init: S: 304
  regex abaCaseInitStepRegex("^Step (\\d+): Case init: S: (\\d+)$");
  // Step 10: Case 1.(i): A: 314, Contrary 322, NewGId 1
  // TODO: Handle when the Contrary already exists.
  regex abaCase1iStepRegex("^Step (\\d+): Case 1\\.\\(i\\): A: (\\d+), Contrary (\\d+), NewGId (\\d+)");
  // Step 10: Case 1.(ii): S: 304, NewUnMarkedAs: [314 316], NewUnMarkedNonAs: [312], ExistingBody: [310]
  regex abaCase1iiStepRegex("^Step (\\d+): Case 1\\.\\(ii\\): S: (\\d+), NewUnMarkedAs: \\[(.*)\\], NewUnMarkedNonAs: \\[(.*)\\], ExistingBody: \\[(.*)\\]$");
  // Step 10: Case 2.(ia): A: 904, GId 1
  regex abaCase2iaStepRegex("^Step (\\d+): Case 2\\.\\(ia\\): A: (\\d+), GId (\\d+)$");
  // Step 10: Case 2.(ib): A: 904, GId 1, Culprit 864
  regex abaCase2ibStepRegex("^Step (\\d+): Case 2\\.\\(ib\\): A: (\\d+), GId (\\d+), Culprit (\\d+)");
  // Step 10: Case 2.(ic): A: 324, GId 1, Contrary 326 new? Y
  regex abaCase2icStepRegex("^Step (\\d+): Case 2\\.\\(ic\\): A: (\\d+), GId (\\d+), Contrary (\\d+) new\\? (\\w)$");
  // Step 10: Case 2.(ii): S: 322, GId 1, mark graph? N
  regex abaCase2iiMarkStepRegex("^Step (\\d+): Case 2\\.\\(ii\\): S: (\\d+), GId (\\d+), mark graph\\? (\\w)$");
  // Step 10: Case 2.(ii): S: 322, NewGId 1, NewUnMarkedAs: [324], NewUnMarkedNonAs: [312], ExistingBody: [310]
  regex abaCase2iiStepRegex("^Step (\\d+): Case 2\\.\\(ii\\): S: (\\d+), NewGId (\\d+), NewUnMarkedAs: \\[(.*)\\], NewUnMarkedNonAs: \\[(.*)\\], ExistingBody: \\[(.*)\\]$");

  progress.setLabelText(replicodeObjects_.getProgressLabelText("Reading runtime output"));

  // Count the number of lines, to use in the progress dialog.
  int nLines;
  {
    ifstream fileForCount(runtimeOutputFilePath);
    nLines = std::count(istreambuf_iterator<char>(fileForCount), istreambuf_iterator<char>(), '\n');
  }
  progress.setMaximum(nLines);

  // pendingEvents is an ordered map keyed by event time. The value is a list of pending events at the time.
  map<core::Timestamp, vector<shared_ptr<AeraEvent> > > pendingEvents;

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
        // Restore the initial count, success rate and strength.
        core::float32 strength = stof(matches[3].str());
        core::float32 evidenceCount = stol(matches[4].str());
        core::float32 successRate = stof(matches[5].str());
        model->code(MDL_STRENGTH) = Atom::Float(strength);
        model->code(MDL_CNT) = Atom::Float(evidenceCount);
        model->code(MDL_SR) = Atom::Float(successRate);
        startupEvents_.push_back(make_shared <NewModelEvent>(
          replicodeObjects_.getTimeReference(), model, strength, evidenceCount, successRate, stoll(matches[2].str())));
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

    while (pendingEvents.size() >= 1 && pendingEvents.begin()->first <= timestamp) {
      // Insert the pending event before this new event.
      for (int i = 0; i < pendingEvents.begin()->second.size(); ++i)
        events_.push_back(pendingEvents.begin()->second[i]);
      pendingEvents.erase(pendingEvents.begin());
    }

    if (regex_search(lineAfterTimestamp, matches, newModelRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      core::float32 strength = stof(matches[2].str());
      core::float32 evidenceCount = stol(matches[3].str());
      core::float32 successRate = stof(matches[4].str());
      if (model)
        events_.push_back(make_shared<NewModelEvent>(
          timestamp, model, strength, evidenceCount, successRate, stoll(matches[2].str())));
    }
    else if (regex_search(lineAfterTimestamp, matches, setEvidenceCountAndSuccessRateRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model)
        events_.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(
          timestamp, model, stol(matches[2].str()), stof(matches[3].str())));
    }
    else if (regex_search(lineAfterTimestamp, matches, setStrengthRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model)
        events_.push_back(make_shared<SetModelStrengthEvent>(
          timestamp, model, stof(matches[2].str())));
    }
    else if (regex_search(lineAfterTimestamp, matches, deleteOrPhaseInOrOutModelRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (model) {
        if (matches[2] == "phased in")
          events_.push_back(make_shared<PhaseInModelEvent>(timestamp, model));
        if (matches[2] == "phased out")
          events_.push_back(make_shared<PhaseOutModelEvent>(timestamp, model));
        else if (matches[2] == "deleted")
          events_.push_back(make_shared<DeleteModelEvent>(timestamp, model));
      }
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
      if (fromObject && toObject)
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
              timestamp, model, factPred, cause, (Code*)NULL, false, false));
          else
            events_.push_back(make_shared<ModelImdlPredictionEvent>(
              timestamp, factPred, model, cause));
        }
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, modelSimulatedPredictionFromGoalRequirementRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto input = replicodeObjects_.getObjectByDetailOid(stoul(matches[2].str()));
      auto goal_requirement = replicodeObjects_.getObject(stoul(matches[4].str()));

      if (model && factPred && input && goal_requirement)
        events_.push_back(make_shared<ModelSimulatedPredictionReductionFromGoalRequirement>(
          timestamp, model, factPred, input, goal_requirement));
    }
    else if (regex_search(lineAfterTimestamp, matches, modelPredictionDisabledByStrongRequirementRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto input = replicodeObjects_.getObjectByDetailOid(stoul(matches[2].str()));
      Code* goal_requirement = 0;
      if (matches[4].length() > 0)
        goal_requirement = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto strong_requirement = replicodeObjects_.getObjectByDetailOid(stoul(matches[6].str()));

      if (model && input && strong_requirement)
        events_.push_back(make_shared<ModelPredictionFromRequirementDisabledEvent>(
          timestamp, model, input, goal_requirement, strong_requirement));
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
          events_.push_back(make_shared<NewInstantiatedModelEvent>(
            timestamp, reduction, factPred));
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
      auto input = replicodeObjects_.getObject(stoul(matches[2].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[3].str()));
      bool factPredIsImdl = (matches[4] == " fact imdl");
      Code* requirement = 0;
      if (matches[5].length() > 0)
        requirement = replicodeObjects_.getObjectByDetailOid(stoul(matches[5].str()));

      if (model && factPred && input)
        events_.push_back(make_shared<ModelSimulatedPredictionReduction>(
          timestamp, model, factPred, input, requirement, false, factPredIsImdl));
    }
    else if (regex_search(lineAfterTimestamp, matches, modelSimulatedPredictionStartRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[2].str()));
      auto factPred = replicodeObjects_.getObjectByDetailOid(stoul(matches[3].str()));
      Code* requirement = 0;
      if (matches[4].length() > 0)
        requirement = replicodeObjects_.getObjectByDetailOid(stoul(matches[4].str()));

      if (model && factPred && input) {
        core::Timestamp injectionTime = getTimestamp(matches, 5);
        if (injectionTime < timestamp)
          // We don't expect this, but the runtime would not have injected earlier.
          injectionTime = timestamp;
        // TODO: Use an AeraEvent with the details of starting the simulated forward chaining, and include requirement.
        auto event = make_shared<ModelSimulatedPredictionReduction>(injectionTime, model, factPred, input, requirement, true, false);
        // Put in pendingEvents to be added to events_ later.
        if (pendingEvents.find(event->time_) == pendingEvents.end())
          // Create the entry.
          pendingEvents[event->time_] = vector<shared_ptr<AeraEvent> >();
        pendingEvents[event->time_].push_back(event);
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, compositeStateSimulatedPredictionRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[2].str()));

      // Get the matching inputs.
      vector<Code*> inputs;
      bool gotAllInputs = replicodeObjects_.getObjects(matches[4].str(), inputs);

      if (compositeState && factPred && input && gotAllInputs)
        events_.push_back(make_shared<CompositeStateSimulatedPredictionReduction>(
          timestamp, compositeState, factPred, input, inputs));
    }
    else if (regex_search(lineAfterTimestamp, matches, newInstantiatedCompositeStateRegex)) {
      auto instantiatedCompositeState = replicodeObjects_.getObject(stoul(matches[1].str()));

      // Get the matching inputs.
      vector<Code*> inputs;
      bool gotAllInputs = replicodeObjects_.getObjects(matches[2].str(), inputs);;

      if (instantiatedCompositeState && gotAllInputs)
        events_.push_back(make_shared<NewInstantiatedCompositeStateEvent>(
          timestamp, instantiatedCompositeState, inputs));
    }
    else if (regex_search(lineAfterTimestamp, matches, newPredictedInstantiatedCompositeStateRegex)) {
      auto f_p_f_icst = replicodeObjects_.getObject(stoul(matches[1].str()));

      // Get the matching inputs.
      string inputOids = matches[2].str();
      vector<Code*> inputs;
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

      if (f_p_f_icst && gotAllInputs)
        events_.push_back(make_shared<NewPredictedInstantiatedCompositeStateEvent>(
          timestamp, f_p_f_icst, inputs));
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
    else if (regex_search(lineAfterTimestamp, matches, simulationPromotedSimulatedPredictionRegex)) {
      auto promotedFact = replicodeObjects_.getObject(stoul(matches[2].str()));
      auto promotedFromFact = replicodeObjects_.getObject(stoul(matches[1].str()));
      auto timingsFact = replicodeObjects_.getObject(stoul(matches[3].str()));
      if (promotedFact && promotedFromFact && timingsFact)
        events_.push_back(make_shared<PromotedSimulatedPredictionEvent>(
          timestamp, promotedFact, promotedFromFact,timingsFact));
    }
    else if (regex_search(lineAfterTimestamp, matches, simulationPromotedSimulatedPredictionDefeatedRegex)) {
      auto input = replicodeObjects_.getObject(stoul(matches[3].str()));
      auto promotedFact = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (input && promotedFact)
        events_.push_back(make_shared<PromotedSimulatedPredictionDefeatEvent>(
          timestamp, input, promotedFact));
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCaseInitStepRegex)) {
      auto fact = replicodeObjects_.getObject(stoul(matches[2].str()));
      if (fact) {
        abaNewStep(stoul(matches[1].str()));
        events_.push_back(make_shared<AbaAddSentence>(timestamp, fact, false, true, 0, (Code*)NULL, "init"));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase1iStepRegex)) {
      auto assumption = replicodeObjects_.getObject(stoul(matches[2].str()));
      auto contrary = replicodeObjects_.getObject(stoul(matches[3].str()));
      int newGId = stoul(matches[4].str());

      if (assumption && newGId > 0 && contrary) {
        abaNewStep(stoul(matches[1].str()));
        // This step sets the assumption to marked.
        events_.push_back(make_shared<AbaMarkSentence>(timestamp, assumption));
        // TODO: If newGId == 0 then find the contrary in an existing group.
        if (newGId > 0)
          events_.push_back(make_shared<AbaAddSentence>(
            timestamp, contrary, false, true, newGId, assumption, "1.(i)"));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase1iiStepRegex)) {
      auto head = replicodeObjects_.getObject(stoul(matches[2].str()));
      vector<Code*> newUnmarkedAssumptions;
      vector<Code*> newUnmarkedNonAssumptions;
      vector<Code*> existingBody;

      if (head &&
          replicodeObjects_.getObjects(matches[3].str(), newUnmarkedAssumptions) &&
          replicodeObjects_.getObjects(matches[4].str(), newUnmarkedNonAssumptions) &&
          replicodeObjects_.getObjects(matches[5].str(), existingBody)) {
        abaNewStep(stoul(matches[1].str()));
        // This step sets the head to marked.
        events_.push_back(make_shared<AbaMarkSentence>(timestamp, head));

        for (auto fact = existingBody.begin(); fact != existingBody.end(); ++fact)
          events_.push_back(make_shared<AbaMarkedSentenceToParent>(timestamp, *fact, head));
        for (auto fact = newUnmarkedAssumptions.begin(); fact != newUnmarkedAssumptions.end(); ++fact)
          events_.push_back(make_shared<AbaAddSentence>(timestamp, *fact, true, false, 0, head, "1.(ii)"));
        for (auto fact = newUnmarkedNonAssumptions.begin(); fact != newUnmarkedNonAssumptions.end(); ++fact)
          events_.push_back(make_shared<AbaAddSentence>(timestamp, *fact, false, false, 0, head, "1.(ii)"));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2iaStepRegex)) {
      auto fact = replicodeObjects_.getObject(stoul(matches[2].str()));

      if (fact) {
        abaNewStep(stoul(matches[1].str()));
        // (Don't mark the graph.)
        events_.push_back(make_shared<AbaMarkSentence>(timestamp, fact, false));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2ibStepRegex)) {
      auto fact = replicodeObjects_.getObject(stoul(matches[2].str()));
      auto culprit = replicodeObjects_.getObject(stoul(matches[4].str()));

      if (fact) {
        abaNewStep(stoul(matches[1].str()));
        // (Also mark the graph that the fact is in.)
        events_.push_back(make_shared<AbaMarkSentence>(timestamp, fact, true));

        if (culprit)
          // The fact is the same as the culprit in a different graph.
          events_.push_back(make_shared<AbaMarkedSentenceToParent>(timestamp, fact, culprit));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2icStepRegex)) {
      auto fact = replicodeObjects_.getObject(stoul(matches[2].str()));
      auto contrary = replicodeObjects_.getObject(stoul(matches[4].str()));
      bool contraryIsNew = (matches[5].str() == "Y");

      if (fact && contrary) {
        abaNewStep(stoul(matches[1].str()));
        // (Also mark the graph that the fact is in.)
        events_.push_back(make_shared<AbaMarkSentence>(timestamp, fact, true));
        if (contraryIsNew)
          events_.push_back(make_shared<AbaAddSentence>(
            timestamp, contrary, false, false, 0, fact, "2.(ic)"));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2iiMarkStepRegex)) {
      auto head = replicodeObjects_.getObject(stoul(matches[2].str()));
      bool markGraph = (matches[3].str() == "Y");

      if (head) {
        abaNewStep(stoul(matches[1].str()));
        // This step sets the head to marked. Further actions are in abaCase2iiStepRegex.
        events_.push_back(make_shared<AbaMarkSentence>(timestamp, head, markGraph));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2iiStepRegex)) {
      auto head = replicodeObjects_.getObject(stoul(matches[2].str()));
      int newGraphId = stoul(matches[3].str());
      vector<Code*> newUnmarkedAssumptions;
      vector<Code*> newUnmarkedNonAssumptions;
      vector<Code*> existingBody;

      if (head &&
          replicodeObjects_.getObjects(matches[4].str(), newUnmarkedAssumptions) &&
          replicodeObjects_.getObjects(matches[5].str(), newUnmarkedNonAssumptions) &&
          replicodeObjects_.getObjects(matches[6].str(), existingBody)) {
        // We have already set the head to marked with abaCase2iiMarkStepRegex. Don't call abaNewStep or add AbaMarkSentence.

        for (auto fact = existingBody.begin(); fact != existingBody.end(); ++fact)
          events_.push_back(make_shared<AbaMarkedSentenceToParent>(timestamp, *fact, head));
        for (auto fact = newUnmarkedAssumptions.begin(); fact != newUnmarkedAssumptions.end(); ++fact)
          events_.push_back(make_shared<AbaAddSentence>(timestamp, *fact, true, false, newGraphId, head, "2.(ii)"));
        for (auto fact = newUnmarkedNonAssumptions.begin(); fact != newUnmarkedNonAssumptions.end(); ++fact)
          events_.push_back(make_shared<AbaAddSentence>(timestamp, *fact, false, false, newGraphId, head, "2.(ii)"));
      }
    }
  }

  // Transfer any remaining pendingEvents to events_.
  for (auto event = pendingEvents.begin(); event != pendingEvents.end(); ++event) {
    for (int i = 0; i < event->second.size(); ++i)
      events_.push_back(event->second[i]);
  }
  pendingEvents.clear();

  return true;
}

void AeraVisualizerWindow::abaNewStep(int step)
{
  if (step < abaStepIndexes_.size()) {
    // There are already events for this step. Erase them.
    size_t eventIndex = abaStepIndexes_[step];
    abaStepIndexes_.erase(abaStepIndexes_.begin() + step, abaStepIndexes_.end());
    if (eventIndex < events_.size())
      // TODO: What if there are non-ABA events?
      events_.erase(events_.begin() + eventIndex, events_.end());
  }

  // Set abaStepIndexes_[step] to the next index in events_.
  // This loop should only iterate once, but step may have skipped a step.
  while (abaStepIndexes_.size() <= step)
    abaStepIndexes_.push_back(events_.size());
}

void AeraVisualizerWindow::addStartupItems()
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

Timestamp AeraVisualizerWindow::getTimestamp(const smatch& matches, int index)
{
  microseconds us(1000000 * stoll(matches[index].str()) +
                     1000 * stoll(matches[index + 1].str()) +
                            stoll(matches[index + 2].str()));
  return replicodeObjects_.getTimeReference() + us;
}

AeraGraphicsItem* AeraVisualizerWindow::getAeraGraphicsItem(Code* object, AeraVisualizerScene** scene)
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

void AeraVisualizerWindow::zoomToAeraGraphicsItem(Code* object)
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

void AeraVisualizerWindow::focusOnAeraGraphicsItem(Code* object)
{
  AeraVisualizerScene* scene;
  auto item = getAeraGraphicsItem(object, &scene);
  if (item) {
    if (item == hoverHighlightItem_ && !hoverHighlightItemWasVisible_)
      // The item is temporarily visible while hovering. Make sure it stays visible when we un-hover.
      hoverHighlightItemWasVisible_ = true;

    scene->focusOnItem(item);
  }
}

void AeraVisualizerWindow::centerOnAeraGraphicsItem(Code* object)
{
  AeraVisualizerScene* scene;
  auto item = getAeraGraphicsItem(object, &scene);
  if (item) {
    if (item == hoverHighlightItem_ && !hoverHighlightItemWasVisible_)
      // The item is temporarily visible while hovering. Make sure it stays visible when we un-hover.
      hoverHighlightItemWasVisible_ = true;

    scene->centerOnItem(item);
  }
}

void AeraVisualizerWindow::textItemHoverMoveEvent(const QTextDocument* document, QPointF position)
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

Timestamp AeraVisualizerWindow::getINextStepEvent
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
           event->eventType_ == SetModelStrengthEvent::EVENT_TYPE ||
           event->eventType_ == PhaseInModelEvent::EVENT_TYPE ||
           event->eventType_ == PhaseOutModelEvent::EVENT_TYPE ||
           event->eventType_ == DeleteModelEvent::EVENT_TYPE ||
           event->eventType_ == AbaMarkSentence::EVENT_TYPE ||
           event->eventType_ == AbaMarkedSentenceToParent::EVENT_TYPE) {
    // We already set the default iNextStepEvent.
  }
  else
    // Skip this event.
    return getINextStepEvent(maximumTime, iNextEventStart + 1, iNextStepEvent);

  return event->time_;
}

Timestamp AeraVisualizerWindow::stepEvent(Timestamp maximumTime)
{
  if (iNextEvent_ >= events_.size())
    // Return the value meaning no change.
    return Utils_MaxTime;

  AeraEvent* event = events_[iNextEvent_].get();
  if (event->time_ > maximumTime)
    return Utils_MaxTime;

  // Report the change in time to the find dialog
  findDialog_->reportStepEvent();

  auto relativeTime = duration_cast<microseconds>(event->time_ - replicodeObjects_.getTimeReference());
  auto frameStartTime = event->time_ - (relativeTime % replicodeObjects_.getSamplingPeriod());
  bool isNewFrame = (iNextEvent_ <= 0 || frameStartTime > events_[iNextEvent_ - 1]->time_);
  if (isNewFrame) {
    auto thisFrameMaxTime = frameStartTime + replicodeObjects_.getSamplingPeriod() - microseconds(1);

    // Set iCommand to the simulation event showing a ModelGoalReduction for a command (presumably the simulation's committed goal).
    // TODO: What about multiple committed goals including for mandatory solutions?
    int iCommand = -1;
    for (size_t i = iNextEvent_; i < events_.size(); ++i) {
      if (events_[i]->time_ > thisFrameMaxTime)
        // We searched the frame but didn't find a command.
        break;

      if (events_[i]->eventType_ == ModelGoalReduction::EVENT_TYPE) {
        auto value = ((ModelGoalReduction*)events_[i].get())->factGoal_->get_goal()->get_target()->get_reference(0);
        if (value->code(0).asOpcode() == Opcodes::Cmd) {
          iCommand = i;
          break;
        }
      }
    }

    if (iCommand >= 0) {
      // Start from the committed command and get the chain of inputs and set the simulation detail OIDs.
      set<int> focusSimulationDetailOids;
      set<int> otherDetailOids;
      int i = iCommand;
      while (i >= iNextEvent_) {
        focusSimulationDetailOids.insert(events_[i]->object_->get_detail_oid());
        if (allSimulationInputsCheckBox_->checkState() == Qt::Checked) {
          for (int j = 0; j < events_[i]->otherInputs_.size(); ++j)
            // These will be checked below.
            otherDetailOids.insert(events_[i]->otherInputs_[j]->get_detail_oid());
        }

        auto input = events_[i]->getInput();
        if (!input)
          // The end of the backward links, presumably the drive.
          break;

        // Keep searching backwards (back to the first simulation event) for the event of the input.
        --i;
        for (; i >= iNextEvent_; --i) {
          auto event = events_[i].get();
          if (event->object_ == input)
            break;

          if (allSimulationInputsCheckBox_->checkState() == Qt::Checked) {
            if (event->object_ && otherDetailOids.erase(event->object_->get_detail_oid()) > 0) {
              // Focus this event and queue up other inputs to focus on.
              focusSimulationDetailOids.insert(event->object_->get_detail_oid());
              if (event->getInput())
                otherDetailOids.insert(event->getInput()->get_detail_oid());
              for (int j = 0; j < event->otherInputs_.size(); ++j)
                otherDetailOids.insert(event->otherInputs_[j]->get_detail_oid());
            }
          }
        }
      }

      // This will display the focus simulation items at the top.
      mainScene_->setFocusSimulationDetailOids(focusSimulationDetailOids);
    }
  }

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

      // Restore the evidence count, success rate and strength in case we did a rewind.
      newModelEvent->object_->code(MDL_STRENGTH) = Atom::Float(newModelEvent->strength_);
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

      // Add an arrow to the requirement.
      if (reductionEvent->getRequirement()) {
        auto requirementItem = scene->getAeraGraphicsItem(reductionEvent->getRequirement());
        if (requirementItem)
          scene->addArrow(requirementItem, newItem);
      }

      visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelImdlPredictionEvent::EVENT_TYPE) {
      auto reductionEvent = (ModelImdlPredictionEvent*)event;
      newItem = new ModelImdlPredictionItem(reductionEvent, replicodeObjects_, scene);

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
      if (inputItem) {
        if (reductionEvent->factPredIsImdl_)
          // This is the imdl, not the RHS, so don't use red/green arrows.
          scene->addArrow(inputItem, newItem);
        else
          // If the input is a super goal, then the inputItem is the RHS, otherwise,
          // the input of the prediction is the LHS.
          scene->addArrow(inputItem, newItem, reductionEvent->inputIsSuperGoal_ ? newItem : inputItem);
      }

      // Add an arrow to the requirement.
      if (reductionEvent->requirement_) {
        auto requirementItem = scene->getAeraGraphicsItem(reductionEvent->requirement_);
        if (requirementItem)
          scene->addArrow(requirementItem, newItem);
      }

      scene->addHorizontalLine(newItem);

      if (newItem->is_sim())
        visible = (simulationsCheckBox_->checkState() == Qt::Checked);
      else
        visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelSimulatedPredictionReductionFromGoalRequirement::EVENT_TYPE) {
      auto reductionEvent = (ModelSimulatedPredictionReductionFromGoalRequirement*)event;
      newItem = new ModelPredictionFromRequirementItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(reductionEvent->input_);
      if (inputItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(inputItem, newItem);

      // Add an arrow to the signaling goal requirement.
      auto goalRequirementItem = scene->getAeraGraphicsItem(reductionEvent->goal_requirement_);
      if (goalRequirementItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(goalRequirementItem, newItem);

      scene->addHorizontalLine(newItem);

      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == ModelPredictionFromRequirementDisabledEvent::EVENT_TYPE) {
      auto requirementDisabledEvent = (ModelPredictionFromRequirementDisabledEvent*)event;
      newItem = new ModelPredictionFromRequirementDisabledItem(requirementDisabledEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(requirementDisabledEvent->input_);
      if (inputItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(inputItem, newItem);

      // Add an arrow to the strong requirement.
      auto strongRequirementItem = scene->getAeraGraphicsItem(requirementDisabledEvent->strong_requirement_);
      if (strongRequirementItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(strongRequirementItem, newItem);

      if (requirementDisabledEvent->goal_requirement_) {
        // Add an arrow to the goal requirement.
        auto goalRequirementItem = scene->getAeraGraphicsItem(requirementDisabledEvent->goal_requirement_);
        if (goalRequirementItem)
          // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
          scene->addArrow(goalRequirementItem, newItem);
      }

      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == CompositeStateSimulatedPredictionReduction::EVENT_TYPE) {
      auto reductionEvent = (CompositeStateSimulatedPredictionReduction*)event;
      newItem = new CompositeStatePredictionItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(reductionEvent->input_);
      if (inputItem)
        scene->addArrow(inputItem, newItem, inputItem);

      // Add arrows to the inputs.
      for (int i = 0; i < reductionEvent->inputs_.size(); ++i) {
        auto referencedItem = scene->getAeraGraphicsItem(reductionEvent->inputs_[i]);
        if (!referencedItem)
          continue;
        if (referencedItem == inputItem)
          // We already added the arrow above.
          continue;
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
    else if (event->eventType_ == NewPredictedInstantiatedCompositeStateEvent::EVENT_TYPE) {
      auto newIcstEvent = (NewPredictedInstantiatedCompositeStateEvent*)event;
      newItem = new PredictedInstantiatedCompositeStateItem(newIcstEvent, replicodeObjects_, scene);

      // Add arrows to inputs.
      for (int i = 0; i < newIcstEvent->inputs_.size(); ++i) {
        auto referencedItem = scene->getAeraGraphicsItem(newIcstEvent->inputs_[i]);
        if (referencedItem)
          scene->addArrow(referencedItem, newItem);
      }

      visible = ((nonSimulationsCheckBox_->checkState() == Qt::Checked) && 
                 (predictedInstantiatedCompositeStatesCheckBox_->checkState() == Qt::Checked));
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
    else if (event->eventType_ == PromotedSimulatedPredictionEvent::EVENT_TYPE) {
      auto promotedPredictionEvent = (PromotedSimulatedPredictionEvent*)event;
      newItem = new PromotedPredictionItem(promotedPredictionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(promotedPredictionEvent->timingsFact_);
      if (inputItem)
        scene->addArrow(inputItem, newItem);

      // Add an arrow to the promoted from item.
      auto promotedFromItem = scene->getAeraGraphicsItem(promotedPredictionEvent->promotedFromFact_);
      if (promotedFromItem)
        scene->addArrow(promotedFromItem, newItem);

      scene->addHorizontalLine(newItem);

      if (newItem->is_sim())
        visible = (simulationsCheckBox_->checkState() == Qt::Checked);
      else
        visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == PromotedSimulatedPredictionDefeatEvent::EVENT_TYPE) {
      auto defeatEvent = (PromotedSimulatedPredictionDefeatEvent*)event;
      newItem = new PromotedPredictionDefeatedItem(defeatEvent, replicodeObjects_, scene);

      // Add an arrow from the input fact.
      auto inputItem = scene->getAeraGraphicsItem(defeatEvent->input_);
      if (inputItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(inputItem, newItem);

      // Add an arrow to the defeated promoted prediction item.
      auto promotedItem = scene->getAeraGraphicsItem(defeatEvent->promotedFact_);
      if (promotedItem)
        // This is not a normal prediction or abduction, so no LHS/RHS arrowheads.
        scene->addArrow(newItem, promotedItem);

      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == AbaAddSentence::EVENT_TYPE) {
      auto addEvent = (AbaAddSentence*)event;
      newItem = new AbaSentenceItem(addEvent, replicodeObjects_, scene);

      // Add an arrow to the parent fact.
      auto parentItem = scene->getAeraGraphicsItem(addEvent->parent_);
      if (parentItem) {
        if (((AbaSentenceItem*)newItem)->isBetweenProponentAndOpponent(parentItem))
          scene->addArrow(newItem, parentItem, Arrow::RedArrowheadPen,
            Arrow::RedArrowheadPen, Arrow::RedArrowheadPen);
        else if (((AbaSentenceItem*)newItem)->isBetweenOpponents(parentItem))
          scene->addArrow(newItem, parentItem, Arrow::GreenArrowheadPen,
            Arrow::GreenArrowheadPen, Arrow::GreenArrowheadPen);
        else
          scene->addArrow(newItem, parentItem);
      }

      scene->addHorizontalLine(newItem);

      visible = (simulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == NewInstantiatedModelEvent::EVENT_TYPE) {
      auto newImdlEvent = (NewInstantiatedModelEvent*)event;
      newItem = new ImdlItem(newImdlEvent, replicodeObjects_, scene);

      visible = ((instantiatedModelsCheckBox_->checkState() == Qt::Checked));

      // Add an arrow to the 'fact pred' of the 'fact pred fact imdl...'
      if (newImdlEvent->factPred_) {
        auto factItem = scene->getAeraGraphicsItem(newImdlEvent->factPred_);
        if (factItem)
          scene->addArrow(factItem, newItem);
      }
    }

    // Add the new item.
    scene->addAeraGraphicsItem(newItem);

    if (event->object_) {
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
  else if (event->eventType_ == SetModelStrengthEvent::EVENT_TYPE) {
    auto setStrengthEvent = (SetModelStrengthEvent*)event;

    // Save the current values for a later undo.
    setStrengthEvent->oldStrength_ = setStrengthEvent->object_->code(MDL_STRENGTH).asFloat();

    // Update the model.
    setStrengthEvent->object_->code(MDL_STRENGTH) = Atom::Float(setStrengthEvent->strength_);

    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(setStrengthEvent->object_));
    if (modelItem) {
      modelItem->updateFromModel();
      modelItem->strengthFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      modelsScene_->establishFlashTimer();
    }
  }
  else if (event->eventType_ == PhaseInModelEvent::EVENT_TYPE) {
    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::white);
  }
  else if (event->eventType_ == PhaseOutModelEvent::EVENT_TYPE) {
    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(phasedOutModelColor_);
  }
  else if (event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::gray);
  }
  else if (event->eventType_ == AbaMarkSentence::EVENT_TYPE) {
    auto markEvent = (AbaMarkSentence*)event;
    auto sentenceItem = dynamic_cast<AbaSentenceItem*>(mainScene_->getAeraGraphicsItem(markEvent->fact_));
    if (sentenceItem) {
      sentenceItem->setStatus(AeraGraphicsItem::STATUS_DONE);
      if (sentenceItem->isVisible()) {
        sentenceItem->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        mainScene_->establishFlashTimer();
      }

      if (markEvent->alsoMarkGraph_ && sentenceItem->getAeraEvent()->eventType_ == AbaAddSentence::EVENT_TYPE) {
        auto graph = mainScene_->getItemGroup(((AbaAddSentence*)sentenceItem->getAeraEvent())->graphId_);
        if (graph)
          graph->setBrush(AeraGraphicsItem::Color_opponent_finished_justification);
      }
    }
  }
  else if (event->eventType_ == AbaMarkedSentenceToParent::EVENT_TYPE) {
    auto markedSentenceItem = dynamic_cast<AbaSentenceItem*>
      (mainScene_->getAeraGraphicsItem(((AbaMarkedSentenceToParent*)event)->markedFact_));
    auto parentItem = mainScene_->getAeraGraphicsItem(((AbaMarkedSentenceToParent*)event)->parent_);
    if (markedSentenceItem && parentItem) {
      if (markedSentenceItem->isBetweenProponentAndOpponent(parentItem))
        mainScene_->addArrow(markedSentenceItem, parentItem, Arrow::RedArrowheadPen,
          Arrow::RedArrowheadPen, Arrow::RedArrowheadPen);
      else if (markedSentenceItem->isBetweenOpponents(parentItem))
        mainScene_->addArrow(markedSentenceItem, parentItem, Arrow::GreenArrowheadPen,
          Arrow::GreenArrowheadPen, Arrow::GreenArrowheadPen);
      else
        mainScene_->addArrow(markedSentenceItem, parentItem);

      if (markedSentenceItem->isVisible() && parentItem->isVisible()) {
        markedSentenceItem->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        parentItem->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        mainScene_->establishFlashTimer();
      }
    }
  }
  else {
    // Skip this event.
    ++iNextEvent_;
    return stepEvent(maximumTime);
  }

  ++iNextEvent_;

  return event->time_;
}

Timestamp AeraVisualizerWindow::unstepEvent(Timestamp minimumTime, bool& foundGraphicsItem)
{
  foundGraphicsItem = false;

  if (iNextEvent_ == 0)
    // Return the value meaning no change.
    return Utils_MaxTime;

  if (events_[iNextEvent_ - 1]->time_ < minimumTime)
    // Don't decrement iNextEvent_.
    return Utils_MaxTime;

  --iNextEvent_;

  // Report the change in time to the find dialog
  findDialog_->reportStepEvent();

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
      foundGraphicsItem = true;
      aeraGraphicsItem->removeArrowsAndHorizontalLines();
      scene->removeAeraGraphicsItem(aeraGraphicsItem);

      // If this item was highlighted, remove it and null it out
      if (scene->currentMatch_ == aeraGraphicsItem)
        scene->currentMatch_ = NULL;
      for (int i = 0; i < scene->allMatches_.size(); i++) {
        if (scene->allMatches_.at(i) == aeraGraphicsItem)
          scene->allMatches_.erase(scene->allMatches_.begin() + i);
      }

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
  else if (event->eventType_ == SetModelStrengthEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set to the old strength.
    auto setStrengthEvent = (SetModelStrengthEvent*)event;

    setStrengthEvent->object_->code(MDL_STRENGTH) = Atom::Float(setStrengthEvent->oldStrength_);

    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(setStrengthEvent->object_));
    if (modelItem) {
      modelItem->strengthFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;

      modelItem->updateFromModel();
      modelsScene_->establishFlashTimer();
    }
  }
  else if (event->eventType_ == PhaseInModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not phased out.
    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color. Assume the model was phased out before phase in.
      modelItem->setBrush(phasedOutModelColor_);
  }
  else if (event->eventType_ == PhaseOutModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not phased out.
    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::white);
  }
  else if (event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not deleted.
    auto modelItem = dynamic_cast<ModelItem*>(modelsScene_->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::white);
  }
  else if (event->eventType_ == AbaMarkSentence::EVENT_TYPE) {
    auto markEvent = (AbaMarkSentence*)event;
    auto sentenceItem = dynamic_cast<AbaSentenceItem*>(mainScene_->getAeraGraphicsItem(markEvent->fact_));
    if (sentenceItem) {
      // Revert to unmarked.
      sentenceItem->setStatus(AeraGraphicsItem::STATUS_PROCESSING);
      if (sentenceItem->isVisible()) {
        sentenceItem->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        mainScene_->establishFlashTimer();
      }

      if (markEvent->alsoMarkGraph_ && sentenceItem->getAeraEvent()->eventType_ == AbaAddSentence::EVENT_TYPE) {
        auto graph = mainScene_->getItemGroup(((AbaAddSentence*)sentenceItem->getAeraEvent())->graphId_);
        if (graph)
          // Revert to unmarked.
          graph->setBrush(AeraGraphicsItem::Color_opponent_unfinished_justification);
      }
    }
  }
  else if (event->eventType_ == AbaMarkedSentenceToParent::EVENT_TYPE) {
    auto markedSentenceItem = dynamic_cast<AbaSentenceItem*>(mainScene_->getAeraGraphicsItem(((AbaMarkedSentenceToParent*)event)->markedFact_));
    auto parentItem = mainScene_->getAeraGraphicsItem(((AbaMarkedSentenceToParent*)event)->parent_);
    if (markedSentenceItem) {
      markedSentenceItem->removeAndDeleteArrowToObject(((AbaMarkedSentenceToParent*)event)->parent_);

      if (parentItem && markedSentenceItem->isVisible() && parentItem->isVisible()) {
        markedSentenceItem->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        parentItem->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        mainScene_->establishFlashTimer();
      }
    }
  }
  else
    // Skip this event.
    return unstepEvent(minimumTime, foundGraphicsItem);

  if (iNextEvent_ > 0)
    return events_[iNextEvent_ - 1]->time_;
  else
    // The caller will use the time reference.
    return Timestamp(seconds(0));
}

void AeraVisualizerWindow::startPlay()
{
  if (isPlaying_)
    // Already playing.
    return;

  playPauseButton_->setIcon(pauseIcon_);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playPauseButton_->setIcon(pauseIcon_);
  isPlaying_ = true;
  if (playTimerId_ == 0)
    playTimerId_ = startTimer(AeraVisualizer_playTimerTick.count());
}

void AeraVisualizerWindow::stopPlay()
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

void AeraVisualizerWindow::setPlayTime(Timestamp time)
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

  QSettings settings;
  // If auto scroll is enabled, ensure the new item is visible
  if (mainScene_ && settings.value("AutoScroll", Qt::Unchecked).toInt() == Qt::Checked) {
    mainScene_->scrollToTimestamp(time);
  }
}

void AeraVisualizerWindow::setSliderToPlayTime()
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

void AeraVisualizerWindow::playPauseButtonClickedImpl()
{
  if (isPlaying_)
    stopPlay();
  else
    startPlay();
}

void AeraVisualizerWindow::stepButtonClickedImpl()
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
      // Set iNonSimulation to the next non-simulation event.
      for (iNonSimulation = iNextStepEvent; iNonSimulation < events_.size(); ++iNonSimulation) {
        if (simulationEventTypes_.find(events_[iNonSimulation]->eventType_) == simulationEventTypes_.end())
          break;
      }
    }
  }

  while (true) {
    if (stepEvent(thisFrameMaxTime) == Utils_MaxTime)
      break;
    eventTime = events_[iNextEvent_ - 1]->time_;

    if (simulationsCheckBox_->isChecked()) {
      if (singleStepSimulationCheckBox_->isChecked() &&
          simulationEventTypes_.find(events_[iNextEvent_ - 1]->eventType_) != simulationEventTypes_.end()) {
        // Single-step through the simulation.
        _Fact* markedFact = 0;
        if (events_[iNextEvent_ - 1]->eventType_ == AbaMarkSentence::EVENT_TYPE)
          markedFact = ((AbaMarkSentence*)events_[iNextEvent_ - 1].get())->fact_;
        if (markedFact && iNextEvent_ < events_.size() &&
          (events_[iNextEvent_]->eventType_ == AbaAddSentence::EVENT_TYPE &&
            ((AbaAddSentence*)events_[iNextEvent_].get())->parent_ == markedFact
            ||
            events_[iNextEvent_]->eventType_ == AbaMarkedSentenceToParent::EVENT_TYPE &&
            (((AbaMarkedSentenceToParent*)events_[iNextEvent_].get())->parent_ == markedFact ||
              ((AbaMarkedSentenceToParent*)events_[iNextEvent_].get())->markedFact_ == markedFact))) {
          // The next event will have an arrow with this. Don't break so that we show it right away.
        }
        else
          break;
      }

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

void AeraVisualizerWindow::stepBackButtonClickedImpl()
{
  stopPlay();
  bool foundGraphicsItem;
  auto newTime = max(unstepEvent(Timestamp(seconds(0)), foundGraphicsItem), replicodeObjects_.getTimeReference());
  if (newTime == Utils_MaxTime)
    return;
  // Debug: How to step the children also?

  // Keep unstepping remaining events in this same frame.
  auto relativeTime = duration_cast<microseconds>(newTime - replicodeObjects_.getTimeReference());
  auto frameStartTime = newTime - (relativeTime % replicodeObjects_.getSamplingPeriod());
  while (true) {
    if (simulationsCheckBox_->isChecked() && singleStepSimulationCheckBox_->isChecked() && foundGraphicsItem &&
        simulationEventTypes_.find(events_[iNextEvent_]->eventType_) != simulationEventTypes_.end())
      // Single-step through the simulation.
      break;

    auto localNewTime = unstepEvent(frameStartTime, foundGraphicsItem);
    if (localNewTime == Utils_MaxTime)
      break;
    newTime = localNewTime;
  }

  setPlayTime(max(newTime, replicodeObjects_.getTimeReference()));
  setSliderToPlayTime();
}

void AeraVisualizerWindow::playTimeLabelClickedImpl()
{
  showRelativeTime_ = !showRelativeTime_;
  setPlayTime(playTime_);
}

void AeraVisualizerWindow::timerEvent(QTimerEvent* event)
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
  auto playTime = playTime_ + AeraVisualizer_playTimerTick;

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

void AeraVisualizerWindow::closeEvent(QCloseEvent* event) {
  findDialog_->close();
  event->accept();
}

void AeraVisualizerWindow::zoomIn()
{
  // Make sure zoom is focused on the center of the screen
  QGraphicsView* view = selectedScene_->views().at(0);
  view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

  // Zoom in
  selectedScene_->scaleViewBy(1.09);
}

void AeraVisualizerWindow::zoomOut()
{
  // Make sure zoom is focused on the center of the screen
  QGraphicsView* view = selectedScene_->views().at(0);
  view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

  // Zoom out
  selectedScene_->scaleViewBy(1 / 1.09);
}

void AeraVisualizerWindow::zoomHome()
{
  selectedScene_->zoomViewHome();
}

void AeraVisualizerWindow::find()
{
  // Don't open the dialog multiple times, just bring it forward
  if (!findDialog_->isVisible()) {
    findDialog_->show();
  }
  else {
    findDialog_->activateWindow();
  }
  return;
}

void AeraVisualizerWindow::findNext()
{
  findDialog_->findNext();
  return;
}

void AeraVisualizerWindow::findPrev()
{
  findDialog_->findPrev();
  return;
}

void AeraVisualizerWindow::fitAll() {
  findDialog_->fitAll();
  return;
}

void AeraVisualizerWindow::createActions()
{
  exitAction_ = new QAction(tr("E&xit"), this);
  exitAction_->setShortcuts(QKeySequence::Quit);
  connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  zoomInAction_ = new QAction(QIcon(":/images/zoom-in.png"), tr("Zoom In"), this);
  zoomInAction_->setStatusTip(tr("Zoom In"));
  zoomInAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Equal));
  connect(zoomInAction_, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAction_ = new QAction(QIcon(":/images/zoom-out.png"), tr("Zoom Out"), this);
  zoomOutAction_->setStatusTip(tr("Zoom Out"));
  zoomOutAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));
  connect(zoomOutAction_, SIGNAL(triggered()), this, SLOT(zoomOut()));

  zoomHomeAction_ = new QAction(QIcon(":/images/zoom-home.png"), tr("Zoom Home"), this);
  zoomHomeAction_->setStatusTip(tr("Zoom to show all"));
  zoomHomeAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Home));
  connect(zoomHomeAction_, SIGNAL(triggered()), this, SLOT(zoomHome()));

  findAction_ = new QAction(QIcon(":/images/zoom-to.png"), tr("Find"), this);
  findAction_->setStatusTip(tr("Find a specific object"));
  findAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  connect(findAction_, SIGNAL(triggered()), this, SLOT(find()));

  findNextAction_ = new QAction(tr("Find Next"), this);
  findNextAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
  connect(findNextAction_, SIGNAL(triggered()), this, SLOT(findNext()));
  this->addAction(findNextAction_);

  findPrevAction_ = new QAction(tr("Find Prev"), this);
  findPrevAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
  connect(findPrevAction_, SIGNAL(triggered()), this, SLOT(findPrev()));
  this->addAction(findPrevAction_);

  fitAllAction_ = new QAction(tr("Fit All Matches"), this);
  fitAllAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Home));
  connect(fitAllAction_, SIGNAL(triggered()), this, SLOT(fitAll()));
  this->addAction(fitAllAction_);
}

void AeraVisualizerWindow::createMenus()
{
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(exitAction_);

  QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomHomeAction_);
  viewMenu->addAction(zoomInAction_);
  viewMenu->addAction(zoomOutAction_);
  viewMenu->addAction(findAction_);

  QMenu* findMenu = menuBar()->addMenu(tr("Fin&d"));
  findMenu->addAction(findAction_);
  findMenu->addAction(findNextAction_);
  findMenu->addAction(findPrevAction_);
  findMenu->addAction(fitAllAction_);
}

void AeraVisualizerWindow::createToolbars()
{
  QToolBar* toolbar = addToolBar(tr("Main"));
  toolbar->addAction(zoomHomeAction_);
  toolbar->addAction(zoomInAction_);
  toolbar->addAction(zoomOutAction_);
  toolbar->addAction(findAction_);

  toolbar->addSeparator();
  // Checkbox for auto scroll
  toolbar->addWidget(new AeraCheckbox("Auto-scroll", SettingsKeyAutoScroll, this));

  toolbar->addSeparator();
  toolbar->addWidget(new QLabel("Show/Hide: ", this));

  const QColor simulationColor("#ffffdc");
  // Show simulations by default.
  simulationsCheckBox_ = new AeraCheckbox("Simulations", SettingsKeySimulationsVisible, this, Qt::Checked);
  simulationsCheckBox_->setColor(simulationColor);
  connect(simulationsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    allSimulationInputsCheckBox_->setEnabled(state == Qt::Checked);
    singleStepSimulationCheckBox_->setEnabled(state == Qt::Checked);

    for (auto i = simulationEventTypes_.begin(); i != simulationEventTypes_.end(); ++i)
      mainScene_->setItemsVisible(*i, state == Qt::Checked);
    });
  toolbar->addWidget(simulationsCheckBox_);

  allSimulationInputsCheckBox_ = new AeraCheckbox("All Inputs", SettingsKeyAllSimulationInputsVisible, this, Qt::Unchecked);
  allSimulationInputsCheckBox_->setColor(simulationColor);
  toolbar->addWidget(allSimulationInputsCheckBox_);

  singleStepSimulationCheckBox_ = new AeraCheckbox("Single Step", SettingsKeySingleStepSimulationVisible, this, Qt::Unchecked);
  singleStepSimulationCheckBox_->setColor(simulationColor);
  toolbar->addWidget(singleStepSimulationCheckBox_);

  // Separate the non-simulations check boxes.
  toolbar->addWidget(new QLabel("    ", this));

  // Show non-simulations by default.
  nonSimulationsCheckBox_ = new AeraCheckbox("Non-Simulations", SettingsKeyNonSimulationsVisible, this, Qt::Checked);
  connect(nonSimulationsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    essenceFactsCheckBox_->setEnabled(state == Qt::Checked);
    instantiatedCompositeStatesCheckBox_->setEnabled(state == Qt::Checked);
    predictedInstantiatedCompositeStatesCheckBox_->setEnabled(state == Qt::Checked);
    requirementsCheckBox_->setEnabled(state == Qt::Checked);

    // Do the opposite of simulationsCheckBox_ .
    mainScene_->setNonItemsVisible(simulationEventTypes_, state == Qt::Checked);
    if (state == Qt::Checked) {
      // Make specific non-simulation items not visible, if needed.
      mainScene_->setAutoFocusItemsVisible("essence", essenceFactsCheckBox_->checkState() == Qt::Checked);
      mainScene_->setItemsVisible(
        NewInstantiatedCompositeStateEvent::EVENT_TYPE, instantiatedCompositeStatesCheckBox_->checkState() == Qt::Checked);
      mainScene_->setItemsVisible(
        NewPredictedInstantiatedCompositeStateEvent::EVENT_TYPE, predictedInstantiatedCompositeStatesCheckBox_->checkState() == Qt::Checked);
      mainScene_->setItemsVisible(
        ModelImdlPredictionEvent::EVENT_TYPE, requirementsCheckBox_->checkState() == Qt::Checked);
    }
  });
  toolbar->addWidget(nonSimulationsCheckBox_);

  essenceFactsCheckBox_ = new AeraCheckbox("Essence Facts", SettingsKeyEssenceFactsVisible, this);
  connect(essenceFactsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setAutoFocusItemsVisible("essence", state == Qt::Checked);  });
  toolbar->addWidget(essenceFactsCheckBox_);

  instantiatedCompositeStatesCheckBox_ = new AeraCheckbox("Instantiated Comp. States", SettingsKeyInstantiatedCompositeStatesVisible, this);
  connect(instantiatedCompositeStatesCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewInstantiatedCompositeStateEvent::EVENT_TYPE, state == Qt::Checked); });
  toolbar->addWidget(instantiatedCompositeStatesCheckBox_);

  instantiatedModelsCheckBox_ = new AeraCheckbox("Instantiated Models", SettingsKeyInstantiatedModelsVisible, this);
  connect(instantiatedModelsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewInstantiatedModelEvent::EVENT_TYPE, state == Qt::Checked); });
  toolbar->addWidget(instantiatedModelsCheckBox_);

  predictedInstantiatedCompositeStatesCheckBox_ = new AeraCheckbox("Pred. Instantiated Comp. States", SettingsKeyPredictedInstantiatedCompositeStatesVisible, this);
  connect(predictedInstantiatedCompositeStatesCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewPredictedInstantiatedCompositeStateEvent::EVENT_TYPE, state == Qt::Checked); });
  toolbar->addWidget(predictedInstantiatedCompositeStatesCheckBox_);

  requirementsCheckBox_ = new AeraCheckbox("Requirements", SettingsKeyRequirementsVisible, this);
  connect(requirementsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(ModelImdlPredictionEvent::EVENT_TYPE, state == Qt::Checked);  });
  toolbar->addWidget(requirementsCheckBox_);
}

}
