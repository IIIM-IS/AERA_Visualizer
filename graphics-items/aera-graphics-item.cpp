//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2021 Jeff Thompson
//_/_/ Copyright (c) 2021 Kristinn R. Thorisson
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
//_/_/ Copyright (c) 2021 Leonard Eberding
//_/_/ Copyright (c) 2021 Icelandic Institute for Intelligent Machines
//_/_/ http://www.iiim.is
//_/_/
//_/_/ --- Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without
//_/_/ modification, is permitted provided that the following conditions
//_/_/ are met:
//_/_/ - Redistributions of source code must retain the above copyright
//_/_/   and collaboration notice, this list of conditions and the
//_/_/   following disclaimer.
//_/_/ - Redistributions in binary form must reproduce the above copyright
//_/_/   notice, this list of conditions and the following disclaimer 
//_/_/   in the documentation and/or other materials provided with 
//_/_/   the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its
//_/_/   contributors may be used to endorse or promote products
//_/_/   derived from this software without specific prior 
//_/_/   written permission.
//_/_/   
//_/_/ - CADIA Clause: The license granted in and to the software 
//_/_/   under this agreement is a limited-use license. 
//_/_/   The software may not be used in furtherance of:
//_/_/    (i)   intentionally causing bodily injury or severe emotional 
//_/_/          distress to any person;
//_/_/    (ii)  invading the personal privacy or violating the human 
//_/_/          rights of any person; or
//_/_/    (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
//_/_/ CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
//_/_/ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//_/_/ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//_/_/ DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//_/_/ CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//_/_/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//_/_/ SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//_/_/ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//_/_/ WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//_/_/ NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//_/_/ OF SUCH DAMAGE.
//_/_/ 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <regex>
#include <algorithm>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtWidgets>
#include <QRegularExpression>
#include "submodules/AERA/r_exec/opcodes.h"
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
const QString AeraGraphicsItem::RightPointingTriangleHtml = "<font size=\"+2\">&#x25B6;</font>";
const QString AeraGraphicsItem::DownPointingTriangleHtml = "<font size=\"+1\">&#x25BC;</font>";

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
    "</font> <a href=\"#this\">" + replicodeObjects_.getLabel(aeraEvent_->object_).c_str() + "</a></b></font></td>" +
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
  else if (shape == SHAPE_STOP) {
    path.moveTo(left + diameter / 2, top);
    path.lineTo(right - diameter / 2, top);
    path.lineTo(right, top + (bottom - top) / 4);
    path.lineTo(right, top + (bottom - top) * 3 / 4);
    path.lineTo(right - diameter / 2, bottom);
    path.lineTo(left + diameter / 2, bottom);
    path.lineTo(left, top + (bottom - top) * 3 / 4);
    path.lineTo(left, top + (bottom - top) / 4);
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

void AeraGraphicsItem::focus()
{
  bringToFront();
  ensureVisible();
  setSelected(true);
}

void AeraGraphicsItem::resetPosition()
{
  if (!qIsNaN(aeraEvent_->itemInitialTopLeftPosition_.x()))
    setPos(aeraEvent_->itemInitialTopLeftPosition_ - boundingRect().topLeft());
}

void AeraGraphicsItem::centerOn()
{
  QGraphicsView *qGraphicsView = parent_->views().at(0);
  QRectF sceneRect = sceneBoundingRect();

  // If the item is wider than the scene, just center on the left part of it
  if (qGraphicsView->viewport()->width() < sceneRect.width()) {
    sceneRect.setWidth(qGraphicsView->viewport()->width());
    qGraphicsView->centerOn(sceneRect.center());
  }
  else {
    qGraphicsView->centerOn(this);
  }
  bringToFront();
  setSelected(true);
}

void AeraGraphicsItem::ensureVisible()
{
  QGraphicsView *qGraphicsView = parent_->views().at(0);
  QRectF sceneRect = sceneBoundingRect();

  // If the item is wider than the scene, just ensure the left side of it is visible
  if (qGraphicsView->viewport()->width() < sceneRect.width()) {
    sceneRect.setWidth(qGraphicsView->viewport()->width());
    qGraphicsView->ensureVisible(sceneRect, 0, 0);
  }
  else {
    qGraphicsView->ensureVisible(this);
  }
}

void AeraGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  auto menu = new QMenu();
  menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
  menu->addAction("Focus on This", [=]() { parent_->focusOnItem(this); });
  menu->addAction("Center on This", [=]() { parent_->centerOnItem(this); });
  menu->addAction("Bring To Front", [=]() { bringToFront(); });
  menu->addAction("Send To Back", [=]() { sendToBack(); });
  menu->addAction("Reset Position", [=]() { resetPosition(); });
  menu->exec(QCursor::pos() - QPoint(10, 10));
  delete menu;
}

void AeraGraphicsItem::updateArrowsAndLines()
{
  foreach(Arrow * arrow, arrows_)
    arrow->updatePosition();
  foreach(AnchoredHorizontalLine * line, horizontalLines_)
    line->updatePosition();
}

QVariant AeraGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange) {
    aeraEvent_->itemTopLeftPosition_ = boundingRect().topLeft() + value.toPointF();

    updateArrowsAndLines();
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
    result = "<div style=\"white-space: nowrap;\">" + result + "</div>";

  return result;
}

QString AeraGraphicsItem::makeHtmlLink(
  r_code::Code* object, const ReplicodeObjects& replicodeObjects)
{
  QString label = replicodeObjects.getLabel(object).c_str();
  if (label == "")
    // We don't expect this to happen.
    label = "object_" + QString::number(object->get_detail_oid());

  return "<a href=\"#detail_oid-" + QString::number(object->get_detail_oid()) + "\">" +
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
      adjustItemYPosition();
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

void aera_visualizer::AeraGraphicsItem::adjustItemYPosition()
{
  // Only adjust positions for non-simulation items
  if (AeraVisulizerWindow::simulationEventTypes_.find(getAeraEvent()->eventType_) !=
      AeraVisulizerWindow::simulationEventTypes_.end()) {
    return;
  }
  if (getAeraEvent()->itemInitialTopLeftPosition_ != getAeraEvent()->itemTopLeftPosition_) {
    return;
  }
  // Margin between two items
  float margin = 15;
  // Lowest edge of all colliding items - this is under which we want to move the current item
  float max_y_border = 0;
  auto time = std::chrono::duration_cast<std::chrono::microseconds>(getAeraEvent()->time_ - replicodeObjects_.getTimeReference());
  auto frame_time = getAeraEvent()->time_ - (time % replicodeObjects_.getSamplingPeriod());
  while (true) {
    for (auto it : collidingItems()) {
      // Check whether the colliding item is an AeraGraphicsItem.
      auto valid_item = dynamic_cast<AeraGraphicsItem*>(it);
      if (!valid_item) {
        continue;
      }
      if (valid_item->getAeraEvent()->itemInitialTopLeftPosition_ != valid_item->getAeraEvent()->itemTopLeftPosition_) {
        continue;
      }
      // We do not care for sim-items
      if (AeraVisulizerWindow::simulationEventTypes_.find(valid_item->getAeraEvent()->eventType_) !=
        AeraVisulizerWindow::simulationEventTypes_.end()) {
        continue;
      }
      // If the colliding item comes from a different time-stamp we do not want to adjust the position.
      auto relative_time = std::chrono::duration_cast<std::chrono::microseconds>
        (valid_item->getAeraEvent()->time_ - replicodeObjects_.getTimeReference());
      auto valid_item_frame_time = valid_item->getAeraEvent()->time_ - (relative_time % replicodeObjects_.getSamplingPeriod());
      if (valid_item_frame_time != frame_time) {
        continue;
      }
      // Gets the y-position of the bottom edge of the collding item
      float bottom_border = valid_item->pos().y() + valid_item->boundingRect().height() / 2;
      if (max_y_border < bottom_border) {
        max_y_border = bottom_border;
      }
    }
    if (max_y_border < (pos().y() - boundingRect().height() / 2)) {
      return;
    }
    // Move the current item below the lowest colliding item
    setPos(QPointF(pos().x(), max_y_border + margin + boundingRect().height() / 2));
    getAeraEvent()->itemInitialTopLeftPosition_ = getAeraEvent()->itemTopLeftPosition_;
  }
}

bool AeraGraphicsItem::is_sim()
{
  if (!aeraEvent_->object_ || aeraEvent_->object_->references_size() < 1)
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
    menu->addAction("Focus on This", [=]() { parent_->focusOnItem(this); });
    menu->addAction("Center on This", [=]() { parent_->centerOnItem(this); });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else if (link.startsWith("#detail_oid-")) {
    uint64 detail_oid = link.mid(12).toULongLong();
    auto object = replicodeObjects_.getObjectByDetailOid(detail_oid);
    if (object) {
      if (parent_->getParent()->getAeraGraphicsItem(object)) {
        // Show the menu.
        auto menu = new QMenu();
        menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
          [=]() { parent_->getParent()->zoomToAeraGraphicsItem(object); });
        menu->addAction(QString("Focus on ") + replicodeObjects_.getLabel(object).c_str(),
                        [=]() { parent_->getParent()->focusOnAeraGraphicsItem(object); });
        menu->addAction(QString("Center on ") + replicodeObjects_.getLabel(object).c_str(),
                        [=]() { parent_->getParent()->centerOnAeraGraphicsItem(object); });
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
    // The arrow base and tip pens were given to the Arrow constructor as needed.
    arrow->setPens(
      Arrow::HighlightedPen, arrow->getHighlightArrowBasePen(), arrow->getHighlightArrowTipPen());
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
    arrow->setPens(Arrow::DefaultPen, Arrow::DefaultPen, Arrow::DefaultPen);
    arrow->update();
  }
  foreach(AnchoredHorizontalLine* line, horizontalLines_) {
    line->setPen(AnchoredHorizontalLine::DefaultPen);
    line->update();
  }

  QGraphicsPolygonItem::hoverLeaveEvent(event);
}

void AeraGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  if (mouseEvent->button() == Qt::LeftButton)
  {
    bringToFront();
  }

  QGraphicsItem::mousePressEvent(mouseEvent);
}

void AeraGraphicsItem::TextItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  parent_->parent_->getParent()->textItemHoverMoveEvent(document(), event->pos());

  QGraphicsTextItem::hoverMoveEvent(event);
}

void AeraGraphicsItem::TextItem::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  // Forward mouse click events to the graphics item itself
  parent_->mousePressEvent(mouseEvent);
  QGraphicsTextItem::mousePressEvent(mouseEvent);
}

}
