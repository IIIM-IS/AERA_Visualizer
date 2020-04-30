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

  evidenceCountLabel_ = new QGraphicsSimpleTextItem(this);
  evidenceCountLabel_->setPos(left + 10, top + 25);
  setEvidenceCount(newModelEvent->evidenceCount_);

  successRateLabel_ = new QGraphicsSimpleTextItem(this);
  successRateLabel_->setPos(left + 10, top + 40);
  setSuccessRate(newModelEvent->successRate_);

  borderFlashCountdown_ = 6;
  evidenceCountFlashCountdown_ = 0;
  successRateFlashCountdown_ = 0;
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

void AeraModelItem::setEvidenceCount(float32 evidenceCount)
{
  evidenceCountIncreased_ = (evidenceCount >= evidenceCount_);
  evidenceCount_ = evidenceCount;
  evidenceCountLabel_->setText("Evidence Count: " + QString::number(evidenceCount_));
}

void AeraModelItem::setSuccessRate(float32 successRate)
{
  successRateIncreased_ = (successRate >= successRate_);
  successRate_ = successRate;
  successRateLabel_->setText("    Success Rate: " + QString::number(successRate_));
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
