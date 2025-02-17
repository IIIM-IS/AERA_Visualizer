//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2025 Jeff Thompson
//_/_/ Copyright (c) 2018-2025 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2025 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
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

#include "arrow.hpp"
#include "aera-graphics-item.hpp"
#include "aera-visualizer-scene.hpp"

#include <qmath.h>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QGraphicsView>
#include <QMenu>

namespace aera_visualizer {

const QPen Arrow::DefaultPen(Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
const QPen Arrow::HighlightedPen(QColor(0, 128, 255), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
const QPen Arrow::GreenArrowheadPen(QColor(0, 220, 0), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
const QPen Arrow::RedArrowheadPen(QColor(255, 0, 0), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

Arrow::Arrow(
  QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem,
   const QPen& highlightBodyPen, const QPen& highlightArrowBasePen,
   const QPen& highlightArrowTipPen, AeraVisualizerScene* parent)
: QGraphicsLineItem(),
  highlightBodyPen_(highlightBodyPen),
  highlightArrowBasePen_(highlightArrowBasePen),
  highlightArrowTipPen_(highlightArrowTipPen),
  wasSelected_(false),
  startItemLastPen_(DefaultPen),
  endItemLastPen_(DefaultPen)
{
  parent_ = parent;
  startItem_ = startItem;
  endItem_ = endItem;
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setPen(DefaultPen);
  arrowBasePen_ = DefaultPen;
  arrowTipPen_ = DefaultPen;
}

void Arrow::showBothSides()
{
  QRectF boundingRect = startItem_->sceneBoundingRect();
  // United gives a rectangle containing both items
  boundingRect = boundingRect.united(endItem_->sceneBoundingRect());
  if (boundingRect.isValid()) {
    parent_->views().at(0)->fitInView(boundingRect, Qt::KeepAspectRatio);
  }
}

void Arrow::moveEndsSideBySide()
{
  // Scene bounding rect contains the position on the scene
  QPointF position = startItem_->pos();
  QRectF endRect = endItem_->boundingRect();

  // Position the end item {rightOffset} from the right of the start item
  int rightOffset { 50 };
  position.setX(position.x() + startItem_->boundingRect().width() / 2 + endRect.width() / 2 + rightOffset);
  endItem_->setPos(position);

  // Updates the position of other arrows connected to end item
  auto aeraGraphicsEndItem = dynamic_cast<AeraGraphicsItem*>(endItem_);
  if (aeraGraphicsEndItem) {
    aeraGraphicsEndItem->updateArrowsAndLine();
  }

  // Fit the view with the newly positioned side-by-side items
  showBothSides();
}

void Arrow::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  auto menu = new QMenu();

  menu->addAction("Zoom to Start", [=]() { parent_->zoomToItem(startItem_); });
  menu->addAction("Zoom to End", [=]() { parent_->zoomToItem(endItem_); });
  menu->addAction("Focus on Start", [=]() { parent_->focusOnItem(startItem_); });
  menu->addAction("Focus on End", [=]() { parent_->focusOnItem(endItem_); });
  menu->addAction("Center on Start", [=]() { parent_->centerOnItem(startItem_); });
  menu->addAction("Center on End", [=]() { parent_->centerOnItem(endItem_); });
  menu->addAction("Move side-by-side", [=]() { moveEndsSideBySide(); });
  menu->addAction("Show both sides", [=]() { showBothSides(); });

  menu->exec(QCursor::pos() - QPoint(10, 10));

  delete menu;
}

QRectF Arrow::boundingRect() const
{
  qreal extra = (pen().width() + 20) / 2.0;

  return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
    line().p2().y() - line().p1().y()))
    .normalized()
    .adjusted(-extra, -extra, extra, extra);
}

QPainterPath Arrow::shape() const
{
  QPainterPath path = QGraphicsLineItem::shape();
  path.addPolygon(arrowTip_);
  path.addPolygon(arrowBase_);
  return path;
}

void Arrow::updatePosition()
{
  QLineF line(mapFromItem(startItem_, 0, 0), mapFromItem(endItem_, 0, 0));
  setLine(line);
}

void Arrow::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  if (startItem_->collidesWithItem(endItem_))
    return;

  // Add the boundingRect().center() so the arrow points to the center of the item.
  QLineF centerLine(startItem_->pos() + startItem_->boundingRect().center(), 
                    endItem_->pos() +   endItem_->boundingRect().center());
  QPointF startIntersectPoint = intersectItem(centerLine, *startItem_);
  QPointF endIntersectPoint = intersectItem(centerLine, *endItem_);

  setLine(QLineF(endIntersectPoint, startIntersectPoint));

  double angle = std::atan2(-line().dy(), line().dx());
  // The tip of the arrowhead goes into the item a little, so move a little toward the base.
  setArrowhead(
    arrowTip_,
    line().p1() - QPointF(sin(angle + -M_PI / 2),
                          cos(angle + -M_PI / 2)),
    angle);
  // The tip of the arrowhead is at the base of the line, so move a little toward the tip.
  setArrowhead(
    arrowBase_, 
    line().p2() + 10 * QPointF(sin(angle + -M_PI / 2), 
                               cos(angle + -M_PI / 2)),
    angle);

  // If selected, change the border color of the start and end items as well as that of the arrow
  if (isSelected()) {
    // Remember the last pen settings if this is the first pass
    if (!wasSelected_) {
      startItemLastPen_ = startItem_->pen();
      endItemLastPen_ = endItem_->pen();
    }

    // Set the colors for the arrow
    setPens(Arrow::HighlightedPen, getHighlightArrowBasePen(), getHighlightArrowTipPen());
    painter->setPen(Arrow::HighlightedPen);

    // Set the colors for the objects on either end
    startItem_->setPen(Arrow::HighlightedPen);
    endItem_->setPen(Arrow::HighlightedPen);

    // Keep this updated so we can debounce the deselected state
    wasSelected_ = true;

  }
  else {
    // If we just deselected the arrow, recolor the objects at the ends
    if (wasSelected_) {
      // Reset the arrow's pens
      setPens(Arrow::DefaultPen, Arrow::DefaultPen, Arrow::DefaultPen);

      // Reset the end objects' pens
      startItem_->setPen(startItemLastPen_);
      endItem_->setPen(endItemLastPen_);

      // Make sure we only do this once
      wasSelected_ = false;
    }
  }

  // Draw the line
  painter->setPen(pen());
  painter->setBrush(pen().color());
  painter->drawLine(line());

  // Draw the lower arrowhead
  painter->setPen(arrowBasePen_);
  painter->setBrush(arrowBasePen_.color());
  painter->drawPolygon(arrowBase_);

  // Draw the upper arrowhead
  painter->setPen(arrowTipPen_);
  painter->setBrush(arrowTipPen_.color());
  painter->drawPolygon(arrowTip_);
}

QPointF Arrow::intersectItem(const QLineF& line, const QGraphicsPolygonItem& item)
{
  QPolygonF polygon(item.polygon());
  QPointF itemPos(item.pos());

  QPointF p1 = polygon.first() + itemPos;
  QPointF p2;
  QPointF intersectPoint;
  QLineF polyLine;
  for (int i = 1; i < polygon.count(); ++i) {
    p2 = polygon.at(i) + itemPos;
    polyLine = QLineF(p1, p2);
    QLineF::IntersectType intersectType =
      polyLine.intersect(line, &intersectPoint);
    if (intersectType == QLineF::BoundedIntersection)
      return intersectPoint;
    p1 = p2;
  }

  // This shouldn't happen.
  return QPointF();
}

void Arrow::setArrowhead(QPolygonF& polygon, const QPointF& tip, double angle)
{
  QPointF arrowP1 = tip + QPointF(sin(angle + M_PI / 4) * arrowSize_,
    cos(angle + M_PI / 4) * arrowSize_);
  QPointF arrowP2 = tip + QPointF(sin(angle + M_PI - M_PI / 4) * arrowSize_,
    cos(angle + M_PI - M_PI / 4) * arrowSize_);

  polygon.clear();
  polygon << tip << arrowP1 << arrowP2;
}

}
