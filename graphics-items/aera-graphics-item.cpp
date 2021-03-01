//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020 Icelandic Institute for Intelligent Machines ses
//_/_/ Vitvelastofnun Islands ses, kt. 571209-0390
//_/_/ Author: Jeffrey Thompson <jeff@iiim.is>
//_/_/
//_/_/ -----------------------------------------------------------------------
//_/_/ Released under Open-Source BSD License with CADIA Clause v 1.0
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/   and collaboration notice, this list of conditions and the 
//_/_/   following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/   notice, this list of conditions and the following
//_/_/   disclaimer in the documentation and/or other materials provided 
//_/_/   with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/   contributors may be used to endorse or promote products 
//_/_/   derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause v 1.0: The license granted in and to the software under 
//_/_/   this agreement is a limited-use license. The software may not be used
//_/_/   in furtherance of: 
//_/_/   (i) intentionally causing bodily injury or severe emotional distress 
//_/_/   to any person; 
//_/_/   (ii) invading the personal privacy or violating the human rights of 
//_/_/   any person; or 
//_/_/   (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <regex>
#include <algorithm>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtWidgets>
#include <QRegularExpression>
#include "submodules/replicode/r_exec/opcodes.h"
#include "arrow.hpp"
#include "anchored-horizontal-line.hpp"
#include "aera-visualizer-scene.hpp"
#include "../aera-visualizer-window.hpp"
#include "aera-graphics-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

const QString AeraGraphicsItem::DownArrowHtml = "<sub><font size=\"+2\"><b>&#129047;</b></font></sub>";
const QString AeraGraphicsItem::RightArrowHtml = "<font size=\"+1\"><b>&#129050;</b></font>";
const QString AeraGraphicsItem::RightDoubleArrowHtml = "<font size=\"+1\"><b>&#8658;</b></font>";
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
  setAcceptHoverEvents(true);

  Timestamp eventTime;
  if (is_sim())
    // We know that a simulated item's object has the form (fact (goal_or_pred (fact ...)))
    eventTime = ((_Fact*)aeraEvent->object_->get_reference(0)->get_reference(0))->get_after();
  else
    eventTime = aeraEvent_->time_;
  headerHtml_ = QString("<table width=\"100%\"><tr>") + 
    "<td style=\"white-space:nowrap\"><font size=\"+1\"><b><font color=\"darkred\">" + headerPrefix +
    "</font> <a href=\"#this""\">" + replicodeObjects_.getLabel(aeraEvent_->object_).c_str() + "</a></b></font></td>" +
    "<td style=\"white-space:nowrap\" align=\"right\"><font style=\"color:gray\"> " + 
    replicodeObjects_.relativeTime(eventTime).c_str() + "</font></td>" + "</tr></table><br>";
}

void AeraGraphicsItem::setTextItemAndPolygon(QString html, bool prependHeaderHtml, Shape shape, qreal targetWidth)
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

  qreal width = (textItem_->boundingRect().width() / 2 + 15) - left;
  if (targetWidth > width)
    width = targetWidth;
  qreal right = left + width;
  qreal bottom = textItem_->boundingRect().height() / 2 + 5;
  const qreal diameter = 20;

  QPainterPath path;
  if (shape == SHAPE_GOAL) {
    path.moveTo(right, top);
    path.lineTo(left + diameter / 2, top);
    path.lineTo(left, (top + bottom) / 2);
    path.lineTo(left + diameter / 2, bottom);
    path.lineTo(right, bottom);
  }
  else if (shape == SHAPE_PRED) {
    path.moveTo(left, top);
    path.lineTo(right - diameter / 2, top);
    path.lineTo(right, (top + bottom) / 2);
    path.lineTo(right - diameter / 2, bottom);
    path.lineTo(left, bottom);
  }
  else {
    path.moveTo(right, top + diameter / 2);
    path.arcTo(right - diameter, top, diameter, diameter, 0, 90);
    path.arcTo(left, top, diameter, diameter, 90, 90);
    path.arcTo(left, bottom - diameter, diameter, diameter, 180, 90);
    path.arcTo(right - diameter, bottom - diameter, diameter, diameter, 270, 90);
  }

  auto saveRect = boundingRect();
  setPolygon(path.toFillPolygon());
  if (saveRect.width() > 0) {
    // We are resizing. Preserve the location of the top-left.
    auto delta = boundingRect().topLeft() - saveRect.topLeft();
    setPos(pos() - delta);
  }
}

void AeraGraphicsItem::removeArrowsAndHorizontalLines()
{
  foreach(Arrow* arrow, arrows_) {
    auto startItem = dynamic_cast<AeraGraphicsItem*>(arrow->startItem());
    if (startItem)
      startItem->removeArrow(arrow);
    auto endItem = dynamic_cast<AeraGraphicsItem*>(arrow->endItem());
    if (endItem)
      endItem->removeArrow(arrow);
    scene()->removeItem(arrow);
    delete arrow;
  }

  foreach(AnchoredHorizontalLine* line, horizontalLines_) {
    auto item = dynamic_cast<AeraGraphicsItem*>(line->item());
    if (item)
      item->removeHorizontalLine(line);
    scene()->removeItem(line);
    delete line;
  }
}

void AeraGraphicsItem::removeArrow(Arrow* arrow)
{
  int index = arrows_.indexOf(arrow);
  if (index != -1)
    arrows_.removeAt(index);
}

void AeraGraphicsItem::removeHorizontalLine(AnchoredHorizontalLine* line)
{
  int index = horizontalLines_.indexOf(line);
  if (index != -1)
    horizontalLines_.removeAt(index);
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

void AeraGraphicsItem::resetPosition()
{
  if (!qIsNaN(aeraEvent_->itemInitialTopLeftPosition_.x()))
    setPos(aeraEvent_->itemInitialTopLeftPosition_ - boundingRect().topLeft());
}

void AeraGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  auto menu = new QMenu();
  menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
  menu->addAction("Bring To Front", [=]() { bringToFront(); });
  menu->addAction("Send To Back", [=]() { sendToBack(); });
  menu->addAction("Reset Position", [=]() { resetPosition(); });
  menu->exec(QCursor::pos() - QPoint(10, 10));
  delete menu;
}

QVariant AeraGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange) {
    aeraEvent_->itemTopLeftPosition_ = boundingRect().topLeft() + value.toPointF();

    foreach(Arrow * arrow, arrows_)
      arrow->updatePosition();
    foreach(AnchoredHorizontalLine * line, horizontalLines_)
      line->updatePosition();
  }

  return value;
}

QString AeraGraphicsItem::htmlify(const QString& input, bool useNowrap)
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

  result.replace("|fact", "<font color=\"red\">|fact</font>");
  result.replace("|pgm", "<font color=\"red\">|pgm</font>");
  result.replace("\n", "<br>");
  result.replace("\x01", "<br>");

  if (useNowrap)
    result = "<div style=\"white-space: nowrap;\">" + htmlify(result) + "</div>";

  return result;
}

QString AeraGraphicsItem::makeHtmlLink(
  r_code::Code* object, const ReplicodeObjects& replicodeObjects)
{
  QString label = replicodeObjects.getLabel(object).c_str();
  if (label == "")
    // We don't expect this to happen.
    label = "object_" + QString::number(object->get_debug_oid());

  return "<a href=\"#debug_oid-" + QString::number(object->get_debug_oid()) + "\">" +
    label + "</a>";
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
      " " + makeHtmlLink(referencedObject, replicodeObjects) + " ");
    // The same for at the end of a line.
    html.replace(
      " " + referencedLabel + "<br>",
      " " + makeHtmlLink(referencedObject, replicodeObjects) + "<br>");
    // The same for at the end of a list.
    html.replace(
      " " + referencedLabel + ")",
      " " + makeHtmlLink(referencedObject, replicodeObjects) + ")");
  }
}

void AeraGraphicsItem::setItemAndArrowsAndHorizontalLinesVisible(bool visible)
{
  foreach(Arrow* arrow, arrows_) {
    if (visible) {
      // Only set the arrow visible if the connected item is visible.
      if (arrow->startItem() == this && arrow->endItem()->isVisible() ||
          arrow->endItem() == this && arrow->startItem()->isVisible())
        arrow->setVisible(true);
      else
        arrow->setVisible(false);
    }
    else
      arrow->setVisible(false);
  }

  foreach(AnchoredHorizontalLine* line, horizontalLines_)
    line->setVisible(visible);
  
  setVisible(visible);
}

bool AeraGraphicsItem::is_sim()
{
  if (aeraEvent_->object_->references_size() < 1)
    return false;

  auto obj = aeraEvent_->object_->get_reference(0);
  if (obj->code(0).asOpcode() == Opcodes::Goal)
    return ((Goal*)obj)->is_simulation();
  else if (obj->code(0).asOpcode() == Opcodes::Pred)
    return ((Pred*)obj)->is_simulation();

  return false;
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

void AeraGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  // Highlight connected arrows and horizontal lines.
  foreach(Arrow * arrow, arrows_) {
    arrow->setPen(Arrow::HighlightedPen);
    arrow->update();
  }
  foreach(AnchoredHorizontalLine* line, horizontalLines_) {
    line->setPen(AnchoredHorizontalLine::HighlightedPen);
    line->update();
  }

  QGraphicsPolygonItem::hoverEnterEvent(event);
}

void AeraGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  // Reset highlighting of connected arrows and horizontal lines.
  foreach(Arrow * arrow, arrows_) {
    arrow->setPen(Arrow::DefaultPen);
    arrow->update();
  }
  foreach(AnchoredHorizontalLine* line, horizontalLines_) {
    line->setPen(AnchoredHorizontalLine::DefaultPen);
    line->update();
  }

  QGraphicsPolygonItem::hoverLeaveEvent(event);
}

void AeraGraphicsItem::TextItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  parent_->parent_->getParent()->textItemHoverMoveEvent(document(), event->pos());

  QGraphicsTextItem::hoverMoveEvent(event);
}

}
