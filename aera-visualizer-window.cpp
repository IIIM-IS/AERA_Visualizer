//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2026 Jeff Thompson
//_/_/ Copyright (c) 2018-2026 Kristinn R. Thorisson
//_/_/ Copyright (c) 2023-2026 Chloe Schaff
//_/_/ Copyright (c) 2018-2026 Icelandic Institute for Intelligent Machines
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
#include "graphics-items/reduction-marker-item.hpp"
#include "graphics-items/simulation-commit-item.hpp"
#include "submodules/AERA/r_exec/opcodes.h"
#include "submodules/AERA/AERA/settings.h"
#include "submodules/AERA/AERA/main.h"

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
  AbaSolutionFound::EVENT_TYPE,
  AbaMarkSentence::EVENT_TYPE,
  AbaMarkedSentenceToParent::EVENT_TYPE,
  AbaBindVariable::EVENT_TYPE,
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
  NewReductionMarkerEvent::EVENT_TYPE,
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

AeraVisualizerWindow::AeraVisualizerWindow()
: QMainWindow(0),
  aera_(0),
  iNextEvent_(0), explanationLogView_(0),
  essencePropertyObject_(NULL),
  hoverHighlightItem_(0),
  phasedOutModelColor_(255, 192, 192),
  abagraph_("", replicodeObjects_), // ("/work/abagraph-mercury/mercury/abagraph.exe", replicodeObjects_),
  itemBorderHighlightPen_(Qt::blue, 3)
{
  createActions();
  createToolbars();

  mainScene_ = new AeraVisualizerScene(this, true);

  // Use a MyQGraphicsView so that we can track movements to the scene view.
  auto mainSceneView = new MyQGraphicsView(mainScene_, this);
  mainSceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  mainSceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
  // Set a default selected scene.
  selectedScene_ = mainScene_;
  
  createDockWidgets();
  createMenus();
  createStatusBar();

  auto centralWidget = new QWidget();
  auto centralLayout = new QVBoxLayout();
  centralLayout->addWidget(mainSceneView);
  centralLayout->addWidget(timelineControls_);
  centralWidget->setLayout(centralLayout);
  setCentralWidget(centralWidget);

  setWindowTitle(tr("AERA Visualizer (EXPERIMENTAL)"));
  setUnifiedTitleAndToolBarOnMac(true);

  // Reset the widgets to the way they were last time
  QSettings preferences;
  restoreGeometry(preferences.value("geometry").toByteArray());
  restoreState(preferences.value("state").toByteArray());

  // Turn everything off until something's loaded in
  setUIEnabled(false);

  // Indicate that AERA hasn't been started
  setAERAstatus("AERA instance has not been started", true);
  
  // This feature isn't implemented yet so just leave this one empty
  setOperatingModeStatus("", PAUSED);
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
  // Start solution 8 from solution 5 step 45
  regex abaSolutionStartRegex("^Start solution (\\d+) from solution (\\d+) step (\\d+)$");
  // Solution 5
  regex abaSolutionIdRegex("^Solution (\\d+)$");
  // Step 0: Case init: S: 304
  regex abaCaseInitStepRegex("^Step (\\d+): Case init: S: (\\d+)$");
  // Step 10: Case 1.(i): A: 314, Contrary 322 has body? Y, NewGId 1
  // TODO: Handle when the Contrary already exists.
  regex abaCase1iStepRegex("^Step (\\d+): Case 1\\.\\(i\\): A: (\\d+), Contrary (\\d+) has body\\? (\\w), NewGId (\\d+)");
  // Step 10: Case 1.(ii): S: 304, NewUnMarkedAs: [314 316], NewUnMarkedNonAs: [312], ExistingBody: [310]
  regex abaCase1iiStepRegex("^Step (\\d+): Case 1\\.\\(ii\\): S: (\\d+), NewUnMarkedAs: \\[(.*)\\], NewUnMarkedNonAs: \\[(.*)\\], ExistingBody: \\[(.*)\\]$");
  // Step 10: Case 1.(iii): (:= (var 3) 15.000000)
  // Step 10: Case 1.(iii): (<= (var 3) 15.000000)
  regex abaCase1Or2iiiStepRegex("^Step (\\d+): Case [12]\\.\\(iii\\): \\((:=|<=) \\(var (\\d+)\\) (-?[\\.\\w]+)\\)$");
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
  // Step 10: Solution found
  regex abaSolutionFound("^Step (\\d+): Solution found$");

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
  int abaSolutionId = 1;
  abaSolutions_[abaSolutionId] = AbaSolution(0, 0);
  // Map of solutions ID -> max step number of solution steps that have already been copied to events_.
  map<int, int> solutionMaxStepCopied;
  while (getline(runtimeOutputFile, line)) {
    if (progress.wasCanceled())
      return false;

    ++lineNumber;
    progress.setValue(lineNumber);
    if (lineNumber % 100 == 0)
      QApplication::processEvents();

    // Fast foward past the last line read
    if (lineNumber <= lastLine_)
      continue;
    else
      lastLine_ = lineNumber;

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
      auto reduction = (MkRdx*)replicodeObjects_.getObject(stoul(matches[1].str()));
      if (reduction && reduction->code(0).asOpcode() == Opcodes::MkRdx) {
        // Check the type of prediction.
        auto factPred = AeraEvent::getFirstProduction(reduction);
        auto pred = factPred->get_reference(0);
        auto factValue = pred->get_reference(0);
        auto value = factValue->get_reference(0);
        auto valueOpcode = value->code(0).asOpcode();

        if (valueOpcode == Opcodes::MkVal) {
          events_.push_back(make_shared<ModelMkValPredictionReduction>(timestamp, reduction));
          events_.push_back(make_shared<NewInstantiatedModelEvent>(
            timestamp, reduction, factPred));
        }

        events_.push_back(make_shared<NewReductionMarkerEvent>(timestamp, reduction));
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
    else if (regex_search(lineAfterTimestamp, matches, abaSolutionStartRegex)) {
      int solutionId = stoul(matches[1].str());
      int parentSolutionId = stoul(matches[2].str());
      int parentStep = stoul(matches[3].str());
      abaSolutions_[solutionId] = AbaSolution(parentSolutionId, parentStep);
    }
    else if (regex_search(lineAfterTimestamp, matches, abaSolutionIdRegex)) {
      abaSolutionId = stoul(matches[1].str());
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCaseInitStepRegex)) {
      int step = stoul(matches[1].str());
      auto fact = abagraph_.getObject(stoul(matches[2].str()));
      if (fact) {
        addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
          timestamp, fact, false, true, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER, (Code*)NULL, "init"));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase1iStepRegex)) {
      int step = stoul(matches[1].str());
      auto assumption = abagraph_.getObject(stoul(matches[2].str()));
      auto contrary = abagraph_.getObject(stoul(matches[3].str()));
      bool contraryHasBody = (matches[4].str() == "Y");
      int newGId = stoul(matches[5].str());

      if (assumption && newGId > 0 && contrary) {
        // This step sets the assumption to marked.
        addAbaEvent(abaSolutionId, step, make_shared<AbaMarkSentence>(timestamp, assumption));
        // TODO: If newGId == 0 then find the contrary in an existing group.
        // TODO: Maybe add option to show singleton opponent graphs where contraryHasBody is false.
        if (newGId > 0 && contraryHasBody)
          addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
            timestamp, contrary, false, true, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER + newGId, assumption, 
            "1.(i)", stoul(matches[1].str())));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase1iiStepRegex)) {
      int step = stoul(matches[1].str());
      auto head = abagraph_.getObject(stoul(matches[2].str()));
      vector<Code*> newUnmarkedAssumptions;
      vector<Code*> newUnmarkedNonAssumptions;
      vector<Code*> existingBody;

      if (head &&
          abagraph_.getObjects(matches[3].str(), newUnmarkedAssumptions) &&
          abagraph_.getObjects(matches[4].str(), newUnmarkedNonAssumptions) &&
          abagraph_.getObjects(matches[5].str(), existingBody)) {
        // This step sets the head to marked.
        addAbaEvent(abaSolutionId, step, make_shared<AbaMarkSentence>(timestamp, head));

        for (auto fact = existingBody.begin(); fact != existingBody.end(); ++fact)
          addAbaEvent(abaSolutionId, step, make_shared<AbaMarkedSentenceToParent>(timestamp, *fact, head));
        for (auto fact = newUnmarkedAssumptions.begin(); fact != newUnmarkedAssumptions.end(); ++fact)
          addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
            timestamp, *fact, true, false, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER, head,
            "1.(ii)", stoul(matches[1].str())));
        for (auto fact = newUnmarkedNonAssumptions.begin(); fact != newUnmarkedNonAssumptions.end(); ++fact)
          addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
            timestamp, *fact, false, false, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER, head,
            "1.(ii)", stoul(matches[1].str())));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase1Or2iiiStepRegex)) {
      int step = stoul(matches[1].str());
      int varNumber = stoul(matches[3].str());
      QString value = matches[4].str().c_str();
      if (value.contains(".")) {
        // Simplify the float.
        bool ok;
        double d = value.toDouble(&ok);
        if (ok)
          value = QString::number(d, 'f', 1);
      }
      addAbaEvent(abaSolutionId, step, make_shared<AbaBindVariable>(timestamp, varNumber, value));
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2iaStepRegex)) {
      int step = stoul(matches[1].str());
      auto fact = abagraph_.getObject(stoul(matches[2].str()));

      if (fact) {
        // (Don't mark the graph.)
        addAbaEvent(abaSolutionId, step, make_shared<AbaMarkSentence>(timestamp, fact, false));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2ibStepRegex)) {
      int step = stoul(matches[1].str());
      auto fact = abagraph_.getObject(stoul(matches[2].str()));
      auto culprit = abagraph_.getObject(stoul(matches[4].str()));

      if (fact) {
        // (Also mark the graph that the fact is in.)
        addAbaEvent(abaSolutionId, step, make_shared<AbaMarkSentence>(timestamp, fact, true));

        if (culprit)
          // The fact is the same as the culprit in a different graph.
          addAbaEvent(abaSolutionId, step, make_shared<AbaMarkedSentenceToParent>(timestamp, fact, culprit));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2icStepRegex)) {
      int step = stoul(matches[1].str());
      auto fact = abagraph_.getObject(stoul(matches[2].str()));
      auto contrary = abagraph_.getObject(stoul(matches[4].str()));
      bool contraryIsNew = (matches[5].str() == "Y");

      if (fact && contrary) {
        // (Also mark the graph that the fact is in.)
        addAbaEvent(abaSolutionId, step, make_shared<AbaMarkSentence>(timestamp, fact, true));
        if (contraryIsNew)
          addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
            timestamp, contrary, false, false, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER, fact,
            "2.(ic)", stoul(matches[1].str())));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2iiMarkStepRegex)) {
      int step = stoul(matches[1].str());
      auto head = abagraph_.getObject(stoul(matches[2].str()));
      bool markGraph = (matches[3].str() == "Y");

      if (head) {
        // This step sets the head to marked. Further actions are in abaCase2iiStepRegex.
        addAbaEvent(abaSolutionId, step, make_shared<AbaMarkSentence>(timestamp, head, markGraph));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaCase2iiStepRegex)) {
      int step = stoul(matches[1].str());
      auto head = abagraph_.getObject(stoul(matches[2].str()));
      int newGraphId = stoul(matches[3].str());
      vector<Code*> newUnmarkedAssumptions;
      vector<Code*> newUnmarkedNonAssumptions;
      vector<Code*> existingBody;

      if (head &&
          abagraph_.getObjects(matches[4].str(), newUnmarkedAssumptions) &&
          abagraph_.getObjects(matches[5].str(), newUnmarkedNonAssumptions) &&
          abagraph_.getObjects(matches[6].str(), existingBody)) {
        // We have already set the head to marked with abaCase2iiMarkStepRegex. Don't add AbaMarkSentence.

        for (auto fact = existingBody.begin(); fact != existingBody.end(); ++fact)
          addAbaEvent(abaSolutionId, step, make_shared<AbaMarkedSentenceToParent>(timestamp, *fact, head));
        for (auto fact = newUnmarkedAssumptions.begin(); fact != newUnmarkedAssumptions.end(); ++fact)
          addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
            timestamp, *fact, true, false, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER + newGraphId, head,
            "2.(ii)", stoul(matches[1].str())));
        for (auto fact = newUnmarkedNonAssumptions.begin(); fact != newUnmarkedNonAssumptions.end(); ++fact)
          addAbaEvent(abaSolutionId, step, make_shared<AbaAddSentence>(
            timestamp, *fact, false, false, abaSolutionId * PROPONENT_GRAPH_ID_MULTIPLIER + newGraphId, head,
            "2.(ii)", stoul(matches[1].str())));
      }
    }
    else if (regex_search(lineAfterTimestamp, matches, abaSolutionFound)) {
      int step = stoul(matches[1].str());
      addAbaEvent(abaSolutionId, step, make_shared<AbaSolutionFound>(timestamp, abaSolutionId));

      // Copy from abaEvents_ working backwards through parent solutions.
      vector<shared_ptr<AeraEvent> > reverseEvents;
      int solutionId = abaSolutionId;
      int maxStepToCopy = INT_MAX;
      while (true) {
        int maxStepCopied = (solutionMaxStepCopied.count(solutionId) > 0 ? solutionMaxStepCopied[solutionId] : - 1);
        int maxStepCopiedThisPass = -1;
        for (auto it = abaSolutions_[solutionId].abaEvents_.rbegin();
             it != abaSolutions_[solutionId].abaEvents_.rend(); ++it) {
          if (it->first <= maxStepCopied)
            // We have already copied this step and lower.
            break;
          if (it->first > maxStepToCopy)
            // Wait to start copying.
            continue;

          maxStepCopiedThisPass = max(maxStepCopiedThisPass, it->first);
          // Also reverse the list of events.
          reverseEvents.insert(reverseEvents.end(), it->second.rbegin(), it->second.rend());
        }
        // Update the highest step copied for this solutionId.
        solutionMaxStepCopied[solutionId] = max(maxStepCopied, maxStepCopiedThisPass);

        // Update solutionId with the parent. We will start copying at parentSolutionStep_.
        maxStepToCopy = abaSolutions_[solutionId].parentSolutionStep_;
        solutionId = abaSolutions_[solutionId].parentSolutionId_;
        if (solutionId == 0)
          break;
      }

      // Reverse copy to events_.
      events_.insert(events_.end(), reverseEvents.rbegin(), reverseEvents.rend());
      // Expect to match abaSolutionStartRegex which will set abaSolutionId.
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

void AeraVisualizerWindow::addStartupItems()
{
  for (int i = 0; i < startupEvents_.size(); ++i) {
    AeraEvent* event = startupEvents_[i].get();
    if (event->time_ > replicodeObjects_.getTimeReference())
      // Finished scanning the initial events.
      return;

    if (event->eventType_ == NewModelEvent::EVENT_TYPE)
      // TODO: Add arrows.
      semanticsView_->getModelsScene()->addAeraGraphicsItem(
        new ModelItem((NewModelEvent*)event, replicodeObjects_, semanticsView_->getModelsScene()));
    else if (event->eventType_ == NewCompositeStateEvent::EVENT_TYPE)
      // TODO: Add arrows.
      semanticsView_->getModelsScene()->addAeraGraphicsItem(
        new CompositeStateItem((NewCompositeStateEvent*)event, replicodeObjects_, semanticsView_->getModelsScene()));
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

  auto item = semanticsView_->getModelsScene()->getAeraGraphicsItem(object);
  if (item) {
    if (scene)
      *scene = semanticsView_->getModelsScene();
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
      hoverHighlightItem_->setItemAndArrowsAndHorizontalLineVisible(hoverHighlightItemWasVisible_);
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
        hoverHighlightItem_->setItemAndArrowsAndHorizontalLineVisible(hoverHighlightItemWasVisible_);
        hoverHighlightItem_ = 0;
      }

      hoverHighlightItem_ = getAeraGraphicsItem(object);
      if (hoverHighlightItem_) {
        hoverHighlightItemWasVisible_ = hoverHighlightItem_->isVisible();
        if (!hoverHighlightItemWasVisible_)
          // Make the item visible while we hover.
          hoverHighlightItem_->setItemAndArrowsAndHorizontalLineVisible(true);

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
           event->eventType_ == AbaSolutionFound::EVENT_TYPE ||
           event->eventType_ == AbaMarkSentence::EVENT_TYPE ||
           event->eventType_ == AbaMarkedSentenceToParent::EVENT_TYPE ||
           event->eventType_ == AbaBindVariable::EVENT_TYPE) {
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
      scene = semanticsView_->getModelsScene();
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
    else if (event->eventType_ == NewReductionMarkerEvent::EVENT_TYPE) {
      auto newReductionMarkerEvent = (NewReductionMarkerEvent*)event;

      newItem = new ReductionMarkerItem(newReductionMarkerEvent, replicodeObjects_, scene);

      visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
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
        else if (((AbaSentenceItem*)newItem)->isBetweenProponentGraphs(parentItem) ||
                 ((AbaSentenceItem*)newItem)->isBetweenOpponentGraphs(parentItem))
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
    if (newItem->getAeraEvent()->eventType_ == AbaAddSentence::EVENT_TYPE && bindings_.size() > 0) {
      for (pair<int, QString> pair : bindings_)
        ((AbaSentenceItem*)newItem)->setBinding(pair.first, pair.second);
    }

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

    // Call setItemAndArrowsAndHorizontalLineVisible, even if visible is true because we need to hide arrows to non-visible items.
    newItem->setItemAndArrowsAndHorizontalLineVisible(visible);

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

    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(setSuccessRateEvent->object_));
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
      semanticsView_->getModelsScene()->establishFlashTimer();
    }
  }
  else if (event->eventType_ == SetModelStrengthEvent::EVENT_TYPE) {
    auto setStrengthEvent = (SetModelStrengthEvent*)event;

    // Save the current values for a later undo.
    setStrengthEvent->oldStrength_ = setStrengthEvent->object_->code(MDL_STRENGTH).asFloat();

    // Update the model.
    setStrengthEvent->object_->code(MDL_STRENGTH) = Atom::Float(setStrengthEvent->strength_);

    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(setStrengthEvent->object_));
    if (modelItem) {
      modelItem->updateFromModel();
      modelItem->strengthFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
      semanticsView_->getModelsScene()->establishFlashTimer();
    }
  }
  else if (event->eventType_ == PhaseInModelEvent::EVENT_TYPE) {
    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::white);
  }
  else if (event->eventType_ == PhaseOutModelEvent::EVENT_TYPE) {
    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(phasedOutModelColor_);
  }
  else if (event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(event->object_));
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
      else if (markedSentenceItem->isBetweenProponentGraphs(parentItem) ||
               markedSentenceItem->isBetweenOpponentGraphs(parentItem))
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
  else if (event->eventType_ == AbaBindVariable::EVENT_TYPE) {
    auto bindEvent = (AbaBindVariable*)event;
    auto entry = bindings_.find(bindEvent->varNumber_);
    if (entry == bindings_.end() || entry->second != bindEvent->value_) {
      // TODO: Flash changed items.
      bindings_[bindEvent->varNumber_] = bindEvent->value_;
      mainScene_->abaSetBinding(bindEvent->varNumber_, bindEvent->value_);
    }
  }
  else if (event->eventType_ == AbaSolutionFound::EVENT_TYPE) {
    auto solutionFoundEvent = (AbaSolutionFound*)event;
    auto graph = mainScene_->getItemGroup(solutionFoundEvent->solutionId_ * PROPONENT_GRAPH_ID_MULTIPLIER);
    if (graph)
      graph->setBrush(AeraGraphicsItem::Color_proponent_justifications);
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
      scene = semanticsView_->getModelsScene();
    else
      scene = mainScene_;

    // Find the AeraGraphicsItem for this event and remove it.
    // Note that the event saves the updated item position and will use it when recreating the item.
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(scene->getAeraGraphicsItem(event->object_));
    if (aeraGraphicsItem) {
      foundGraphicsItem = true;
      aeraGraphicsItem->removeArrowsAndHorizontalLine();
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

    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(setSuccessRateEvent->object_));
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
      semanticsView_->getModelsScene()->establishFlashTimer();
    }
  }
  else if (event->eventType_ == SetModelStrengthEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set to the old strength.
    auto setStrengthEvent = (SetModelStrengthEvent*)event;

    setStrengthEvent->object_->code(MDL_STRENGTH) = Atom::Float(setStrengthEvent->oldStrength_);

    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(setStrengthEvent->object_));
    if (modelItem) {
      modelItem->strengthFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;

      modelItem->updateFromModel();
      semanticsView_->getModelsScene()->establishFlashTimer();
    }
  }
  else if (event->eventType_ == PhaseInModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not phased out.
    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color. Assume the model was phased out before phase in.
      modelItem->setBrush(phasedOutModelColor_);
  }
  else if (event->eventType_ == PhaseOutModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not phased out.
    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(event->object_));
    if (modelItem)
      // Set the background color.
      modelItem->setBrush(Qt::white);
  }
  else if (event->eventType_ == DeleteModelEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set its appearance to not deleted.
    auto modelItem = dynamic_cast<ModelItem*>(semanticsView_->getModelsScene()->getAeraGraphicsItem(event->object_));
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
  else if (event->eventType_ == AbaBindVariable::EVENT_TYPE) {
    auto bindEvent = (AbaBindVariable*)event;
    auto entry = bindings_.find(bindEvent->varNumber_);
    if (entry != bindings_.end()) {
      // TODO: Flash changed items.
      bindings_.erase(entry);
      mainScene_->abaRemoveBinding(bindEvent->varNumber_);
    }
  }
  else if (event->eventType_ == AbaSolutionFound::EVENT_TYPE) {
    auto solutionFoundEvent = (AbaSolutionFound*)event;
    auto graph = mainScene_->getItemGroup(solutionFoundEvent->solutionId_ * PROPONENT_GRAPH_ID_MULTIPLIER);
    if (graph)
      // Revert to partial solution.
      graph->setBrush(AeraGraphicsItem::Color_proponent_partial_justifications);
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

core::Timestamp AeraVisualizerWindow::aera_stepFwd() {
  // Indicate that AERA's running
  playerView_->setAERARunning();
  setCursor(QCursor(Qt::BusyCursor)); // This may take a minute

  // Run AERA a bit
  // TO DO: This only works for steps >200ms. Best guess is there's something in the interface between
  //        AERA and the Visualizer that breaks on short steps since AERA seems to do just fine with 
  //        them when run on its own.
  aera_->runFor(milliseconds(250));

  // Update everything
  updateObjectsAndEvents();
  setCursor(QCursor(Qt::ArrowCursor)); // Back to normal

  // Return the current time
  return aera_->getCurrentTime();
}

core::Timestamp AeraVisualizerWindow::aera_jumpToEnd() {
  // Indicate that AERA's running
  playerView_->setAERARunning();
  setCursor(QCursor(Qt::BusyCursor)); // This may take a minute

  // Run AERA to the end and call processEvents to keep the visualizer from freezing
  while (aera_->step())
    QApplication::processEvents();

  // Update everything
  updateObjectsAndEvents();
  setCursor(QCursor(Qt::ArrowCursor)); // Back to normal

  // Return the current time
  return aera_->getCurrentTime();
}

core::Timestamp AeraVisualizerWindow::vis_stepFwd()
{
  playerView_->stopPlay();
  size_t iNextStepEvent;
  if (getINextStepEvent(Utils_MaxTime, iNextEvent_, iNextStepEvent) == Utils_MaxTime)
    return playerView_->getAERATime();
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

  // Scroll to the current time
  scrollToTime(eventTime);

  return eventTime;
}

core::Timestamp AeraVisualizerWindow::vis_stepBack()
{
  playerView_->stopPlay();
  bool foundGraphicsItem;
  auto newTime = max(unstepEvent(Timestamp(seconds(0)), foundGraphicsItem), replicodeObjects_.getTimeReference());
  if (newTime == Utils_MaxTime)
    return replicodeObjects_.getTimeReference();
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

  // Scroll to the current time
  scrollToTime(max(newTime, replicodeObjects_.getTimeReference()));

  return max(newTime, replicodeObjects_.getTimeReference());
}

core::Timestamp AeraVisualizerWindow::vis_jumpToStart() {
  // This only works if we have a time reference to jump to
  if (!replicodeObjects_.initialized())
    return playerView_->getPlayTime();

  core::Timestamp startTime = replicodeObjects_.getTimeReference();
  core::Timestamp playTime = playerView_->getPlayTime();

  // Step until the beginning
  while (playTime > startTime)
    playTime = vis_stepBack();

  // Scroll to the current time
  scrollToTime(playTime);

  return playTime;
}

core::Timestamp AeraVisualizerWindow::vis_jumpToEnd() {
  // This only works if we have a time reference to jump to
  if (!replicodeObjects_.initialized())
    return playerView_->getPlayTime();

  core::Timestamp endTime = playerView_->getAERATime();
  core::Timestamp playTime = playerView_->getPlayTime();

  setCursor(QCursor(Qt::BusyCursor)); // This may take a minute

  // Step until the end
  while (playTime < endTime)
    playTime = vis_stepFwd();

  setCursor(QCursor(Qt::ArrowCursor)); // Back to normal

  // Scroll to the current time
  scrollToTime(playTime);

  return playTime;
}

void AeraVisualizerWindow::timerTick() {
  if (events_.size() == 0) {
    playerView_->stopPlay();
    return;
  }

  auto maximumEventTime = events_.back()->time_;
  // TODO: Make this track the passage of real clock time.
  auto playTime = playerView_->getPlayTime() + AeraVisualizer_playTimerTick;

  // Step events while events_[iNextEvent_] is less than or equal to the playTime.
  // Debug: How to step the children also?
  while (stepEvent(playTime) != Utils_MaxTime);

  if (iNextEvent_ >= events_.size()) {
    // We have played all events.
    playTime = maximumEventTime;
    playerView_->stopPlay();
  }

  playerView_->setPlayTime(playTime);
  scrollToTime(playTime);
}

void AeraVisualizerWindow::closeEvent(QCloseEvent* event) {
  findDialog_->close();

  if (aera_)
    // Shut down AERA when we're done
    aera_->stop();
  
  // Save current state for next time
  QSettings preferences;
  preferences.setValue("geometry", saveGeometry());
  preferences.setValue("state", saveState());
  
  event->accept();
}

void AeraVisualizerWindow::loadNewSeed()
{ // TO DO: This is temporarily disabled until the user can select seed programs directly.
  //        For that to work, we'll need to finish the work on the settings-GUI branch
  /*
  // Try and retrieve the last settings file loaded (fall back to the local one)
  QSettings preferences;
  QString settingsFilePath0 = preferences.value("settingsFilePath").toString();
  if (settingsFilePath0 == "")
    settingsFilePath0 = "./settings.xml";

  // Present a file dialog to the user so they can choose a settings file
  QString settingsFilePath = QFileDialog::getOpenFileName(NULL,
    "Open AERA settings XML file", settingsFilePath0, "XML Files (*.xml);;All Files (*.*)");
  if (settingsFilePath == "")
    return;
  else
    preferences.setValue("settingsFilePath", settingsFilePath);
    */
  QString settingsFilePath = "settings.xml";

  // Load the settings
  Settings settings;
  if (!settings.load(settingsFilePath.toStdString().c_str())) {
    QMessageBox::information(NULL, "XML Error", "Cannot load XML file " + settingsFilePath, QMessageBox::Ok);
    return;
  }

  // Put the filename in the title
  setWindowTitle(QString("AERA Visualizer (EXPERIMENTAL) - ") + QFileInfo(settings.source_file_name_.c_str()).fileName());

  // Show a dialog while AERA starts (it may hang a bit on the TCP I/O device)
  QProgressDialog progress(this);
  progress.setWindowTitle("Please wait");
  progress.setWindowIcon(QIcon(":/images/app.ico"));
  progress.setMinimum(0);
  progress.setMaximum(100);

  if (settings.io_device_ == "tcp_io_device") {
    progress.setWindowTitle("Standing by");
    progress.setLabelText("Waiting for a TCP connection, please start external program...");
  }
  else
    progress.setLabelText("Starting AERA, please wait...");

  progress.show();
  QApplication::processEvents();

  // Reset AERA
  aera_ = new AERA_interface(settingsFilePath.toStdString().c_str(), "");
  
  // Clear the progress dialog
  progress.setValue(100);
    
  // Files are relative to the directory of settingsFilePath.
  QDir settingsFileDir = QFileInfo(settingsFilePath).dir();
  string runtimeOutputFilePath = settingsFileDir.absoluteFilePath(settings.runtime_output_file_path_.c_str()).toStdString();
  {
    // Test opening the file now so we can exit on error.
    ifstream testOpen(runtimeOutputFilePath);
    if (!testOpen) {
      QMessageBox::information(NULL, "File Error",
        QString("Can't open debug stream output file: ") + runtimeOutputFilePath.c_str(), QMessageBox::Ok);
      return;
    }
  }

  // Create replicodeObjects_ but don't initialize it
  replicodeObjects_ = ReplicodeObjects();
  findDialog_->setReplicodeObjects(&replicodeObjects_);

  // Send this to the text output so it can read in the outputs
  textOutputView_->setOutputFilepaths(settings.decompilation_file_path_, settings.runtime_output_file_path_);

  // Update internal environment view with a link to AERA so it can access TestMem
  taskEnvironmentView_->setAERA(aera_);

  // This version isn't resettable just yet
  newInstanceAction_->setEnabled(false);
  openOutputAction_->setEnabled(false);

  // Enable the UI now that there's something to analyze
  setUIEnabled(true);

  // Indicate that AERA is started
  setAERAstatus("AERA running", false);
}

void AeraVisualizerWindow::updateObjectsAndEvents(bool live)
{
  // Create the progress dialog to show while compiling and reading the runtime output.
  QProgressDialog progress("", "Cancel", 0, 100);
  progress.setWindowModality(Qt::WindowModal);
  // Remove the '?' in the title.
  progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  progress.setWindowIcon(QIcon(":/images/app.ico"));
  progress.setWindowTitle("Initializing");
  progress.setAutoReset(false);
  progress.setAutoClose(false);
  progress.show();
  QApplication::processEvents();
  
  // If connected to a live instance, retrieve settings_ and replicodeObjects_ from there
  if (live) {
    settings_ = *aera_->getSettings();
    string error = replicodeObjects_.init(aera_, microseconds(settings_.base_period_), progress);
    if (error == "cancel")
      return;
    if (error != "") {
      QMessageBox::information(NULL, "Compiler Error", error.c_str(), QMessageBox::Ok);
      return;
    }
  }

  // Otherwise, settings_ and replicodeObjects_ should have already been filled by openOutput
  else {
    // TODO: Might be best to validate settings_ and replicodeObjects_ just in case?
  }
  
  QSettings preferences;
  // This was already set by openOutput.
  QString settingsFilePath = preferences.value("settingsFilePath").toString();
  // Files are relative to the directory of settingsFilePath.
  QDir settingsFileDir = QFileInfo(settingsFilePath).dir();
  // Process runtime_out.txt for events (these form the basis for graphics objects)
  if (!addEvents(settingsFileDir.absoluteFilePath(settings_.runtime_output_file_path_.c_str()).toStdString(), progress))
    return;

  // Show the last progress message
  progress.setLabelText(replicodeObjects_.getProgressLabelText("Setting up workspace"));
  QApplication::processEvents();

  // Pass on the changes
  essencePropertyObject_ = replicodeObjects_.getObject("essence");
  explanationLogView_->setReplicodeObjects(&replicodeObjects_);
  semanticsView_->setReplicodeObjects(&replicodeObjects_);
  playerView_->setTimeReference(replicodeObjects_.getTimeReference());
  playerView_->setPlayTime(replicodeObjects_.getTimeReference());
  findDialog_->setReplicodeObjects(&replicodeObjects_);
  mainScene_->setReplicodeObjects(&replicodeObjects_);
  
  addStartupItems();

  // Some changes only matter during a live run
  if (live) {
    playerView_->setRunTime(milliseconds(settings_.run_time_));
    taskEnvironmentView_->refresh();
    aera_->brainDump(&replicodeObjects_.getObjectLabelMap());   // Some views require the text outputs
  }
  
  textOutputView_->refresh();

  // Clean up
  progress.close();
}

void AeraVisualizerWindow::openOutput()
{
  // Configure QSettings to use .ini files to store settings
  QSettings::setDefaultFormat(QSettings::IniFormat);

  QSettings preferences;

  QString settingsFilePath0 = preferences.value("settingsFilePath").toString();
  if (settingsFilePath0 == "")
    settingsFilePath0 = "./settings.xml";
  QString settingsFilePath = QFileDialog::getOpenFileName(NULL, "Open AERA settings XML file", settingsFilePath0, "XML Files (*.xml);;All Files (*.*)");
  if (settingsFilePath == "")
    return;
  preferences.setValue("settingsFilePath", settingsFilePath);
  
  if (!settings_.load(settingsFilePath.toStdString().c_str())) {
    QMessageBox::information(NULL, "XML Error", "Cannot load XML file " + settingsFilePath, QMessageBox::Ok);
    return;
  }

  // Files are relative to the directory of settingsFilePath.
  QDir settingsFileDir = QFileInfo(settingsFilePath).dir();
  string runtimeOutputFilePath = settingsFileDir.absoluteFilePath(settings_.runtime_output_file_path_.c_str()).toStdString();
  
  // Test opening the file now so we can exit on error.
  ifstream testOpen(runtimeOutputFilePath);
  if (!testOpen) {
    QMessageBox::information(NULL, "File Error",
      QString("Can't open debug stream output file: ") + runtimeOutputFilePath.c_str(), QMessageBox::Ok);
    return;
  }

  // Create the progress dialog to show while compiling and reading the runtime output.
  QProgressDialog progress("", "Cancel", 0, 100);
  progress.setWindowModality(Qt::WindowModal);
  // Remove the '?' in the title.
  progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  progress.setWindowIcon(QIcon(":/images/app.ico"));
  progress.setWindowTitle("Initializing");
  progress.setAutoReset(false);
  progress.setAutoClose(false);
  progress.show();
  QApplication::processEvents();
  
  // Initialize replicodeObjects_
  string error = replicodeObjects_.init(
    settingsFileDir.absoluteFilePath(settings_.usr_class_path_.c_str()).toStdString(),
    settingsFileDir.absoluteFilePath(settings_.decompilation_file_path_.c_str()).toStdString(),
    microseconds(settings_.base_period_), progress);
  if (error == "cancel")
    return;
  if (error != "") {
    QMessageBox::information(NULL, "Compiler Error", error.c_str(), QMessageBox::Ok);
    return;
  }

  // Put the filename in the title
  setWindowTitle(QString("AERA Visualizer (EXPERIMENTAL) - ") + QFileInfo(settings_.source_file_name_.c_str()).fileName());

  // Point the text view to the right output files
  textOutputView_->setOutputFilepaths(settings_.decompilation_file_path_, settings_.runtime_output_file_path_);
  
  // Disable these to prevent (re)loading anything
  newInstanceAction_->setEnabled(false);
  openOutputAction_->setEnabled(false);

  // Enable the UI now that there's something to analyze
  setUIEnabled(true);

  // Indicate that everything's loaded
  setAERAstatus("Viewing previous AERA run", false);
  playerView_->indicatePreviousRun();
  
  // Push the data to the GUI without trying to fetch anything from AERA
  updateObjectsAndEvents(false);
}

void AeraVisualizerWindow::saveOutput()
{
  // Save everything and display a confirmation
  aera_->brainDump(&replicodeObjects_.getObjectLabelMap());
  QString decompiled_objects = QString::fromStdString(aera_->getSettings()->decompilation_file_path_);
  QString runtime_out = QString::fromStdString(aera_->getSettings()->runtime_output_file_path_);
  QMessageBox::information(this, "Success!",
    "Outut saved to \"" + decompiled_objects + "\" and \"" + runtime_out + "\"");
}

void AeraVisualizerWindow::saveMainWindowImage()
{
  auto fileName = QFileDialog::getSaveFileName(this, "Save image", QDir::homePath(), "PNG (*.png)");
  if (!fileName.isNull()) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    // Get the bounding rect including the top-left plus all AeraGraphicsItem. This excludes lines such as frame boundaries.
    QRectF boundingRect(0, 0, 100, 100);
    foreach(auto item, mainScene_->items()) {
      if (dynamic_cast<AeraGraphicsItem*>(item) && item->isVisible())
          boundingRect = boundingRect.united(item->sceneBoundingRect());
    }

    QImage image(boundingRect.size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    mainScene_->render(&painter, QRectF(), boundingRect);
    image.save(fileName, "PNG", 0);
    QApplication::restoreOverrideCursor();
  }
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

void AeraVisualizerWindow::createDockWidgets() {
  // Configure docking settings
  setDockNestingEnabled(true);
  setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

  // Set up the semantics view
  semanticsView_ = new SemanticsView(this);
  semanticsView_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
  semanticsView_->setObjectName("SemanticsView");
  addDockWidget(Qt::LeftDockWidgetArea, semanticsView_);
  
  // Set up the explanation log
  explanationLogView_ = new ExplanationLogView(this);
  explanationLogView_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
  explanationLogView_->setObjectName("ExplanationLog");
  addDockWidget(Qt::RightDockWidgetArea, explanationLogView_);

  // Set up the text view
  textOutputView_ = new TextOutputView(this);
  textOutputView_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
  textOutputView_->setObjectName("TextOutputView");
  addDockWidget(Qt::RightDockWidgetArea, textOutputView_);

  // Set up the internal environment view
  taskEnvironmentView_ = new TaskEnvironmentView(this);
  taskEnvironmentView_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
  taskEnvironmentView_->setObjectName("TaskEnvironmentView");
  addDockWidget(Qt::LeftDockWidgetArea, taskEnvironmentView_);

  // Make the player a fixed dock widget so it's always at the bottom of the window
  playerView_ = new PlayerView(this);
  playerView_->setFeatures(QDockWidget::NoDockWidgetFeatures);
  playerView_->setObjectName("PlayerView");
  addDockWidget(Qt::BottomDockWidgetArea, playerView_);
}

void AeraVisualizerWindow::createActions()
{
  // TO DO: This should allow the user to select a seed .replicode file
  //newInstanceAction_ = new QAction(tr("&Load seed program"), this);
  newInstanceAction_ = new QAction(tr("&Start AERA"), this);
  newInstanceAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
  //newInstanceAction_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  newInstanceAction_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  connect(newInstanceAction_, SIGNAL(triggered()), this, SLOT(loadNewSeed()));

  openOutputAction_ = new QAction(tr("&Open AERA Output"), this);
  openOutputAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
  openOutputAction_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  connect(openOutputAction_, SIGNAL(triggered()), this, SLOT(openOutput()));

  saveOutputAction_ = new QAction(tr("&Save AERA Output"), this);
  saveOutputAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
  saveOutputAction_->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  connect(saveOutputAction_, SIGNAL(triggered()), this, SLOT(saveOutput()));
  saveMainWindowImageAction_ = new QAction(tr("&Save Main Window Image"), this);
  connect(saveMainWindowImageAction_, SIGNAL(triggered()), this, SLOT(saveMainWindowImage()));

  exitAction_ = new QAction(tr("E&xit"), this);
  exitAction_->setShortcuts(QKeySequence::Quit);
  exitAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
  exitAction_->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  resetAERAInstanceAction_ = new QAction(tr("&Reset AERA Instance"), this);
  resetAERAInstanceAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
  resetAERAInstanceAction_->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
  resetAERAInstanceAction_->setEnabled(false);
  //connect(resetAERAInstanceAction_, SIGNAL(triggered()), this, SLOT(close()));

  configureAERAInstanceAction_ = new QAction(tr("&Configure AERA Instance"), this);
  configureAERAInstanceAction_->setIcon(style()->standardIcon(QStyle::SP_FileDialogInfoView));
  configureAERAInstanceAction_->setEnabled(false);
  //connect(configureAERAInstanceAction_, SIGNAL(triggered()), this, SLOT(close()));

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
  // Reset the menu so we can add more actions
  menuBar()->clear();

  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newInstanceAction_);
  fileMenu->addAction(openOutputAction_);
  fileMenu->addAction(saveOutputAction_);
  fileMenu->addAction(saveMainWindowImageAction_);
  fileMenu->addAction(exitAction_);

  // These are turned off until they're fully implemented
  //QMenu* AERAMenu = menuBar()->addMenu(tr("&AERA"));
  //AERAMenu->addAction(resetAERAInstanceAction_);
  //AERAMenu->addAction(configureAERAInstanceAction_);

  QMenu* viewMenu = menuBar()->addMenu(tr("&Views"));
  viewMenu->addAction(findAction_);
  viewMenu->addAction(explanationLogView_->toggleViewAction());
  viewMenu->addAction(semanticsView_->toggleViewAction());
  viewMenu->addAction(textOutputView_->toggleViewAction());
  viewMenu->addAction(taskEnvironmentView_->toggleViewAction());

  QMenu* findMenu = menuBar()->addMenu(tr("Fin&d"));
  findMenu->addAction(findAction_);
  findMenu->addAction(findNextAction_);
  findMenu->addAction(findPrevAction_);
  findMenu->addAction(fitAllAction_);
}

void AeraVisualizerWindow::createToolbars()
{
  timelineControls_ = new QToolBar(this); //addToolBar(tr("Main"));
  timelineControls_->addAction(zoomHomeAction_);
  timelineControls_->addAction(zoomInAction_);
  timelineControls_->addAction(zoomOutAction_);
  timelineControls_->addAction(findAction_);
  timelineControls_->setIconSize(QSize(16, 16));

  timelineControls_->addSeparator();
  // Checkbox for auto scroll
  timelineControls_->addWidget(new AeraCheckbox("Auto-scroll", SettingsKeyAutoScroll, this));

  timelineControls_->addSeparator();
  timelineControls_->addWidget(new QLabel("Show/Hide: ", this));

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
  timelineControls_->addWidget(simulationsCheckBox_);

  allSimulationInputsCheckBox_ = new AeraCheckbox("All Inputs", SettingsKeyAllSimulationInputsVisible, this, Qt::Unchecked);
  allSimulationInputsCheckBox_->setColor(simulationColor);
  timelineControls_->addWidget(allSimulationInputsCheckBox_);

  singleStepSimulationCheckBox_ = new AeraCheckbox("Single Step", SettingsKeySingleStepSimulationVisible, this, Qt::Unchecked);
  singleStepSimulationCheckBox_->setColor(simulationColor);
  timelineControls_->addWidget(singleStepSimulationCheckBox_);

  // Separate the non-simulations check boxes.
  timelineControls_->addWidget(new QLabel("    ", this));

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
  timelineControls_->addWidget(nonSimulationsCheckBox_);

  essenceFactsCheckBox_ = new AeraCheckbox("Essence Facts", SettingsKeyEssenceFactsVisible, this);
  connect(essenceFactsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setAutoFocusItemsVisible("essence", state == Qt::Checked);  });
  timelineControls_->addWidget(essenceFactsCheckBox_);

  instantiatedCompositeStatesCheckBox_ = new AeraCheckbox("Instantiated Comp. States", SettingsKeyInstantiatedCompositeStatesVisible, this);
  connect(instantiatedCompositeStatesCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewInstantiatedCompositeStateEvent::EVENT_TYPE, state == Qt::Checked); });
  timelineControls_->addWidget(instantiatedCompositeStatesCheckBox_);

  instantiatedModelsCheckBox_ = new AeraCheckbox("Instantiated Models", SettingsKeyInstantiatedModelsVisible, this);
  connect(instantiatedModelsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewInstantiatedModelEvent::EVENT_TYPE, state == Qt::Checked); });
  timelineControls_->addWidget(instantiatedModelsCheckBox_);

  predictedInstantiatedCompositeStatesCheckBox_ = new AeraCheckbox("Pred. Instantiated Comp. States", SettingsKeyPredictedInstantiatedCompositeStatesVisible, this);
  connect(predictedInstantiatedCompositeStatesCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(NewPredictedInstantiatedCompositeStateEvent::EVENT_TYPE, state == Qt::Checked); });
  timelineControls_->addWidget(predictedInstantiatedCompositeStatesCheckBox_);

  requirementsCheckBox_ = new AeraCheckbox("Requirements", SettingsKeyRequirementsVisible, this);
  connect(requirementsCheckBox_, &QCheckBox::stateChanged, [=](int state) {
    mainScene_->setItemsVisible(ModelImdlPredictionEvent::EVENT_TYPE, state == Qt::Checked);  });
  timelineControls_->addWidget(requirementsCheckBox_);
}


void AeraVisualizerWindow::createStatusBar() {
  AERAStatusLabel_ = new QLabel(this);
  operatingModeStatusLabel_ = new QLabel(this);
  QStatusBar* mainWindowStatusBar = new QStatusBar(this);

  mainWindowStatusBar->insertPermanentWidget(0, AERAStatusLabel_, 1);            // Left side
  mainWindowStatusBar->insertPermanentWidget(1, operatingModeStatusLabel_, 1);   // Right side
  setStatusBar(mainWindowStatusBar);
}


void AeraVisualizerWindow::setUIEnabled(bool enabled) {
  // Actions
  saveOutputAction_->setEnabled(enabled);
  resetAERAInstanceAction_->setEnabled(enabled);
  configureAERAInstanceAction_->setEnabled(enabled);
  zoomInAction_->setEnabled(enabled);
  zoomOutAction_->setEnabled(enabled);
  zoomHomeAction_->setEnabled(enabled);
  findAction_->setEnabled(enabled);
  findNextAction_->setEnabled(enabled);
  findPrevAction_->setEnabled(enabled);
  fitAllAction_->setEnabled(enabled);

  // Checkboxes
  simulationsCheckBox_->setEnabled(enabled);
  allSimulationInputsCheckBox_->setEnabled(enabled);
  singleStepSimulationCheckBox_->setEnabled(enabled);
  nonSimulationsCheckBox_->setEnabled(enabled);
  essenceFactsCheckBox_->setEnabled(enabled);
  instantiatedCompositeStatesCheckBox_->setEnabled(enabled);
  instantiatedModelsCheckBox_->setEnabled(enabled);
  predictedInstantiatedCompositeStatesCheckBox_->setEnabled(enabled);
  requirementsCheckBox_->setEnabled(enabled);

  // Views
  playerView_->setUIEnabled(enabled);
}

void AeraVisualizerWindow::abaSentenceItemClicked(AbaSentenceItem* item)
{
  //auto response = abagraph_.readResponse("abc");
}

}
