#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "arrow.hpp"
#include "aera-visualizer-scene.hpp"
#include "aera-graphics-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraGraphicsItem::AeraGraphicsItem(
  QMenu* contextMenu, AeraEvent* newObjectEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : parent_(parent),
  newObjectEvent_(newObjectEvent), replicodeObjects_(replicodeObjects),
  borderFlashCountdown_(6)
{
  contextMenu_ = contextMenu;

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void AeraGraphicsItem::removeArrows()
{
  foreach(Arrow* arrow, arrows_) {
    dynamic_cast<AeraGraphicsItem*>(arrow->startItem())->removeArrow(arrow);
    dynamic_cast<AeraGraphicsItem*>(arrow->endItem())->removeArrow(arrow);
    scene()->removeItem(arrow);
    delete arrow;
  }
}

void AeraGraphicsItem::removeArrow(Arrow* arrow)
{
  int index = arrows_.indexOf(arrow);
  if (index != -1)
    arrows_.removeAt(index);
}

void AeraGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  scene()->clearSelection();
  setSelected(true);
  contextMenu_->exec(event->screenPos());
}

QVariant AeraGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange) {
    newObjectEvent_->itemPosition_ = value.toPointF();

    foreach(Arrow * arrow, arrows_)
      arrow->updatePosition();
  }

  return value;
}

}
