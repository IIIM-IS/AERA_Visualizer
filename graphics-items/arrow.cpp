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

#include "arrow.hpp"

#include <qmath.h>
#include <QPen>
#include <QPainter>

namespace aera_visualizer {

const QPen Arrow::DefaultPen(Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
const QPen Arrow::HighlightedPen(QColor(0, 128, 255), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

Arrow::Arrow(QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem, QGraphicsItem* parent)
  : QGraphicsLineItem(parent)
{
  startItem_ = startItem;
  endItem_ = endItem;
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setPen(DefaultPen);
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
  path.addPolygon(arrowHead_);
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

  painter->setPen(pen());
  painter->setBrush(pen().color());

  // Add the boundingRect().center() so the arrow points to the center of the item.
  QLineF centerLine(startItem_->pos() + startItem_->boundingRect().center(), 
                    endItem_->pos() +   endItem_->boundingRect().center());
  QPointF startIntersectPoint = intersectItem(centerLine, *startItem_);
  QPointF endIntersectPoint = intersectItem(centerLine, *endItem_);

  setLine(QLineF(endIntersectPoint, startIntersectPoint));

  double angle = std::atan2(-line().dy(), line().dx());
  setArrowPointer(arrowHead_, line().p1(), angle);
  // The tip of the pointer is at the base of the line, moved a little toward the head.
  setArrowPointer(arrowBase_, 
    QPointF(line().p2() + QPointF(sin(angle + -M_PI / 2) * arrowSize_ * 1.7, 
                                  cos(angle + -M_PI / 2) * arrowSize_ * 1.7)),
    angle);

  painter->drawLine(line());
  painter->drawPolygon(arrowHead_);
  painter->drawPolygon(arrowBase_);
  if (isSelected()) {
    painter->setPen(QPen(DefaultPen.color(), 1, Qt::DashLine));
    QLineF myLine = line();
    myLine.translate(0, 4.0);
    painter->drawLine(myLine);
    myLine.translate(0, -8.0);
    painter->drawLine(myLine);
  }
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

void Arrow::setArrowPointer(QPolygonF& polygon, const QPointF& point, double angle)
{
  QPointF arrowP1 = point + QPointF(sin(angle + M_PI / 3) * arrowSize_,
    cos(angle + M_PI / 3) * arrowSize_);
  QPointF arrowP2 = point + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize_,
    cos(angle + M_PI - M_PI / 3) * arrowSize_);

  polygon.clear();
  polygon << point << arrowP1 << arrowP2;
}

}
