#include "aera-model-item.hpp"
#include "arrow.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

using namespace core;

namespace aera_visualizer {

AeraModelItem::AeraModelItem(QMenu* contextMenu, NewModelEvent* newModelEvent, QGraphicsItem* parent)
  : QGraphicsPolygonItem(parent),
  newModelEvent_(newModelEvent)
{
  contextMenu_ = contextMenu;

  const qreal left = -100;
  const qreal right = 90;
  const qreal top = -50;
  const qreal bottom = 40;
  const qreal diameter = 20;
  QPainterPath path;
  path.moveTo(right, diameter / 2);
  path.arcTo(right - diameter, top, diameter, diameter, 0, 90);
  path.arcTo(left, top, diameter, diameter, 90, 90);
  path.arcTo(left, bottom - diameter, diameter, diameter, 180, 90);
  path.arcTo(right - diameter, bottom - diameter, diameter, diameter, 270, 90);
  polygon_ = path.toFillPolygon();

  setPolygon(polygon_);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

  QGraphicsSimpleTextItem* oidLabel = new QGraphicsSimpleTextItem
  ("OID: " + QString::number(newModelEvent_->oid_), this);
  oidLabel->setPos(left + 10, top + 10);

  confidenceLabel_ = new QGraphicsSimpleTextItem(this);
  confidenceLabel_->setPos(left + 10, top + 25);
  setConfidence(newModelEvent->confidence_);

  borderFlashCountdown_ = 6;
  confidenceFlashCountdown_ = 0;
}

void AeraModelItem::removeArrows()
{
  foreach(Arrow* arrow, arrows_) {
    qgraphicsitem_cast<AeraModelItem*>(arrow->startItem())->removeArrow(arrow);
    qgraphicsitem_cast<AeraModelItem*>(arrow->endItem())->removeArrow(arrow);
    scene()->removeItem(arrow);
    delete arrow;
  }
}

void AeraModelItem::removeArrow(Arrow* arrow)
{
  int index = arrows_.indexOf(arrow);
  if (index != -1)
    arrows_.removeAt(index);
}

void AeraModelItem::addArrow(Arrow* arrow)
{
  arrows_.append(arrow);
}

void AeraModelItem::setConfidence(float32 confidence)
{
  confidenceIncreased_ = (confidence >= confidence_);
  confidence_ = confidence;
  confidenceLabel_->setText("Confidence: " + QString::number(confidence_));
}

void AeraModelItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  scene()->clearSelection();
  setSelected(true);
  contextMenu_->exec(event->screenPos());
}

QVariant AeraModelItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange) {
    newModelEvent_->itemPosition_ = value.toPointF();

    foreach(Arrow* arrow, arrows_)
      arrow->updatePosition();
  }

  return value;
}

}
