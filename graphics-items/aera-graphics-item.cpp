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

const char* AeraGraphicsItem::DownArrowHtml = "<sub><font size=\"+2\"><b>&#129047;</b></font></sub>";

AeraGraphicsItem::AeraGraphicsItem(
  QMenu* contextMenu, AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : parent_(parent),
  aeraEvent_(aeraEvent), replicodeObjects_(replicodeObjects),
  borderFlashCountdown_(AeraVisualizerScene::FLASH_COUNT),
  // The base class should call setTextItemAndPolygon()
  textItem_(0)
{
  contextMenu_ = contextMenu;

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void AeraGraphicsItem::setTextItemAndPolygon(QString html)
{

  // Set up the textItem_ first to get its size.
  textItem_ = new QGraphicsTextItem(this);
  textItem_->setHtml(html);
  const qreal left = -textItem_->boundingRect().width() / 2 - 5;
  const qreal top = -textItem_->boundingRect().height() / 2 - 5;
  textItem_->setPos(left + 5, top + 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkHovered,
    [this](const QString& link) { textItemLinkHovered(link); });
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated,
    [this](const QString& link) { textItemLinkActivated(link); });

  qreal right = textItem_->boundingRect().width() / 2 + 15;
  qreal bottom = textItem_->boundingRect().height() / 2 + 5;
  const qreal diameter = 20;

  QPainterPath path;
  path.moveTo(right, diameter / 2);
  path.arcTo(right - diameter, top, diameter, diameter, 0, 90);
  path.arcTo(left, top, diameter, diameter, 90, 90);
  path.arcTo(left, bottom - diameter, diameter, diameter, 180, 90);
  path.arcTo(right - diameter, bottom - diameter, diameter, diameter, 270, 90);
  setPolygon(path.toFillPolygon());
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
    aeraEvent_->itemPosition_ = value.toPointF();

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
          referencedObject->code(0).asOpcode() == Opcodes::Cst ||
          referencedObject->code(0).asOpcode() == Opcodes::Fact ||
          referencedObject->code(0).asOpcode() == Opcodes::AntiFact))
      continue;

    auto referencedLabel = replicodeObjects_.getLabel(referencedObject);
    if (referencedLabel == "")
      continue;

    // Spaces are alreay replaced with &nbsp; .
    // TODO: Handle case when the label is not surrounded by spaces.
    html.replace(
      QString("&nbsp;") + referencedLabel.c_str() + "&nbsp;",
      QString("&nbsp;<a href=\"#debug_oid-") + QString::number(referencedObject->get_debug_oid()) + "\">" +
      referencedLabel.c_str() + "</a>&nbsp;");
    // The same for at the end of a line.
    html.replace(
      QString("&nbsp;") + referencedLabel.c_str() + "<br>",
      QString("&nbsp;<a href=\"#debug_oid-") + QString::number(referencedObject->get_debug_oid()) + "\">" +
      referencedLabel.c_str() + "</a><br>");
  }
}

void AeraGraphicsItem::textItemLinkHovered(const QString& link)
{
  if (link.startsWith("#debug_oid-")) {
    uint64 debug_oid = link.mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (object) {
      auto item = parent_->getAeraGraphicsItem(object);
      if (item) {
        // Flash the corresponding item.
        item->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT;
        parent_->establishFlashTimer();
      }
    }
  }
}

void AeraGraphicsItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->exec(parent_->getMouseScreenPosition() - QPoint(10, 10));
    delete menu;
  }
  else if (link.startsWith("#debug_oid-")) {
    uint64 debug_oid = link.mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (object) {
      auto item = parent_->getAeraGraphicsItem(object);
      if (item) {
        // Show the menu.
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
