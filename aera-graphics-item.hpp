#ifndef AERA_GRAPHICS_ITEM_HPP
#define AERA_GRAPHICS_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "replicode-objects.hpp"
#include "aera-event.hpp"

class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
class QGraphicsSimpleTextItem;

namespace aera_visualizer {

class Arrow;
class AeraVisualizerScene;

class AeraGraphicsItem : public QGraphicsPolygonItem
{
public:
  AeraGraphicsItem(
    QMenu* contextMenu, AeraEvent* newObjectEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent);

  void removeArrows();
  void addArrow(Arrow* arrow) { arrows_.append(arrow); }
  AeraEvent* getNewObjectEvent() { return newObjectEvent_; }

  int borderFlashCountdown_;

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
  void textItemLinkActivated(const QString& link);

  AeraVisualizerScene* parent_;
  ReplicodeObjects& replicodeObjects_;

private:
  void removeArrow(Arrow* arrow);

  QMenu* contextMenu_;
  AeraEvent* newObjectEvent_;
  QList<Arrow*> arrows_;
};

}

#endif
