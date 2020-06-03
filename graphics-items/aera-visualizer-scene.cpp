#include <algorithm>
#include "aera-visualizer-window.hpp"
#include "arrow.hpp"
#include "model-item.hpp"
#include "auto-focus-fact-item.hpp"
#include "aera-visualizer-scene.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QApplication>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraVisualizerScene::AeraVisualizerScene(
  ReplicodeObjects& replicodeObjects, AeraVisulizerWindow* parent, bool isMainScene,
  const OnSceneSelected& onSceneSelected)
: QGraphicsScene(parent),
  parent_(parent),
  replicodeObjects_(replicodeObjects),
  isMainScene_(isMainScene),
  onSceneSelected_(onSceneSelected),
  didInitialFit_(false),
  thisFrameTime_(seconds(0)),
  thisFrameLeft_(0),
  nextFrameLeft_(10),
  borderFlashPen_(Qt::green, 3),
  noFlashColor_("black"),
  valueUpFlashColor_("green"),
  valueDownFlashColor_("red")
{
  itemColor_ = Qt::white;
  lineColor_ = Qt::black;
  setBackgroundBrush(QColor(245, 245, 245));
  flashTimerId_ = 0;
  setSceneRect(QRectF(0, 0, 5000, 5000));

  if (isMainScene_) {
    eventTypeFirstTop_[EnvironmentEjectEvent::EVENT_TYPE] = 20;
    eventTypeFirstTop_[EnvironmentInjectEvent::EVENT_TYPE] = 45;
    eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE] = 120;
    eventTypeFirstTop_[NewInstantiatedCompositeStateEvent::EVENT_TYPE] = 385;
    eventTypeFirstTop_[ModelMkValPredictionReduction::EVENT_TYPE] = 493;
    eventTypeFirstTop_[NewPredictionSuccessEvent::EVENT_TYPE] = 630;
    eventTypeFirstTop_[0] = 750;
  }
  else
    // The default, which is used for the models scene.
    eventTypeFirstTop_[0] = 5;
}

void AeraVisualizerScene::addAeraGraphicsItem(AeraGraphicsItem* item)
{
  if (!didInitialFit_) {
    didInitialFit_ = true;
    // Set the height of the view. The width will be set accordingly.
    views().at(0)->fitInView(QRectF(0, 0, 1, 800), Qt::KeepAspectRatio);

    // Separate the environment eject/inject region.
    auto y = eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE] - 5;
    auto line = addLine(sceneRect().left(), y, sceneRect().right(), y, QPen(Qt::darkGray, 1));
    line->setZValue(-100);
  }

  item->setBrush(itemColor_);

  auto newObjectEvent = item->getAeraEvent();
  if (qIsNaN(newObjectEvent->itemTopLeftPosition_.x())) {
    // Assign an initial position.
    microseconds samplingPeriod(100000);
    // Only update positions based on time for the main scehe.
    if (isMainScene_ && newObjectEvent->time_ >= thisFrameTime_ + samplingPeriod) {
      // Start a new frame (or the first frame).
      // TODO: Quantize thisFrameTime_ to a frame boundary from newObjectEvent->time_.
      auto relativeTime = duration_cast<microseconds>(newObjectEvent->time_ - replicodeObjects_.getTimeReference());
      thisFrameTime_ = newObjectEvent->time_ - (relativeTime % samplingPeriod);
      thisFrameLeft_ = nextFrameLeft_;
      // nextFrameLeft_ will be updated below.
      nextFrameLeft_ = 0;
      // Reset the top.
      eventTypeNextTop_.clear();

      // Add the frame boundary line and timestamp.
      auto line = addLine(thisFrameLeft_, sceneRect().top(), 
                          thisFrameLeft_, sceneRect().bottom(), QPen(Qt::darkGray, 1, Qt::DashLine));
      line->setZValue(-100);
      auto text = addText(replicodeObjects_.relativeTime(thisFrameTime_).c_str());
      text->setZValue(-100);
      text->setDefaultTextColor(Qt::darkGray);
      text->setPos(thisFrameLeft_, 0);
    }

    int eventType = 0;
    if (eventTypeFirstTop_.find(newObjectEvent->eventType_) != eventTypeFirstTop_.end())
      // This is a recognized event type.
      eventType = newObjectEvent->eventType_;

    qreal top;
    if (eventTypeNextTop_.find(eventType) != eventTypeNextTop_.end())
      top = eventTypeNextTop_[eventType];
    else
      top = eventTypeFirstTop_[eventType];

    if (newObjectEvent->object_->get_oid() == 41)
      // Debug: Override for the test case to make the same types of values line up. Should use a layout algorithm.
      top = 296;

    int verticalMargin = 15;
    if (newObjectEvent->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ||
      newObjectEvent->eventType_ == EnvironmentEjectEvent::EVENT_TYPE) {
      // Allow inject/eject items to be on the frame boundary.
      newObjectEvent->itemTopLeftPosition_ = QPointF(thisFrameLeft_ + item->boundingRect().left(), top);
      verticalMargin = 5;
    }
    else
      newObjectEvent->itemTopLeftPosition_ = QPointF(thisFrameLeft_ + 5, top);

    // Set up eventTypeNextTop_ for the next item.
    eventTypeNextTop_[eventType] = top + item->boundingRect().height() + verticalMargin;
    nextFrameLeft_ = max(nextFrameLeft_, thisFrameLeft_ + item->boundingRect().width() + 14);
  }

  addItem(item);
  // Adjust the position from the topLeft.
  item->setPos(newObjectEvent->itemTopLeftPosition_ - item->boundingRect().topLeft());
}

void AeraVisualizerScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  if (mouseEvent->button() != Qt::LeftButton)
    return;

  views().at(0)->setDragMode(QGraphicsView::ScrollHandDrag);
  if (onSceneSelected_)
    // Notify the parent that this scene was selected.
    onSceneSelected_();
  QGraphicsScene::mousePressEvent(mouseEvent);
}

void AeraVisualizerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  // Reset the drag mode.
  views().at(0)->setDragMode(QGraphicsView::NoDrag);
  QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void AeraVisualizerScene::addArrow(AeraGraphicsItem* startItem, AeraGraphicsItem* endItem)
{
  if (startItem == endItem)
    return;

  Arrow* arrow = new Arrow(startItem, endItem);
  startItem->addArrow(arrow);
  endItem->addArrow(arrow);
  arrow->setZValue(-1000.0);
  addItem(arrow);
  arrow->updatePosition();
}

AeraGraphicsItem* AeraVisualizerScene::getAeraGraphicsItem(Code* object)
{
  foreach(QGraphicsItem* item, items()) {
    auto graphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (graphicsItem) {
      if (graphicsItem->getAeraEvent()->object_ == object)
        return graphicsItem;
    }
  }

  return 0;
}

void AeraVisualizerScene::scaleViewBy(double factor)
{
  double currentScale = views().at(0)->transform().m11();

  QGraphicsView* view = views().at(0);
  QMatrix oldMatrix = view->matrix();
  view->resetMatrix();
  view->translate(oldMatrix.dx(), oldMatrix.dy());
  view->scale(currentScale *= factor, currentScale *= factor);
}

void AeraVisualizerScene::zoomViewHome()
{
  // Get the bounding rect of all AeraGraphicsItem. This excludes lines such as frame boundaries.
  QRectF boundingRect;
  foreach(QGraphicsItem* item, items()) {
    if (dynamic_cast<AeraGraphicsItem*>(item)) {
      if (boundingRect.width() == 0)
        boundingRect = item->sceneBoundingRect();
      else
        boundingRect = boundingRect.united(item->sceneBoundingRect());
    }
  }

  if (boundingRect.width() != 0)
    views().at(0)->fitInView(boundingRect, Qt::KeepAspectRatio);
}

void AeraVisualizerScene::zoomToItem(QGraphicsItem* item)
{
  views().at(0)->fitInView(item, Qt::KeepAspectRatio);

  auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
  if (aeraGraphicsItem) {
    if (!aeraGraphicsItem->isVisible())
      aeraGraphicsItem->setItemAndArrowsVisible(true);
    aeraGraphicsItem->bringToFront();
  }
}

void AeraVisualizerScene::setItemsVisible(int eventType, bool visible)
{
  foreach(QGraphicsItem * item, items()) {
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (aeraGraphicsItem && aeraGraphicsItem->getAeraEvent()->eventType_ == eventType)
      aeraGraphicsItem->setItemAndArrowsVisible(visible);
  }
}

void AeraVisualizerScene::setAutoFocusItemsVisible(const string& property, bool visible)
{
  auto propertyObject = replicodeObjects_.getObject(property);
  if (!propertyObject)
    return;

  foreach(QGraphicsItem * item, items()) {
    auto autoFocusItem = dynamic_cast<AutoFocusFactItem*>(item);
    if (autoFocusItem) {
      auto mkVal = autoFocusItem->getAeraEvent()->object_->get_reference(0);
      // TODO: Is this reference always the property?
      if (mkVal->references_size() < 2)
        continue;

      auto mkValProperty = mkVal->get_reference(1);
      if (mkValProperty == propertyObject)
        autoFocusItem->setItemAndArrowsVisible(visible);
    }
  }
}

#if QT_CONFIG(wheelevent)
void AeraVisualizerScene::wheelEvent(QGraphicsSceneWheelEvent* event)
{
  if (event->modifiers() == Qt::ControlModifier) {
    // Accept the event to override other behavior.
    event->accept();
    scaleViewBy(pow((double)2, event->delta() / 1000.0));
  }
}
#endif

void AeraVisualizerScene::timerEvent(QTimerEvent* event)
{
  // TODO: Make sure we don't re-enter.

  if (event->timerId() != flashTimerId_)
    // This timer event is not for us.
    return;

  bool isFlashing = false;
  foreach(QGraphicsItem* item, items()) {
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (!aeraGraphicsItem)
      continue;

    if (aeraGraphicsItem->borderFlashCountdown_ > 0) {
      isFlashing = true;

      --aeraGraphicsItem->borderFlashCountdown_;
      if (aeraGraphicsItem->borderFlashCountdown_ % 2 == 1)
        aeraGraphicsItem->setPen(borderFlashPen_);
      else
        aeraGraphicsItem->setPen(aeraGraphicsItem->getBorderNoHighlightPen());
    }

    auto modelItem = dynamic_cast<ModelItem*>(item);
    if (modelItem) {
      if (modelItem->evidenceCountFlashCountdown_ > 0) {
        isFlashing = true;

        --modelItem->evidenceCountFlashCountdown_;
        if (modelItem->evidenceCountFlashCountdown_ % 2 == 1)
          modelItem->setEvidenceCountColor
          (modelItem->evidenceCountIncreased_ ? valueUpFlashColor_ : valueDownFlashColor_);
        else
          modelItem->setEvidenceCountColor(noFlashColor_);
      }

      if (modelItem->successRateFlashCountdown_ > 0) {
        isFlashing = true;

        --modelItem->successRateFlashCountdown_;
        if (modelItem->successRateFlashCountdown_ % 2 == 1)
          modelItem->setSuccessRateColor
          (modelItem->successRateIncreased_ ? valueUpFlashColor_ : valueDownFlashColor_);
        else
          modelItem->setSuccessRateColor(noFlashColor_);
      }
    }
  }

  if (!isFlashing) {
    killTimer(flashTimerId_);
    flashTimerId_ = 0;
  }
}

}
