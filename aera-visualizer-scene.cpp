#include "aera-visualizer-scene.hpp"
#include "arrow.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QApplication>

using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraVisualizerScene::AeraVisualizerScene(QMenu* itemMenu, ReplicodeObjects& replicodeObjects, QObject* parent)
  : QGraphicsScene(parent),
  replicodeObjects_(replicodeObjects),
  borderFlashPen_(Qt::blue, 3),
  noFlashColor_("black"),
  valueUpFlashColor_("green"),
  valueDownFlashColor_("red")
{
  itemMenu_ = itemMenu;
  line_ = 0;
  itemColor_ = Qt::white;
  lineColor_ = Qt::black;
  setBackgroundBrush(QColor(230, 230, 230));
  flashTimerId_ = 0;
}

AeraModelItem* AeraVisualizerScene::addAeraModelItem(NewModelEvent* newModelEvent)
{
  AeraModelItem* item = new AeraModelItem(itemMenu_, newModelEvent, replicodeObjects_, this);
  item->setBrush(itemColor_);

  if (qIsNaN(newModelEvent->itemPosition_.x())) {
    // Assign an initial position.
    // TODO: Do this with a grid layout.
    newModelEvent->itemPosition_ =
      QPointF(newModelEvent->object_->get_oid() * 320 - 14730, 2380);
  }

  addItem(item);
  item->setPos(newModelEvent->itemPosition_);

  return item;
}

void AeraVisualizerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  mouseScreenPosition_ = event->screenPos();
  QGraphicsScene::mouseMoveEvent(event);
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
      if (graphicsItem->getNewObjectEvent()->object_ == object)
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
  views().at(0)->fitInView(itemsBoundingRect(), Qt::KeepAspectRatio);
}

void AeraVisualizerScene::zoomToItem(QGraphicsItem* item)
{
  views().at(0)->fitInView(item, Qt::KeepAspectRatio);
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
    AeraModelItem* modelItem = dynamic_cast<AeraModelItem*>(item);
    if (!modelItem)
      continue;

    if (modelItem->borderFlashCountdown_ > 0) {
      isFlashing = true;

      --modelItem->borderFlashCountdown_;
      if (modelItem->borderFlashCountdown_ % 2 == 1)
        modelItem->setPen(borderFlashPen_);
      else
        modelItem->setPen(lineColor_);
    }

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

  if (!isFlashing) {
    killTimer(flashTimerId_);
    flashTimerId_ = 0;
  }
}

}
