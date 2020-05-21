#include <fstream>
#include "graphics-items/arrow.hpp"
#include "graphics-items/model-item.hpp"
#include "graphics-items/composite-state-item.hpp"
#include "graphics-items/program-reduction-item.hpp"
#include "graphics-items/auto-focus-fact-item.hpp"
#include "graphics-items/prediction-item.hpp"
#include "graphics-items/instantiated-composite-state-item.hpp"
#include "graphics-items/aera-visualizer-scene.hpp"
#include "aera-visualizer-window.hpp"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraVisulizerWindow::AeraVisulizerWindow(ReplicodeObjects& replicodeObjects)
: AeraVisulizerWindowBase(0),
replicodeObjects_(replicodeObjects), iNextEvent_(0), explanationLogWindow_(0)
{
  createActions();
  createMenus();

  string consoleOutputFilePath = "C:\\Users\\Jeff\\temp\\Test.out.txt";

  setTimeReference(replicodeObjects_.getTimeReference());

  scene_ = new AeraVisualizerScene(itemMenu_, replicodeObjects_, this);
  scene_->setSceneRect(QRectF(0, 0, 5000, 5000));
  connect(scene_, SIGNAL(itemInserted(AeraGraphicsItem*)),
    this, SLOT(itemInserted(AeraGraphicsItem*)));
  createToolbars();

  QVBoxLayout* centralLayout = new QVBoxLayout();
  QGraphicsView* view = new QGraphicsView(scene_, this);
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  centralLayout->addWidget(view);
  centralLayout->addWidget(getPlayerControlPanel());

  QWidget* centralWidget = new QWidget();
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
      if (fromObject && toObject)
        events_.push_back(make_shared<AutoFocusNewObjectEvent>(
          getTimestamp(matches), fromObject, toObject, matches[6].str()));
    }
    else if (regex_search(line, matches, newMkValPredictionRegex)) {
      // TODO: Use the mk.rdx for the prediction.
      auto factImdl = replicodeObjects_.getObject(stol(matches[4].str()));
      auto cause = replicodeObjects_.getObject(stol(matches[5].str()));
      auto factPrediction = replicodeObjects_.getObject(stol(matches[6].str()));
      if (factImdl && cause && factPrediction && /* Debug: Ignore others */ factPrediction->get_oid() == 60)
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
      auto inputObject = replicodeObjects_.getObject(stol(matches[4].str()));
      auto factSuccessFactPred = replicodeObjects_.getObject(stol(matches[5].str()));
      if (inputObject && factSuccessFactPred)
        events_.push_back(make_shared<NewPredictionSuccessEvent>(
          getTimestamp(matches), inputObject, factSuccessFactPred));
    }
  }
}

Timestamp AeraVisulizerWindow::getTimestamp(const smatch& matches)
{
  microseconds us(1000000 * stoll(matches[1].str()) +
                     1000 * stoll(matches[2].str()) +
                            stoll(matches[3].str()));
  return replicodeObjects_.getTimeReference() + us;
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
      event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE) {
    AeraGraphicsItem* newItem;

    if (event->eventType_ == NewModelEvent::EVENT_TYPE) {
      auto newModelEvent = (NewModelEvent*)event;

      // Restore the evidence count and success rate in case we did a rewind.
      newModelEvent->object_->code(MDL_CNT) = Atom::Float(newModelEvent->evidenceCount_);
      newModelEvent->object_->code(MDL_SR) = Atom::Float(newModelEvent->successRate_);

      newItem = new ModelItem(itemMenu_, newModelEvent, replicodeObjects_, scene_);
    }
    else if (event->eventType_ == NewCompositeStateEvent::EVENT_TYPE)
      newItem = new CompositeStateItem(itemMenu_, (NewCompositeStateEvent*)event, replicodeObjects_, scene_);
    else if (event->eventType_ == AutoFocusNewObjectEvent::EVENT_TYPE) {
      if (event->time_ == replicodeObjects_.getTimeReference()) {
        // Debug: For now, skip auto focus events at startup.
        ++iNextEvent_;
        return stepEvent(maximumTime);
      }

      newItem = new AutoFocusFactItem(itemMenu_, (AutoFocusNewObjectEvent*)event, replicodeObjects_, scene_);
    }
    else if (event->eventType_ == NewMkValPredictionEvent::EVENT_TYPE)
      newItem = new PredictionItem(itemMenu_, (NewMkValPredictionEvent*)event, replicodeObjects_, scene_);
    else if (event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE) {
      auto newIcstEvent = (NewInstantiatedCompositeStateEvent*)event;
      newItem = new InstantiatedCompositeStateItem(itemMenu_, newIcstEvent, replicodeObjects_, scene_);

      // Add arrows to inputs.
      for (int i = 0; i < newIcstEvent->inputs_.size(); ++i) {
        auto referencedItem = scene_->getAeraGraphicsItem(newIcstEvent->inputs_[i]);
        if (referencedItem)
          scene_->addArrow(newItem, referencedItem);
      }
    }

    // Add the new item.
    scene_->addAeraGraphicsItem(newItem);

    // Add arrows to all referenced objects.
    for (int i = 0; i < event->object_->references_size(); ++i) {
      auto referencedItem = scene_->getAeraGraphicsItem(event->object_->get_reference(i));
      if (referencedItem)
        scene_->addArrow(newItem, referencedItem);
    }

    scene_->establishFlashTimer();
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE) {
    auto setSuccessRateEvent = (SetModelEvidenceCountAndSuccessRateEvent*)event;

    // Save the current values for a later undo.
    setSuccessRateEvent->oldEvidenceCount_ = setSuccessRateEvent->object_->code(MDL_CNT).asFloat();
    setSuccessRateEvent->oldSuccessRate_ = setSuccessRateEvent->object_->code(MDL_SR).asFloat();

    // Update the model.
    setSuccessRateEvent->object_->code(MDL_CNT) = Atom::Float(setSuccessRateEvent->evidenceCount_);
    setSuccessRateEvent->object_->code(MDL_SR) = Atom::Float(setSuccessRateEvent->successRate_);

    auto modelItem = dynamic_cast<ModelItem*>(scene_->getAeraGraphicsItem(setSuccessRateEvent->object_));
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
      scene_->establishFlashTimer();
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
      event->eventType_ == NewInstantiatedCompositeStateEvent::EVENT_TYPE) {
    // Find the AeraGraphicsItem for this event and remove it.
    // Note that the event saves the updated item position and will use it when recreating the item.
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(scene_->getAeraGraphicsItem(event->object_));
    if (aeraGraphicsItem) {
      aeraGraphicsItem->removeArrows();
      scene_->removeItem(aeraGraphicsItem);
      delete aeraGraphicsItem;
    }
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE) {
    // Find the ModelItem for this event and set to the old evidence count and success rate.
    auto setSuccessRateEvent = (SetModelEvidenceCountAndSuccessRateEvent*)event;

    setSuccessRateEvent->object_->code(MDL_CNT) = Atom::Float(setSuccessRateEvent->oldEvidenceCount_);
    setSuccessRateEvent->object_->code(MDL_SR) = Atom::Float(setSuccessRateEvent->oldSuccessRate_);

    auto modelItem = dynamic_cast<ModelItem*>(scene_->getAeraGraphicsItem(setSuccessRateEvent->object_));
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
      scene_->establishFlashTimer();
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
  scene_->scaleViewBy(1.09);
}

void AeraVisulizerWindow::zoomOut()
{
  scene_->scaleViewBy(1 / 1.09);
}

void AeraVisulizerWindow::zoomHome()
{
  scene_->zoomViewHome();
}

void AeraVisulizerWindow::zoomToThis()
{
  if (scene_->selectedItems().isEmpty())
    return;

  if (scene_->selectedItems().size() == 1)
    scene_->zoomToItem(scene_->selectedItems().first());
}

void AeraVisulizerWindow::bringToFront()
{
  if (scene_->selectedItems().isEmpty())
    return;

  QGraphicsItem* selectedItem = scene_->selectedItems().first();
  QList<QGraphicsItem*> overlapItems = selectedItem->collidingItems();

  qreal zValue = 0;
  foreach(QGraphicsItem* item, overlapItems) {
    if (item->zValue() >= zValue && item->type() == ModelItem::Type)
      zValue = item->zValue() + 0.1;
  }
  selectedItem->setZValue(zValue);
}

void AeraVisulizerWindow::sendToBack()
{
  if (scene_->selectedItems().isEmpty())
    return;

  QGraphicsItem* selectedItem = scene_->selectedItems().first();
  QList<QGraphicsItem*> overlapItems = selectedItem->collidingItems();

  qreal zValue = 0;
  foreach(QGraphicsItem* item, overlapItems) {
    if (item->zValue() <= zValue && item->type() == ModelItem::Type)
      zValue = item->zValue() - 0.1;
  }
  selectedItem->setZValue(zValue);
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

  zoomToThisAction_ = new QAction(tr("&Zoom to This"), this);
  connect(zoomToThisAction_, SIGNAL(triggered()), this, SLOT(zoomToThis()));

  toFrontAction_ = new QAction(tr("Bring to &Front"), this);
  connect(toFrontAction_, SIGNAL(triggered()), this, SLOT(bringToFront()));

  sendBackAction_ = new QAction(tr("Send to &Back"), this);
  connect(sendBackAction_, SIGNAL(triggered()), this, SLOT(sendToBack()));
}

void AeraVisulizerWindow::createMenus()
{
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(exitAction_);

  QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAction_);
  viewMenu->addAction(zoomOutAction_);
  viewMenu->addAction(zoomHomeAction_);

  itemMenu_ = menuBar()->addMenu(tr("&Item"));
  itemMenu_->addAction(zoomToThisAction_);
  itemMenu_->addAction(toFrontAction_);
  itemMenu_->addAction(sendBackAction_);
}

void AeraVisulizerWindow::createToolbars()
{
  QToolBar* toolbar = addToolBar(tr("Main"));
  toolbar->addAction(zoomInAction_);
  toolbar->addAction(zoomOutAction_);
  toolbar->addAction(zoomHomeAction_);
}

}
