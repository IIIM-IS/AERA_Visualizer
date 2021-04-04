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
#include "graphics-items/composite-state-prediction-item.hpp"
#include "graphics-items/prediction-result-item.hpp"
#include "graphics-items/instantiated-composite-state-item.hpp"
#include "graphics-items/io-device-inject-eject-item.hpp"
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
  simulationEventTypes_ = {
    ModelGoalReduction::EVENT_TYPE, CompositeStateGoalReduction::EVENT_TYPE,
    ModelSimulatedPredictionReduction::EVENT_TYPE, CompositeStateSimulatedPredictionReduction::EVENT_TYPE };

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
  regex loadModelRegex("^load mdl (\\d+), MDLController\\((\\d+)\\)$");
  // load cst 36, CSTController(98)
  regex loadCompositeStateRegex("^load cst (\\d+), CSTController\\((\\d+)\\)$");
  // 0s:200ms:0us -> mdl 53, MDLController(389)
  regex newModelRegex("^(\\d+)s:(\\d+)ms:(\\d+)us -> mdl (\\d+), MDLController\\((\\d+)\\)$");
  // 0s:300ms:0us mdl 53 cnt:2 sr:1
  regex setEvidenceCountAndSuccessRateRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mdl (\\d+) cnt:(\\d+) sr:([\\d\\.]+)$");
  // 0s:200ms:0us -> cst 52, CSTController(375)
  regex newCompositeStateRegex("^(\\d+)s:(\\d+)ms:(\\d+)us -> cst (\\d+), CSTController\\((\\d+)\\)$");
  // 0s:0ms:0us A/F -> 35|40 (AXIOM)
  regex autofocusNewObjectRegex("^(\\d+)s:(\\d+)ms:(\\d+)us A/F -> (\\d+)\\|(\\d+) \\((\\w+)\\)$");
  // 0s:300ms:0us fact (1022) imdl mdl 59: 60 -> fact (1029) pred fact (1025) imdl mdl 58
  regex modelImdlPredictionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact \\(\\d+\\) imdl mdl (\\d+): (\\d+) -> fact \\((\\d+)\\) pred fact \\(\\d+\\) imdl mdl \\d+$");
  // 0s:300ms:0us mdl 63 predict -> mk.rdx 68
  regex modelPredictionReductionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mdl \\d+ predict -> mk.rdx (\\d+)$");
  // 0s:510ms:0us mdl 41 abduce -> mk.rdx 97
  regex modelAbductionReductionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mdl \\d+ abduce -> mk.rdx (\\d+)$");
  // 0s:510ms:0us mdl 64: fact 96 super_goal -> fact 98 simulated goal
  regex modelSimulatedAbductionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mdl (\\d+): fact (\\d+) super_goal -> fact (\\d+) simulated goal$");
  // 0s:510ms:0us cst 64: fact 96 super_goal -> fact 98 simulated goal
  regex compositeStateSimulatedAbductionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us cst (\\d+): fact (\\d+) super_goal -> fact (\\d+) simulated goal$");
  // 0s:210ms:0us mdl 57: fact 202 pred -> fact 227 simulated pred
  regex modelSimulatedPredictionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mdl (\\d+): fact (\\d+) (?:pred|super_goal) -> fact (\\d+) simulated pred$");
  // 0s:210ms:0us cst 60: fact 195 -> fact 218 simulated pred
  regex compositeStateSimulatedPredictionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us cst (\\d+): fact (\\d+) -> fact (\\d+) simulated pred$");
  // 0s:200ms:0us fact 59 icst[52][ 50 55]
  regex newInstantiatedCompositeStateRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact (\\d+) icst\\[\\d+\\]\\[([ \\d]+)\\]$");
  // 0s:300ms:0us fact 75 -> fact 79 success fact 60 pred
  regex predictionSuccessRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact (\\d+) -> fact (\\d+) success fact \\d+ pred$");
  // 0s:322ms:933us |fact 72 fact 59 pred failure
  regex predictionFailureRegex("^(\\d+)s:(\\d+)ms:(\\d+)us \\|fact (\\d+) fact \\d+ pred failure$");
  // 0s:600ms:0us fact 121: 96 goal success (TopLevel)
  regex topLevelGoalSuccessRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact (\\d+): (\\d+) goal success \\(TopLevel\\)$");
  // 0s:200ms:0us I/O device inject 46, ijt 0s:200ms:0us
  regex ioDeviceInjectRegex("^(\\d+)s:(\\d+)ms:(\\d+)us I/O device inject (\\d+), ijt (\\d+)s:(\\d+)ms:(\\d+)us$");
  // 0s:100ms:0us mk.rdx(100): I/O device eject 39
  regex ioDeviceEjectWithRdxRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mk.rdx\\((\\d+)\\): I/O device eject (\\d+)$");
  // 0s:100ms:0us I/O device eject 39
  regex ioDeviceEjectWithoutRdxRegex("^(\\d+)s:(\\d+)ms:(\\d+)us I/O device eject (\\d+)$");

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
        // Assume the initial success rate is 1.
        auto event = new NewModelEvent(
          replicodeObjects_.getTimeReference(), model, 1, 1, stoll(matches[2].str()));
        // Assume it is loaded before run time starts, so show now.
        modelsScene_->addAeraGraphicsItem(new ModelItem(event, replicodeObjects_, modelsScene_));
        // TODO: Add arrows.
      }
    }
    else if (regex_search(line, matches, loadCompositeStateRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[1].str()));
      if (compositeState) {
        auto event = new NewCompositeStateEvent(
          replicodeObjects_.getTimeReference(), compositeState, stoll(matches[2].str()));
        // Assume it is loaded before run time starts, so show now.
        modelsScene_->addAeraGraphicsItem(new CompositeStateItem(event, replicodeObjects_, modelsScene_));
        // TODO: Add arrows.
      }
    }
    else if (regex_search(line, matches, newModelRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (model)
        // Assume the initial success rate is 1.
        events_.push_back(make_shared<NewModelEvent>(
          getTimestamp(matches), model, 1, 1, stoll(matches[5].str())));
    }
    else if (regex_search(line, matches, setEvidenceCountAndSuccessRateRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (model)
        events_.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(
          getTimestamp(matches), model, stol(matches[5].str()), stof(matches[6].str())));
    }
    else if (regex_search(line, matches, newCompositeStateRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (compositeState)
        events_.push_back(make_shared<NewCompositeStateEvent>(
          getTimestamp(matches), compositeState, stoll(matches[5].str())));
    }
    else if (regex_search(line, matches, autofocusNewObjectRegex)) {
      auto fromObject = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto toObject = replicodeObjects_.getObject(stoul(matches[5].str()));
      // Skip auto-focus of the same fact (such as eject facts).
      // But show auto-focus of the same anti-fact (such as prediction failure).
      if (fromObject && toObject && !(fromObject == toObject && fromObject->code(0).asOpcode() == Opcodes::Fact))
        events_.push_back(make_shared<AutoFocusNewObjectEvent>(
          getTimestamp(matches), fromObject, toObject, matches[6].str()));
    }
    else if (regex_search(line, matches, modelImdlPredictionRegex)) {
      // TODO: When Replicode makes an mk.rdx for the prediction, use it.
      auto predictingModel = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto cause = replicodeObjects_.getObject(stoul(matches[5].str()));
      auto factPred = replicodeObjects_.getObjectByDebugOid(stoul(matches[6].str()));
      if (predictingModel && cause && factPred)
        events_.push_back(make_shared<ModelImdlPredictionEvent>(
          getTimestamp(matches), factPred, predictingModel, cause));
    }
    else if (regex_search(line, matches, modelPredictionReductionRegex)) {
      auto reduction = replicodeObjects_.getObject(stoul(matches[4].str()));
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
            getTimestamp(matches), reduction, imdlPredictionEventIndex));
        }
      }
    }
    else if (regex_search(line, matches, modelAbductionReductionRegex)) {
      auto reduction = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (reduction) {
        auto factImdl = reduction->get_reference(MK_RDX_IHLP_REF);
        auto model = factImdl->get_reference(0)->get_reference(0);
        // The goal is the first (only) item in the set of productions.
        auto factGoal = AeraEvent::getFirstProduction(reduction);
        // The super goal is the first item in the set of inputs.
        auto factSuperGoal = reduction->get_reference(
          reduction->code(reduction->code(MK_RDX_INPUTS).asIndex() + 1).asIndex());
        events_.push_back(make_shared<ModelGoalReduction>(
          getTimestamp(matches), model, factGoal, factSuperGoal));
      }
    }
    else if (regex_search(line, matches, modelSimulatedAbductionRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto factGoal = replicodeObjects_.getObject(stoul(matches[6].str()));
      auto factSuperGoal = replicodeObjects_.getObject(stoul(matches[5].str()));
      if (model && factGoal && factSuperGoal)
        events_.push_back(make_shared<ModelGoalReduction>(
          getTimestamp(matches), model, factGoal, factSuperGoal));
    }
    else if (regex_search(line, matches, compositeStateSimulatedAbductionRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto factGoal = replicodeObjects_.getObject(stoul(matches[6].str()));
      auto factSuperGoal = replicodeObjects_.getObject(stoul(matches[5].str()));
      if (compositeState && factGoal && factSuperGoal)
        events_.push_back(make_shared<CompositeStateGoalReduction>(
          getTimestamp(matches), compositeState, factGoal, factSuperGoal));
    }
    else if (regex_search(line, matches, modelSimulatedPredictionRegex)) {
      auto model = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[6].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[5].str()));
      if (factPred && factPred->get_oid() == 264)
        // TODO: We need check_simulated_imdl to know the input which triggered the signal.
        input = replicodeObjects_.getObject(256);
      else if (factPred && factPred->get_oid() == 281)
        // TODO: We need check_simulated_imdl to know the input which triggered the signal.
        input = replicodeObjects_.getObject(275);

      if (model && factPred && input)
        events_.push_back(make_shared<ModelSimulatedPredictionReduction>(
          getTimestamp(matches), model, factPred, input));
    }
    else if (regex_search(line, matches, compositeStateSimulatedPredictionRegex)) {
      auto compositeState = replicodeObjects_.getObject(stoul(matches[4].str()));
      auto factPred = replicodeObjects_.getObject(stoul(matches[6].str()));
      auto input = replicodeObjects_.getObject(stoul(matches[5].str()));
      if (compositeState && factPred && input)
        events_.push_back(make_shared<CompositeStateSimulatedPredictionReduction>(
          getTimestamp(matches), compositeState, factPred, input));
    }
    else if (regex_search(line, matches, newInstantiatedCompositeStateRegex)) {
      auto timestamp = getTimestamp(matches);
      auto instantiatedCompositeState = replicodeObjects_.getObject(stoul(matches[4].str()));

      // Get the matching inputs.
      string inputOids = matches[5].str();
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
    else if (regex_search(line, matches, predictionSuccessRegex)) {
      auto factSuccessFactPred = replicodeObjects_.getObject(stoul(matches[5].str()));
      if (factSuccessFactPred)
        events_.push_back(make_shared<PredictionResultEvent>(
          getTimestamp(matches), factSuccessFactPred));
    }
    else if (regex_search(line, matches, predictionFailureRegex)) {
      auto antiFactSuccessFactPred = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (antiFactSuccessFactPred)
        events_.push_back(make_shared<PredictionResultEvent>(
          getTimestamp(matches), antiFactSuccessFactPred));
    }
    else if (regex_search(line, matches, topLevelGoalSuccessRegex)) {
      auto factSuccessFactGoal = replicodeObjects_.getObject(stoul(matches[4].str()));
    }
    else if (regex_search(line, matches, ioDeviceInjectRegex)) {
      auto object = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (object)
        events_.push_back(make_shared<IoDeviceInjectEvent>(
          getTimestamp(matches), object, getTimestamp(matches, 5)));
    }
    else if (regex_search(line, matches, ioDeviceEjectWithRdxRegex)) {
      auto reduction = replicodeObjects_.getObjectByDebugOid(stoul(matches[4].str()));
      auto object = replicodeObjects_.getObject(stoul(matches[5].str()));
      if (object)
        events_.push_back(make_shared<IoDeviceEjectEvent>(
          getTimestamp(matches), object, reduction));
    }
    else if (regex_search(line, matches, ioDeviceEjectWithoutRdxRegex)) {
      auto object = replicodeObjects_.getObject(stoul(matches[4].str()));
      if (object)
        events_.push_back(make_shared<IoDeviceEjectEvent>(
          getTimestamp(matches), object, (Code*)NULL));
    }
  }

  return true;
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
  if (url.startsWith("#debug_oid-")) {
    // Highlight the linked item.
    uint64 debug_oid = url.mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
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

Timestamp AeraVisulizerWindow::stepEvent(Timestamp maximumTime)
{
  if (iNextEvent_ >= events_.size())
    // Return the value meaning no change.
    return Utils_MaxTime;

  AeraEvent* event = events_[iNextEvent_].get();
  if (event->time_ > maximumTime)
    return Utils_MaxTime;

  if (event->eventType_ == NewModelEvent::EVENT_TYPE ||
      event->eventType_ == NewCompositeStateEvent::EVENT_TYPE ||
      event->eventType_ == AutoFocusNewObjectEvent::EVENT_TYPE ||
      event->eventType_ == ModelMkValPredictionReduction::EVENT_TYPE ||
      event->eventType_ == ModelGoalReduction::EVENT_TYPE ||
      event->eventType_ == CompositeStateGoalReduction::EVENT_TYPE ||
      event->eventType_ == ModelSimulatedPredictionReduction::EVENT_TYPE ||
      event->eventType_ == CompositeStateSimulatedPredictionReduction::EVENT_TYPE ||
      event->eventType_ == PredictionResultEvent::EVENT_TYPE ||
      event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE ||
      event->eventType_ == IoDeviceInjectEvent::EVENT_TYPE ||
      event->eventType_ == IoDeviceEjectEvent::EVENT_TYPE) {
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
    else if (event->eventType_ == ModelGoalReduction::EVENT_TYPE) {
      auto reductionEvent = (ModelGoalReduction*)event;
      newItem = new ModelGoalItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the fact super goal.
      auto factSuperGoalItem = scene->getAeraGraphicsItem(reductionEvent->factSuperGoal_);
      if (factSuperGoalItem)
        scene->addArrow(factSuperGoalItem, newItem);

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
        scene->addArrow(factSuperGoalItem, newItem);

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
        scene->addArrow(inputItem, newItem);

      scene->addHorizontalLine(newItem);

      if (newItem->is_sim())
        visible = (simulationsCheckBox_->checkState() == Qt::Checked);
      else
        visible = (nonSimulationsCheckBox_->checkState() == Qt::Checked);
    }
    else if (event->eventType_ == CompositeStateSimulatedPredictionReduction::EVENT_TYPE) {
      auto reductionEvent = (CompositeStateSimulatedPredictionReduction*)event;
      newItem = new CompositeStatePredictionItem(reductionEvent, replicodeObjects_, scene);

      // Add an arrow to the input fact.
      auto inputItem = scene->getAeraGraphicsItem(reductionEvent->input_);
      if (inputItem)
        scene->addArrow(inputItem, newItem);

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
  if (event->eventType_ == NewModelEvent::EVENT_TYPE ||
      event->eventType_ == NewCompositeStateEvent::EVENT_TYPE ||
      event->eventType_ == AutoFocusNewObjectEvent::EVENT_TYPE ||
      event->eventType_ == ModelMkValPredictionReduction::EVENT_TYPE ||
      event->eventType_ == ModelGoalReduction::EVENT_TYPE ||
      event->eventType_ == CompositeStateGoalReduction::EVENT_TYPE ||
      event->eventType_ == ModelSimulatedPredictionReduction::EVENT_TYPE ||
      event->eventType_ == CompositeStateSimulatedPredictionReduction::EVENT_TYPE ||
      event->eventType_ == PredictionResultEvent::EVENT_TYPE ||
      event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE ||
      event->eventType_ == IoDeviceInjectEvent::EVENT_TYPE ||
      event->eventType_ == IoDeviceEjectEvent::EVENT_TYPE) {
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
  if (stepEvent(Utils_MaxTime) == Utils_MaxTime)
    return;
  auto eventTime = events_[iNextEvent_ - 1]->time_;

  // Keep stepping remaining events in this same frame.
  auto relativeTime = duration_cast<microseconds>(eventTime - replicodeObjects_.getTimeReference());
  auto frameStartTime = eventTime - (relativeTime % replicodeObjects_.getSamplingPeriod());
  auto thisFrameMaxTime = frameStartTime + replicodeObjects_.getSamplingPeriod() - microseconds(1);
  bool isNewFrame = (iNextEvent_ <= 1 || frameStartTime > events_[iNextEvent_ - 2]->time_);
  auto firstEventTime = eventTime;
  bool firstEventIsSimulation = (simulationEventTypes_.count(events_[iNextEvent_ - 1]->eventType_) > 0);

  if (isNewFrame)
    // Remove the simulation items from the previous frame.
    // TODO: We don't expect it, but if the first event in the frame is simulated then this will erase it.
    mainScene_->removeAllItemsByEventType(simulationEventTypes_);

  while (true) {
    if (simulationsCheckBox_->isChecked()) {
      if (isNewFrame) {
        // In a new frame, advance until the next item would be a simulation item that is not at the first event time.
        if (iNextEvent_ < events_.size() && simulationEventTypes_.count(events_[iNextEvent_]->eventType_) > 0 &&
            events_[iNextEvent_]->time_ > firstEventTime)
          break;
      }
      else {
        // If not a new frame and the first event is a simulation, keep stepping until the next item would be a non-simulation.
        if (firstEventIsSimulation && iNextEvent_ < events_.size() && 
            simulationEventTypes_.count(events_[iNextEvent_]->eventType_) == 0)
          break;
      }
    }

    if (stepEvent(thisFrameMaxTime) == Utils_MaxTime)
      break;
    eventTime = events_[iNextEvent_ - 1]->time_;
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

    // Do the opposite of simulationsCheckBox_ .
    mainScene_->setNonItemsVisible(simulationEventTypes_, state == Qt::Checked);
    // Make specific non-simulation items visible or not, if needed.
    mainScene_->setAutoFocusItemsVisible("essence", essenceFactsCheckBox_->checkState() == Qt::Checked);
    mainScene_->setItemsVisible(
      NewInstantiatedCompositeStateEvent::EVENT_TYPE, instantiatedCompositeStatesCheckBox_->checkState() == Qt::Checked);
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
}

}
