#include <fstream>
#include "submodules/replicode/r_exec/opcodes.h"
#include "graphics-items/arrow.hpp"
#include "graphics-items/model-item.hpp"
#include "graphics-items/composite-state-item.hpp"
#include "graphics-items/program-reduction-item.hpp"
#include "graphics-items/auto-focus-fact-item.hpp"
#include "graphics-items/prediction-item.hpp"
#include "graphics-items/prediction-success-fact-item.hpp"
#include "graphics-items/instantiated-composite-state-item.hpp"
#include "graphics-items/environment-inject-eject-item.hpp"
#include "graphics-items/aera-visualizer-scene.hpp"
#include "aera-visualizer-window.hpp"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

AeraVisulizerWindow::AeraVisulizerWindow(ReplicodeObjects& replicodeObjects)
: AeraVisulizerWindowBase(0),
  replicodeObjects_(replicodeObjects), iNextEvent_(0), explanationLogWindow_(0),
  hoverHighlightObject_(0),
  itemBorderHighlightPen_(Qt::blue, 3)
{
  createActions();
  createMenus();

  string consoleOutputFilePath = "C:\\Users\\Jeff\\AERA\\replicode\\Test\\debug_out.txt";

  setTimeReference(replicodeObjects_.getTimeReference());

  createToolbars();

  modelsScene_ = new AeraVisualizerScene(replicodeObjects_, this, false,
    [=]() { selectedScene_ = modelsScene_; });
  auto modelsSceneView = new QGraphicsView(modelsScene_, this);
  modelsSceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  modelsSceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  mainScene_ = new AeraVisualizerScene(replicodeObjects_, this, true,
    [=]() { selectedScene_ = mainScene_; });
  auto mainSceneView = new QGraphicsView(mainScene_, this);
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

  addEvents(consoleOutputFilePath);
}

void AeraVisulizerWindow::addEvents(const string& consoleOutputFilePath)
{
  ifstream consoleOutputFile(consoleOutputFilePath);
  // 0s:200ms:0us -> mdl 53, MDLController(389)
  regex newModelRegex("^(\\d+)s:(\\d+)ms:(\\d+)us -> mdl (\\d+), MDLController\\((\\d+)\\)$");
  // 0s:300ms:0us mdl 53 cnt:2 sr:1
  regex setEvidenceCountAndSuccessRateRegex("^(\\d+)s:(\\d+)ms:(\\d+)us mdl (\\d+) cnt:(\\d+) sr:([\\d\\.]+)$");
  // 0s:200ms:0us -> cst 52, CSTController(375)
  regex newCompositeStateRegex("^(\\d+)s:(\\d+)ms:(\\d+)us -> cst (\\d+), CSTController\\((\\d+)\\)$");
  // 0s:100ms:0us PGMController(92) -> mk.rdx 47
  regex programReductionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us PGMController\\((\\d+)\\) -> mk.rdx (\\d+)$");
  // 0s:0ms:0us A/F -> 35|40 (AXIOM)
  regex autofocusNewObjectRegex("^(\\d+)s:(\\d+)ms:(\\d+)us A/F -> (\\d+)\\|(\\d+) \\((\\w+)\\)$");
  // 0s:200ms:0us fact 61(1084) imdl mdl 53: 56 -> fact 60 pred fact mk.val VALUE nb: 2.000000e+01
  regex newMkValPredictionRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact (\\d+)\\(\\d+\\) imdl mdl \\d+: (\\d+) -> fact (\\d+) pred fact mk.val VALUE .+$");
  // 0s:200ms:0us fact 59 icst[52][ 50 55]
  regex newInstantiatedCompositeStateRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact (\\d+) icst\\[\\d+\\]\\[([ \\d]+)\\]$");
  // 0s:300ms:0us fact 75 -> fact 79 success fact 60 pred
  regex newPredictionSuccessRegex("^(\\d+)s:(\\d+)ms:(\\d+)us fact (\\d+) -> fact (\\d+) success fact \\d+ pred$");
  // 0s:200ms:0us environment inject 46, ijt 0s:200ms:0us
  regex newEnvironmentInjectRegex("^(\\d+)s:(\\d+)ms:(\\d+)us environment inject (\\d+), ijt (\\d+)s:(\\d+)ms:(\\d+)us$");
  // 0s:100ms:0us environment eject 44
  regex newEnvironmentEjectRegex("^(\\d+)s:(\\d+)ms:(\\d+)us environment eject (\\d+)$");

  string line;
  while (getline(consoleOutputFile, line)) {
    smatch matches;

    if (regex_search(line, matches, newModelRegex)) {
      auto model = replicodeObjects_.getObject(stol(matches[4].str()));
      if (model)
        // Assume the initial success rate is 1.
        events_.push_back(make_shared<NewModelEvent>(
          getTimestamp(matches), model, 1, 1, stoll(matches[5].str())));
    }
    else if (regex_search(line, matches, setEvidenceCountAndSuccessRateRegex)) {
      auto model = replicodeObjects_.getObject(stol(matches[4].str()));
      if (model)
        events_.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(
          getTimestamp(matches), model, stol(matches[5].str()), stof(matches[6].str())));
    }
    else if (regex_search(line, matches, newCompositeStateRegex)) {
      auto compositeState = replicodeObjects_.getObject(stol(matches[4].str()));
      if (compositeState)
        events_.push_back(make_shared<NewCompositeStateEvent>(
          getTimestamp(matches), compositeState, stoll(matches[5].str())));
    }
    else if (regex_search(line, matches, programReductionRegex)) {
      auto programReduction = replicodeObjects_.getObject(stol(matches[5].str()));
      if (programReduction) {
        // First add events for the output objects.
        // TODO: Get this from the actual object.
        Code* outputObject = 0;
        if (programReduction->get_oid() == 47)
          outputObject = replicodeObjects_.getObject(49);
        else if (programReduction->get_oid() == 58)
          outputObject = replicodeObjects_.getObject(68);
        else if (programReduction->get_oid() == 77)
          outputObject = replicodeObjects_.getObjectByDebugOid(2061);
        if (outputObject)
          events_.push_back(make_shared<ProgramReductionNewObjectEvent>(
            getTimestamp(matches), outputObject, programReduction));

        events_.push_back(make_shared<ProgramReductionEvent>(
          getTimestamp(matches), programReduction, stoll(matches[4].str())));
      }
    }
    else if (regex_search(line, matches, autofocusNewObjectRegex)) {
      auto fromObject = replicodeObjects_.getObject(stol(matches[4].str()));
      auto toObject = replicodeObjects_.getObject(stol(matches[5].str()));
      // Skip auto-focus of the same object (such as eject facts).
      if (fromObject && toObject && fromObject != toObject)
        events_.push_back(make_shared<AutoFocusNewObjectEvent>(
          getTimestamp(matches), fromObject, toObject, matches[6].str()));
    }
    else if (regex_search(line, matches, newMkValPredictionRegex)) {
      // TODO: Use the mk.rdx for the prediction.
      auto factImdl = replicodeObjects_.getObject(stol(matches[4].str()));
      auto cause = replicodeObjects_.getObject(stol(matches[5].str()));
      auto factPrediction = replicodeObjects_.getObject(stol(matches[6].str()));
      if (factImdl && cause && factPrediction)
        events_.push_back(make_shared<NewMkValPredictionEvent>(
          getTimestamp(matches), factPrediction, factImdl, cause));
    }
    else if (regex_search(line, matches, newInstantiatedCompositeStateRegex)) {
      auto timestamp = getTimestamp(matches);
      auto instantiatedCompositeState = replicodeObjects_.getObject(stol(matches[4].str()));

      // Get the matching inputs.
      string inputOids = matches[5].str();
      std::vector<Code*> inputs;
      bool gotAllInputs = true;
      while (regex_search(inputOids, matches, regex("( \\d+)"))) {
        auto input = replicodeObjects_.getObject(stol(matches[1].str()));
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
    else if (regex_search(line, matches, newPredictionSuccessRegex)) {
      auto factSuccessFactPred = replicodeObjects_.getObject(stol(matches[5].str()));
      if (factSuccessFactPred)
        events_.push_back(make_shared<NewPredictionSuccessEvent>(
          getTimestamp(matches), factSuccessFactPred));
    }
    else if (regex_search(line, matches, newEnvironmentInjectRegex)) {
      auto object = replicodeObjects_.getObject(stol(matches[4].str()));
      if (object)
        events_.push_back(make_shared<EnvironmentInjectEvent>(
          getTimestamp(matches), object, getTimestamp(matches, 5)));
    }
    else if (regex_search(line, matches, newEnvironmentEjectRegex)) {
      auto object = replicodeObjects_.getObject(stol(matches[4].str()));
      if (object)
        events_.push_back(make_shared<EnvironmentEjectEvent>(getTimestamp(matches), object));
    }
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
  if (item)
    scene->zoomToItem(item);
}

void AeraVisulizerWindow::setAeraGraphicsItemPen(r_code::Code* object, const QPen& pen)
{
  auto item = getAeraGraphicsItem(object);
  if (item)
    item->setPen(pen);
}

void AeraVisulizerWindow::resetAeraGraphicsItemPen(r_code::Code* object)
{
  auto item = getAeraGraphicsItem(object);
  if (item)
    item->setPen(item->getBorderNoHighlightPen());
}

void AeraVisulizerWindow::textItemHoverMoveEvent(const QTextDocument* document, QPointF position)
{
  auto url = document->documentLayout()->anchorAt(position);
  if (url == "") {
    // The mouse cursor exited the link.
    if (hoverHighlightObject_) {
      // Clear the previous highlighting.
      resetAeraGraphicsItemPen(hoverHighlightObject_);
      hoverHighlightObject_ = 0;
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
      if (hoverHighlightObject_)
        // Unhighlight a previous object.
        resetAeraGraphicsItemPen(hoverHighlightObject_);

      hoverHighlightObject_ = object;
      setAeraGraphicsItemPen(object, itemBorderHighlightPen_);
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
      event->eventType_ == NewMkValPredictionEvent::EVENT_TYPE ||
      event->eventType_ == NewPredictionSuccessEvent::EVENT_TYPE ||
      event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE ||
      event->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ||
      event->eventType_ == EnvironmentEjectEvent::EVENT_TYPE) {
    AeraGraphicsItem* newItem;

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
      // TODO: Make this a Show/Hide option.
      auto fromObjectItem = scene->getAeraGraphicsItem(autoFocusEvent->fromObject_);
      if (fromObjectItem)
        scene->addArrow(newItem, fromObjectItem);
    }
    else if (event->eventType_ == NewMkValPredictionEvent::EVENT_TYPE)
      newItem = new PredictionItem((NewMkValPredictionEvent*)event, replicodeObjects_, scene);
    else if (event->eventType_ == NewPredictionSuccessEvent::EVENT_TYPE)
      newItem = new PredictionSuccessFactItem((NewPredictionSuccessEvent*)event, replicodeObjects_, scene);
    else if (event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE) {
      auto newIcstEvent = (NewInstantiatedCompositeStateEvent*)event;
      newItem = new InstantiatedCompositeStateItem(newIcstEvent, replicodeObjects_, scene);

      // Add arrows to inputs.
      for (int i = 0; i < newIcstEvent->inputs_.size(); ++i) {
        auto referencedItem = scene->getAeraGraphicsItem(newIcstEvent->inputs_[i]);
        if (referencedItem)
          scene->addArrow(newItem, referencedItem);
      }
    }
    else if (event->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ||
             event->eventType_ == EnvironmentEjectEvent::EVENT_TYPE)
      // TODO: Position the EnvironmentInjectEvent at its injectionTime?
      newItem = new EnvironmentInjectEjectItem(event, replicodeObjects_, scene);

    // Add the new item.
    scene->addAeraGraphicsItem(newItem);

    // Add arrows to all referenced objects.
    for (int i = 0; i < event->object_->references_size(); ++i) {
      auto referencedItem = scene->getAeraGraphicsItem(event->object_->get_reference(i));
      if (referencedItem)
        scene->addArrow(newItem, referencedItem);
    }
    if (event->object_->code(0).asOpcode() == Opcodes::Fact ||
        event->object_->code(0).asOpcode() == Opcodes::AntiFact) {
      // Add references from the fact value.
      auto value = event->object_->get_reference(0);
      if (!(value->code(0).asOpcode() == Opcodes::IMdl || value->code(0).asOpcode() == Opcodes::ICst)) {
        for (int i = 0; i < value->references_size(); ++i) {
          auto referencedItem = scene->getAeraGraphicsItem(value->get_reference(i));
          if (referencedItem)
            scene->addArrow(newItem, referencedItem);
        }
      }
    }

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
      event->eventType_ == NewMkValPredictionEvent::EVENT_TYPE ||
      event->eventType_ == NewPredictionSuccessEvent::EVENT_TYPE ||
      event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE ||
      event->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ||
      event->eventType_ == EnvironmentEjectEvent::EVENT_TYPE) {
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
      aeraGraphicsItem->removeArrows();
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
}

}
