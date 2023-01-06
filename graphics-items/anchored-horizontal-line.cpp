//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2023 Jeff Thompson
//_/_/ Copyright (c) 2018-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2023 Icelandic Institute for Intelligent Machines
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

#include "anchored-horizontal-line.hpp"

#include <qmath.h>
#include <QPen>
#include <QPainter>

namespace aera_visualizer {

const QPen AnchoredHorizontalLine::DefaultPen(QColor(200, 200, 200), 1, Qt::DashLine, Qt::SquareCap);
const QPen AnchoredHorizontalLine::HighlightedPen(QColor(240, 240, 0), 3, Qt::SolidLine, Qt::SquareCap);

AnchoredHorizontalLine::AnchoredHorizontalLine(
  QGraphicsPolygonItem* item, qreal left, qreal right, QGraphicsItem* parent)
  : QGraphicsLineItem(parent)
{
  item_ = item;
  left_ = left;
  right_ = right;
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setPen(DefaultPen);
}

QRectF AnchoredHorizontalLine::boundingRect() const
{
  qreal extra = (pen().width() + 20) / 2.0;

  return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
    line().p2().y() - line().p1().y()))
    .normalized()
    .adjusted(-extra, -extra, extra, extra);
}

QPainterPath AnchoredHorizontalLine::shape() const
{
  QPainterPath path = QGraphicsLineItem::shape();
  path.addPolygon(leftVerticalBar_);
  path.addPolygon(rightVerticalBar_);
  return path;
}

void AnchoredHorizontalLine::updatePosition()
{
  if (item_->x() + item_->boundingRect().right() <= left_ ||
      item_->x() + item_->boundingRect().left() >= right_)
    // The item has been moved beyond the boundaries of the line.
    setVisible(false);
  else
    // Debug: This will set the item visible even if someone else hid it.
    setVisible(true);

  // TODO: Correct end points.
  QPointF itemCenter = item_->pos() + item_->boundingRect().center();
  QLineF line(QPointF(left_, itemCenter.y()), QPointF(right_, itemCenter.y()));
  setLine(line);
}

void AnchoredHorizontalLine::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  painter->setPen(pen());
  painter->setBrush(pen().color());

  // Add the boundingRect().center() so the line goes through the center of the item.
  QPointF itemCenter = item_->pos() + item_->boundingRect().center();
  QLineF centerLine(QPointF(left_, itemCenter.y()), QPointF(right_, itemCenter.y()));
  setLine(centerLine);

  leftVerticalBar_.clear();
  leftVerticalBar_ << QPointF(centerLine.x1(), centerLine.y1() - verticalBarRadius_)
                   << QPointF(centerLine.x1(), centerLine.y1() + verticalBarRadius_);
  rightVerticalBar_.clear();
  rightVerticalBar_ << QPointF(centerLine.x2(), centerLine.y2() - verticalBarRadius_)
                    << QPointF(centerLine.x2(), centerLine.y2() + verticalBarRadius_);

  painter->drawLine(line());
  painter->drawPolygon(leftVerticalBar_);
  painter->drawPolygon(rightVerticalBar_);
}

QPointF AnchoredHorizontalLine::intersectItem(const QLineF& line, const QGraphicsPolygonItem& item)
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

}
