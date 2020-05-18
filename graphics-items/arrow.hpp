#ifndef ARROW_HPP
#define ARROW_HPP

#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>

class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QRectF;
class QGraphicsSceneMouseEvent;
class QPainterPath;

namespace aera_visualizer {

class Arrow : public QGraphicsLineItem
{
public:
  enum { Type = UserType + 4 };

  Arrow(QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem,
    QGraphicsItem* parent = 0);

  int type() const override { return Type; }
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void setColor(const QColor& color) { color_ = color; }
  QGraphicsPolygonItem* startItem() const { return startItem_; }
  QGraphicsPolygonItem* endItem() const { return endItem_; }

  void updatePosition();

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

private:
  /**
   * Return the point where the line intersects the polygon of the item.
   * \param line The line.
   * \param item The item to intersect.
   * \return The intersection point, or QPointF() if the line doesn't intersect the iterm.
   */
  static QPointF intersectItem(const QLineF& line, const QGraphicsPolygonItem& item);

  /**
   * Clear polygon and set it to a polygon with the three points of an arrow pointer. 
   * \param polygon The QPolygonF to set. This first clears the polygon.
   * \param point The tip of the arrow pointer.
   * \param angle The angle of the arrow pointer.
   */
  static void setArrowPointer(QPolygonF& polygon, const QPointF& point, double angle);

  static const int arrowSize_ = 10;
  QGraphicsPolygonItem* startItem_;
  QGraphicsPolygonItem* endItem_;
  QColor color_;
  QPolygonF arrowHead_;
  QPolygonF arrowBase_;
};

}

#endif
