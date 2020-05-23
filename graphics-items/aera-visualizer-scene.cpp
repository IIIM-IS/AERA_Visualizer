#include <algorithm>
#include "aera-visualizer-window.hpp"
#include "arrow.hpp"
#include "model-item.hpp"
#include "aera-visualizer-scene.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QApplication>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

const QPen AeraVisualizerScene::ItemBorderNoHighlightPen(Qt::black, 1);

AeraVisualizerScene::AeraVisualizerScene(
  QMenu* itemMenu, ReplicodeObjects& replicodeObjects, AeraVisulizerWindow* parent)
  : QGraphicsScene(parent),
  parent_(parent),
  replicodeObjects_(replicodeObjects),
  didInitialFit_(false),
  thisFrameTime_(seconds(0)),
  nextFrameLeft_(160 /* Debug: Until we have a split panel for models 10 */),
  borderFlashPen_(Qt::green, 3),
  noFlashColor_("black"),
  valueUpFlashColor_("green"),
  valueDownFlashColor_("red")
{
  itemMenu_ = itemMenu;
  itemColor_ = Qt::white;
  lineColor_ = Qt::black;
  setBackgroundBrush(QColor(245, 245, 245));
  flashTimerId_ = 0;

  eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE] = 20;
  eventTypeFirstTop_[NewInstantiatedCompositeStateEvent::EVENT_TYPE] = 285;
  eventTypeFirstTop_[NewMkValPredictionEvent::EVENT_TYPE] = 393;
  eventTypeFirstTop_[NewPredictionSuccessEvent::EVENT_TYPE] = 530;
  eventTypeFirstTop_[0] = 670;
}

void AeraVisualizerScene::addAeraGraphicsItem(AeraGraphicsItem* item)
{
  if (!didInitialFit_) {
    didInitialFit_ = true;
    views().at(0)->fitInView(QRectF(0, 0, 10, eventTypeFirstTop_[0] + 25), Qt::KeepAspectRatio);
  }

  item->setBrush(itemColor_);

  auto newObjectEvent = item->getAeraEvent();
  if (qIsNaN(newObjectEvent->itemPosition_.x())) {
    // Assign an initial position.
    microseconds samplingPeriod(100000);
    if (newObjectEvent->time_ >= thisFrameTime_ + samplingPeriod) {
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

    if (newObjectEvent->eventType_ == NewModelEvent::EVENT_TYPE || newObjectEvent->eventType_ == NewCompositeStateEvent::EVENT_TYPE) {
      // Debug: Until we have a split panel for models.
      if (newObjectEvent->object_->get_oid() == 52)
        newObjectEvent->itemPosition_ = QPointF(0 + item->boundingRect().width() / 2, 330);
      else if (newObjectEvent->object_->get_oid() == 53)
        newObjectEvent->itemPosition_ = QPointF(0 + item->boundingRect().width() / 2, 530);
      else if (newObjectEvent->object_->get_oid() == 54)
        newObjectEvent->itemPosition_ = QPointF(0 + item->boundingRect().width() / 2, 740);
      item->setZValue(-1);
    }
    else {
      int eventType = 0;
        if (eventTypeFirstTop_.find(newObjectEvent->eventType_) != eventTypeFirstTop_.end())
          // This is a recognized event type.
          eventType = newObjectEvent->eventType_;

        qreal top;
        if (eventTypeNextTop_.find(eventType) != eventTypeNextTop_.end())
          top = eventTypeNextTop_[eventType];
        else
          top = eventTypeFirstTop_[eventType];

      // The item origin is in its center, so offset to the top-left.
      newObjectEvent->itemPosition_ = QPointF(
        thisFrameLeft_ + 3 + item->boundingRect().width() / 2, top + item->boundingRect().height() / 2);

      // Set up eventTypeNextTop_ for the next item.
      eventTypeNextTop_[eventType] = top + item->boundingRect().height() + 15;
      // Set up nextFrameLeft_ for the next frame.
      nextFrameLeft_ = max(nextFrameLeft_, thisFrameLeft_ + item->boundingRect().width() + 14);
    }
  }

  addItem(item);
  item->setPos(newObjectEvent->itemPosition_);
}

void AeraVisualizerScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  if (mouseEvent->button() != Qt::LeftButton)
    return;

  views().at(0)->setDragMode(QGraphicsView::ScrollHandDrag);
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
  arrow->setColor(lineColor_);
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
  if (aeraGraphicsItem)
    aeraGraphicsItem->bringToFront();
}

#if QT_CONFIG(wheelevent)
void AeraVisualizerScene::wheelEvent(QGraphicsSceneWheelEvent* event)
{
  // Accept the event to override other behavior.
  event->accept();
  scaleViewBy(pow((double)2, event->delta() / 1000.0));
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
        aeraGraphicsItem->setPen(ItemBorderNoHighlightPen);
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
