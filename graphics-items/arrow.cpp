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
#include "aera-visualizer-scene.hpp"

#include <qmath.h>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QMenu>

namespace aera_visualizer {

const QPen Arrow::DefaultPen(Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
const QPen Arrow::HighlightedPen(QColor(0, 128, 255), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
const QPen Arrow::GreenArrowheadPen(QColor(0, 220, 0), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
const QPen Arrow::RedArrowheadPen(QColor(255, 0, 0), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

Arrow::Arrow(
  QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem,
   const QPen& highlightArrowBasePen, const QPen& highlightArrowTipPen, AeraVisualizerScene* parent)
: QGraphicsLineItem(),
  highlightArrowBasePen_(highlightArrowBasePen),
  highlightArrowTipPen_(highlightArrowTipPen)
{
  parent_ = parent;
  startItem_ = startItem;
  endItem_ = endItem;
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setPen(DefaultPen);
  arrowBasePen_ = DefaultPen;
  arrowTipPen_ = DefaultPen;
}

void Arrow::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    auto menu = new QMenu();
    menu->addAction("Zoom to Start", [=]() { parent_->zoomToItem(startItem_); });
    menu->addAction("Zoom to End", [=]() { parent_->zoomToItem(endItem_); });
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

  if (isSelected())
    painter->setPen(QPen(pen().color(), 4, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
  else
    painter->setPen(pen());
  painter->setBrush(pen().color());
  painter->drawLine(line());

  painter->setPen(arrowBasePen_);
  painter->setBrush(arrowBasePen_.color());
  painter->drawPolygon(arrowBase_);

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
