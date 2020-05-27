#include <regex>
#include <algorithm>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtWidgets>
#include <QRegularExpression>
#include "submodules/replicode/r_exec/opcodes.h"
#include "arrow.hpp"
#include "aera-visualizer-scene.hpp"
#include "../aera-visualizer-window.hpp"
#include "aera-graphics-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

const QString AeraGraphicsItem::DownArrowHtml = "<sub><font size=\"+2\"><b>&#129047;</b></font></sub>";
const QString AeraGraphicsItem::SelectedRadioButtonHtml = "<font size=\"+2\">&#x25C9;</font>";
const QString AeraGraphicsItem::UnselectedRadioButtonHtml = "<font size=\"+3\">&#x25CB;</font>";

AeraGraphicsItem::AeraGraphicsItem(
  AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent,
  const QString& headerPrefix)
: parent_(parent),
  aeraEvent_(aeraEvent), replicodeObjects_(replicodeObjects),
  borderFlashCountdown_(AeraVisualizerScene::FLASH_COUNT),
  // The base class should call setTextItemAndPolygon()
  textItem_(0),
  borderNoHighlightPen_(Qt::black, 1)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

  headerHtml_ = QString("<table width=\"100%\"><tr>") + 
    "<td style=\"white-space:nowrap\"><font size=\"+1\"><b><font color=\"darkred\">" + headerPrefix +
    "</font> <a href=\"#this""\">" + replicodeObjects_.getLabel(aeraEvent_->object_).c_str() + "</a></b></font></td>" +
    "<td style=\"white-space:nowrap\" align=\"right\"><font style=\"color:gray\"> " + 
    replicodeObjects_.relativeTime(aeraEvent_->time_).c_str() + "</font></td>" + "</tr></table><br>";
}

void AeraGraphicsItem::setTextItemAndPolygon(QString html, bool prependHeaderHtml)
{
  // Set up the textItem_ first to get its size.
  if (textItem_)
    delete textItem_;
  textItem_ = new TextItem(this);
  textItem_->setHtml(html);
  // adjustSize() is needed for right-aligned text.
  textItem_->adjustSize();
  if (prependHeaderHtml) {
    // Now add headerHtml_ which has a right-aligned table cell.
    textItem_->setHtml(headerHtml_ + html);
    textItem_->adjustSize();
  }

  qreal left = -textItem_->boundingRect().width() / 2 - 5;
  qreal top = -textItem_->boundingRect().height() / 2 - 5;
  textItem_->setPos(left + 5, top + 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
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

  auto saveRect = boundingRect();
  setPolygon(path.toFillPolygon());
  if (saveRect.width() > 0) {
    // We are resizing. Preserve the location of the top-left.
    auto delta = boundingRect().topLeft() - saveRect.topLeft();
    setPos(pos() - delta);
  }
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

void AeraGraphicsItem::bringToFront()
{
  qreal zValue = 0;
  foreach(QGraphicsItem* item, collidingItems()) {
    if (item->zValue() >= zValue && !!dynamic_cast<AeraGraphicsItem*>(item))
      zValue = item->zValue() + 0.1;
  }
  setZValue(zValue);
}

void AeraGraphicsItem::sendToBack()
{
  qreal zValue = 0;
  foreach(QGraphicsItem * item, collidingItems()) {
    if (item->zValue() <= zValue && !!dynamic_cast<AeraGraphicsItem*>(item))
      zValue = item->zValue() - 0.1;
  }
  setZValue(zValue);
}

void AeraGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  auto menu = new QMenu();
  menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
  menu->addAction("Bring To Front", [=]() { bringToFront(); });
  menu->addAction("Send To Back", [=]() { sendToBack(); });
  menu->exec(QCursor::pos() - QPoint(10, 10));
  delete menu;
}

QVariant AeraGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange) {
    aeraEvent_->itemTopLeftPosition_ = boundingRect().topLeft() + value.toPointF();

    foreach(Arrow * arrow, arrows_)
      arrow->updatePosition();
  }

  return value;
}

QString AeraGraphicsItem::htmlify(const QString& input)
{
  QString result = input;

  int maxExtraSpaces = 0;
  QRegularExpression extraSpaceRegex(" ( +)");
  for (auto i = extraSpaceRegex.globalMatch(result); i.hasNext(); ) {
    auto match = i.next();
    maxExtraSpaces = max(maxExtraSpaces, match.captured(1).size());
  }

  // Start with the most extra spaces and work backwards.
  for (int extraSpaces = maxExtraSpaces; extraSpaces >= 1; --extraSpaces) {
    QString find = " ";
    QString replace = " ";
    for (int i = 0; i <= extraSpaces; ++i) {
      find += " ";
      replace += "&nbsp;";
    }
    
    result.replace(find, replace);
  }

  result.replace("\n", "<br>");
  result.replace("\x01", "<br>");
  return result;
}

void AeraGraphicsItem::addSourceCodeHtmlLinks(
  Code* object, QString& html, const ReplicodeObjects& replicodeObjects)
{
  for (int i = 0; i < object->references_size(); ++i) {
    auto referencedObject = object->get_reference(i);
    if (!(referencedObject->code(0).asOpcode() == Opcodes::Mdl ||
          referencedObject->code(0).asOpcode() == Opcodes::Cst ||
          referencedObject->code(0).asOpcode() == Opcodes::Fact ||
          referencedObject->code(0).asOpcode() == Opcodes::AntiFact))
      continue;

    QString referencedLabel(replicodeObjects.getLabel(referencedObject).c_str());
    if (referencedLabel == "")
      continue;

    // TODO: Handle case when the label is not surrounded by spaces.
    html.replace(
      " " + referencedLabel + " ",
      " <a href=\"#debug_oid-" + QString::number(referencedObject->get_debug_oid()) + "\">" +
      referencedLabel + "</a> ");
    // The same for at the end of a line.
    html.replace(
      " " + referencedLabel + "<br>",
      " <a href=\"#debug_oid-" + QString::number(referencedObject->get_debug_oid()) + "\">" +
      referencedLabel + "</a><br>");
    // The same for at the end of a list.
    html.replace(
      " " + referencedLabel + ")",
      " <a href=\"#debug_oid-" + QString::number(referencedObject->get_debug_oid()) + "\">" +
      referencedLabel + "</a>)");
  }
}

void AeraGraphicsItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else if (link.startsWith("#debug_oid-")) {
    uint64 debug_oid = link.mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (object) {
      if (parent_->getParent()->getAeraGraphicsItem(object)) {
        // Show the menu.
        auto menu = new QMenu();
        menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
          [=]() { parent_->getParent()->zoomToAeraGraphicsItem(object); });
        menu->exec(QCursor::pos() - QPoint(10, 10));
        delete menu;
      }
    }
  }
}

void AeraGraphicsItem::TextItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  parent_->parent_->getParent()->textItemHoverMoveEvent(document(), event->pos());

  QGraphicsTextItem::hoverMoveEvent(event);
}

}
