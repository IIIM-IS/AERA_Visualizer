#include "arrow.hpp"
#include "aera-model-item.hpp"
#include "aera-visualizer-scene.hpp"
#include "aera-visualizer-window.hpp"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;

namespace aera_visualizer {

static void
addExampleEvents(std::vector<shared_ptr<AeraEvent> >& events, Timestamp timeReference)
{
  events.push_back(make_shared<NewModelEvent>(timeReference + microseconds(50000), 53, 1, 1));
  events.push_back(make_shared<NewModelEvent>(timeReference + microseconds(2000044), 54, 1, 1));
  events.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(timeReference + microseconds(4003044), 53, 2, 0.8));
  events.push_back(make_shared<NewModelEvent>(timeReference + microseconds(6000366), 54, 1, 0.52));
  events.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(timeReference + microseconds(8070244), 54, 2, 0.55));
  events.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(timeReference + microseconds(8603044), 53, 3, 0.75));
  events.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(timeReference + microseconds(11060000), 54, 3, 0.45));
  events.push_back(make_shared<SetModelEvidenceCountAndSuccessRateEvent>(timeReference + microseconds(14080000), 53, 4, 0.76));
}

AeraVisulizerWindow::AeraVisulizerWindow()
: AeraVisulizerWindowBase(0),
  iNextEvent_(0)
{
  createActions();
  createMenus();

  string userOperatorsFilePath = "C:\\Users\\Jeff\\AERA\\replicode\\Test\\V1.2\\user.classes.replicode";
  string decompiledFilePath = "C:\\Users\\Jeff\\AERA\\replicode\\Test\\decompiled_objects.txt";
  replicodeObjects_.init(userOperatorsFilePath, decompiledFilePath);
  setTimeReference(replicodeObjects_.getTimeReference());

  scene_ = new AeraVisualizerScene(itemMenu_, this);
  scene_->setSceneRect(QRectF(0, 0, 5000, 5000));
  connect(scene_, SIGNAL(itemInserted(AeraModelItem*)),
    this, SLOT(itemInserted(AeraModelItem*)));
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

  addExampleEvents(events_, replicodeObjects_.getTimeReference());
}

Timestamp AeraVisulizerWindow::stepEvent(Timestamp maximumTime)
{
  if (iNextEvent_ >= events_.size())
    // Return the value meaning no change.
    return Utils_MaxTime;

  AeraEvent* event = events_[iNextEvent_].get();
  if (event->time_ > maximumTime)
    return Utils_MaxTime;

  if (event->eventType_ == NewModelEvent::EVENT_TYPE) {
    // Add the new model.
    AeraModelItem* newItem = scene_->addAeraModelItem((NewModelEvent*)event);
    scene_->establishFlashTimer();

    // Debug: testing arrows.
    if (iNextEvent_ > 0 && events_[0]->eventType_ == NewModelEvent::EVENT_TYPE) {
      AeraModelItem* firstModelItem = scene_->getAeraModelItem
      (((NewModelEvent*)events_[0].get())->oid_);
      if (firstModelItem)
        scene_->addArrow(newItem, firstModelItem);
    }
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE) {
    auto setSuccessRateEvent = (SetModelEvidenceCountAndSuccessRateEvent*)event;
    auto modelItem = scene_->getAeraModelItem(setSuccessRateEvent->modelOid_);
    if (modelItem) {
      // Save the current values for a later undo.
      setSuccessRateEvent->oldEvidenceCount_ = modelItem->getEvidenceCount();
      setSuccessRateEvent->oldSuccessRate_ = modelItem->getSuccessRate();

      modelItem->setEvidenceCount(setSuccessRateEvent->evidenceCount_);
      modelItem->evidenceCountFlashCountdown_ = 6;
      modelItem->setSuccessRate(setSuccessRateEvent->successRate_);
      modelItem->successRateFlashCountdown_ = 6;
      scene_->establishFlashTimer();
    }
  }

  ++iNextEvent_;

  return event->time_;
}

Timestamp AeraVisulizerWindow::unstepEvent()
{
  if (iNextEvent_ == 0)
    // Return the value meaning no change.
    return Utils_MaxTime;

  --iNextEvent_;

  AeraEvent* event = events_[iNextEvent_].get();
  if (event->eventType_ == NewModelEvent::EVENT_TYPE) {
    // Find the AeraModelItem for this event and remove it.
    // Note that the event saves the updated item position and will use it when recreating the item.
    auto modelItem = scene_->getAeraModelItem(((NewModelEvent*)event)->oid_);
    if (modelItem) {
      modelItem->removeArrows();
      scene_->removeItem(modelItem);
      delete modelItem;
    }
  }
  else if (event->eventType_ == SetModelEvidenceCountAndSuccessRateEvent::EVENT_TYPE) {
    // Find the AeraModelItem for this event and set to the old evidence count and success rate.
    auto setSuccessRateEvent = (SetModelEvidenceCountAndSuccessRateEvent*)event;
    auto modelItem = scene_->getAeraModelItem(setSuccessRateEvent->modelOid_);
    if (modelItem) {
      modelItem->setEvidenceCount(setSuccessRateEvent->oldEvidenceCount_);
      modelItem->evidenceCountFlashCountdown_ = 6;
      modelItem->setSuccessRate(setSuccessRateEvent->oldSuccessRate_);
      modelItem->successRateFlashCountdown_ = 6;
      scene_->establishFlashTimer();
    }
  }

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

void AeraVisulizerWindow::bringToFront()
{
  if (scene_->selectedItems().isEmpty())
    return;

  QGraphicsItem* selectedItem = scene_->selectedItems().first();
  QList<QGraphicsItem*> overlapItems = selectedItem->collidingItems();

  qreal zValue = 0;
  foreach(QGraphicsItem* item, overlapItems) {
    if (item->zValue() >= zValue && item->type() == AeraModelItem::Type)
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
    if (item->zValue() <= zValue && item->type() == AeraModelItem::Type)
      zValue = item->zValue() - 0.1;
  }
  selectedItem->setZValue(zValue);
}

void AeraVisulizerWindow::createActions()
{
  exitAction_ = new QAction(tr("E&xit"), this);
  exitAction_->setShortcuts(QKeySequence::Quit);
  exitAction_->setStatusTip(tr("Exit"));
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

  toFrontAction_ = new QAction(tr("Bring to &Front"), this);
  toFrontAction_->setStatusTip(tr("Bring item to front"));
  connect(toFrontAction_, SIGNAL(triggered()), this, SLOT(bringToFront()));

  sendBackAction_ = new QAction(tr("Send to &Back"), this);
  sendBackAction_->setStatusTip(tr("Send item to back"));
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
