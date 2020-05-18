#include "arrow.hpp"

#include <qmath.h>
#include <QPen>
#include <QPainter>

namespace aera_visualizer {

Arrow::Arrow(QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem, QGraphicsItem* parent)
  : QGraphicsLineItem(parent)
{
  startItem_ = startItem;
  endItem_ = endItem;
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  color_ = Qt::black;
  setPen(QPen(color_, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
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

  QPen myPen = pen();
  myPen.setColor(color_);
  qreal arrowSize = 10;
  painter->setPen(myPen);
  painter->setBrush(color_);

  QLineF centerLine(startItem_->pos(), endItem_->pos());
  QPointF startIntersectPoint = intersectItem(centerLine, *startItem_);
  QPointF endIntersectPoint = intersectItem(centerLine, *endItem_);

  setLine(QLineF(endIntersectPoint, startIntersectPoint));

  double angle = std::atan2(-line().dy(), line().dx());
  setArrowPointer(arrowHead_, line().p1(), angle);

  painter->drawLine(line());
  painter->drawPolygon(arrowHead_);
  if (isSelected()) {
    painter->setPen(QPen(color_, 1, Qt::DashLine));
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
  const qreal arrowSize = 10;

  QPointF arrowP1 = point + QPointF(sin(angle + M_PI / 3) * arrowSize,
    cos(angle + M_PI / 3) * arrowSize);
  QPointF arrowP2 = point + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
    cos(angle + M_PI - M_PI / 3) * arrowSize);

  polygon.clear();
  polygon << point << arrowP1 << arrowP2;
}

}
