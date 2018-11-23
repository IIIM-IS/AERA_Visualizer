#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsLineItem>

#include "aera-model-item.h"

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

  Arrow(AeraModelItem* startItem, AeraModelItem* endItem,
    QGraphicsItem* parent = 0);

  int type() const override { return Type; }
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void setColor(const QColor& color) { color_ = color; }
  AeraModelItem* startItem() const { return startItem_; }
  AeraModelItem* endItem() const { return endItem_; }

  void updatePosition();

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

private:
  AeraModelItem* startItem_;
  AeraModelItem* endItem_;
  QColor color_;
  QPolygonF arrowHead_;
};

}

#endif
