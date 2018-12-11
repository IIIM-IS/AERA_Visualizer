#include "arrow.h"
#include "aera-model-item.h"
#include "aera-visualizer-scene.h"
#include "aera-visualizer-window.h"

#include <QtWidgets>

using namespace std;
using namespace core;

namespace aera_visualizer {

static void
addExampleEvents(vector<shared_ptr<AeraEvent> >& events)
{
  events.push_back(make_shared<NewModelEvent>(50000, 2400, 0.5));
  events.push_back(make_shared<NewModelEvent>(2000044, 2401, 0.51));
  events.push_back(make_shared<SetModelConfidenceEvent>(4003044, 2400, 0.8));
  events.push_back(make_shared<NewModelEvent>(6000366, 2641, 0.52));
  events.push_back(make_shared<SetModelConfidenceEvent>(8070244, 2401, 0.55));
  events.push_back(make_shared<SetModelConfidenceEvent>(8603044, 2400, 0.75));
  events.push_back(make_shared<SetModelConfidenceEvent>(11060000, 2401, 0.45));
  events.push_back(make_shared<SetModelConfidenceEvent>(12030000, 2641, 0.71));
  events.push_back(make_shared<SetModelConfidenceEvent>(14080000, 2400, 0.76));
}

AeraVisulizerWindow::AeraVisulizerWindow()
: AeraVisulizerWindowBase(0),
  iNextEvent_(0)
{
  createActions();
  createMenus();

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

  addExampleEvents(events_);
}

uint64 AeraVisulizerWindow::stepEvent(uint64 maximumTime)
{
  if (iNextEvent_ >= events_.size())
    // Return the value meaning no change.
    return uint64_MAX;

  AeraEvent* event = events_[iNextEvent_].get();
  if (event->time_ > maximumTime)
    return uint64_MAX;

  if (event->eventType_ == NewModelEvent::EVENT_TYPE) {
    // Add the new model.
    AeraModelItem* newItem = scene_->addAeraModelItem((NewModelEvent*)event);
    scene_->establishFlashTimer();

    // Debug: testing arrows.
    if (iNextEvent_ > 0 && events_[0]->eventType_ == NewModelEvent::EVENT_TYPE) {
      AeraModelItem* firstModelItem = scene_->getAeraModelItem
      (((NewModelEvent*)events_[0].get())->oid_);
      if (firstModelItem)
        scene_->addArrow(firstModelItem, newItem);
    }
  }
  else if (event->eventType_ == SetModelConfidenceEvent::EVENT_TYPE) {
    auto setConfidenceEvent = (SetModelConfidenceEvent*)event;
    auto modelItem = scene_->getAeraModelItem(setConfidenceEvent->modelOid_);
    if (modelItem) {
      // Save the current value for a later undo.
      setConfidenceEvent->oldConfidence_ = modelItem->getConfidence();

      modelItem->setConfidence(setConfidenceEvent->confidence_);
      modelItem->confidenceFlashCountdown_ = 6;
      scene_->establishFlashTimer();
    }
  }

  ++iNextEvent_;

  return event->time_;
}

uint64 AeraVisulizerWindow::unstepEvent()
{
  if (iNextEvent_ == 0)
    // Return the value meaning no change.
    return uint64_MAX;

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
  else if (event->eventType_ == SetModelConfidenceEvent::EVENT_TYPE) {
    // Find the AeraModelItem for this event and set to the old confidence.
    auto setConfidenceEvent = (SetModelConfidenceEvent*)event;
    auto modelItem = scene_->getAeraModelItem(setConfidenceEvent->modelOid_);
    if (modelItem) {
      modelItem->setConfidence(setConfidenceEvent->oldConfidence_);
      modelItem->confidenceFlashCountdown_ = 6;
      scene_->establishFlashTimer();
    }
  }

  if (iNextEvent_ > 0)
    return events_[iNextEvent_ - 1]->time_;
  else
    return 0;
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
