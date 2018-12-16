#include "arrow.h"

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
  QPolygonF endPolygon = endItem_->polygon();
  QPointF p1 = endPolygon.first() + endItem_->pos();
  QPointF p2;
  QPointF intersectPoint;
  QLineF polyLine;
  for (int i = 1; i < endPolygon.count(); ++i) {
    p2 = endPolygon.at(i) + endItem_->pos();
    polyLine = QLineF(p1, p2);
    QLineF::IntersectType intersectType =
      polyLine.intersect(centerLine, &intersectPoint);
    if (intersectType == QLineF::BoundedIntersection)
      break;
    p1 = p2;
  }

  setLine(QLineF(intersectPoint, startItem_->pos()));

  double angle = std::atan2(-line().dy(), line().dx());

  QPointF arrowP1 = line().p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
    cos(angle + M_PI / 3) * arrowSize);
  QPointF arrowP2 = line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
    cos(angle + M_PI - M_PI / 3) * arrowSize);

  arrowHead_.clear();
  arrowHead_ << line().p1() << arrowP1 << arrowP2;
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

}
