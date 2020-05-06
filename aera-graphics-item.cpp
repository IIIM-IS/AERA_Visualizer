#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "submodules/replicode/r_exec/opcodes.h"
#include "arrow.hpp"
#include "aera-visualizer-scene.hpp"
#include "aera-graphics-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

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

void AeraGraphicsItem::addSourceCodeHtmlLinks(Code* object, QString& html)
{
  for (int i = 0; i < object->references_size(); ++i) {
    auto referencedObject = object->get_reference(i);
    if (!(referencedObject->code(0).asOpcode() == Opcodes::Mdl ||
      referencedObject->code(0).asOpcode() == Opcodes::Cst))
      continue;

    auto referencedLabel = replicodeObjects_.getLabel(referencedObject);
    if (referencedLabel == "")
      continue;

    // Spaces are alreay replaced with &nbsp; .
    // TODO: Handle case when the label is not surrounded by spaces.
    html.replace(
      QString("&nbsp;") + referencedLabel.c_str() + "&nbsp;",
      QString("&nbsp;<a href=\"#oid-") + QString::number(referencedObject->get_oid()) + "\">" +
      referencedLabel.c_str() + "</a>&nbsp;");
  }
}

void AeraGraphicsItem::textItemLinkActivated(const QString& link)
{
  if (link.startsWith("#oid-")) {
    int oid = link.mid(5).toInt();
    auto object = replicodeObjects_.getObject(oid);
    if (object) {
      // TODO: Make this work for other than models.
      auto item = parent_->getAeraGraphicsItem(object);
      if (item) {
        auto menu = new QMenu();
        menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
          [=]() { parent_->zoomToItem(item); });
        menu->exec(parent_->getMouseScreenPosition() - QPoint(10, 10));
        delete menu;
      }
    }
  }
}

}
