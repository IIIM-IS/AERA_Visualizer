#ifndef AERA_MODEL_ITEM_HPP
#define AERA_MODEL_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
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

class AeraModelItem : public QGraphicsPolygonItem
{
public:
  enum { Type = UserType + 15 };

  AeraModelItem(QMenu* contextMenu, NewModelEvent* newModelEvent, QGraphicsItem* parent = 0);

  void removeArrows();
  QPolygonF polygon() const { return polygon_; }
  void addArrow(Arrow* arrow);
  int type() const override { return Type; }
  NewModelEvent* getNewModelEvent() { return newModelEvent_; }
  void setConfidence(core::float32 confidence);
  core::float32 getConfidence() { return confidence_; }
  void setConfidenceBrush(QBrush brush) { confidenceLabel_->setBrush(brush); };
  int borderFlashCountdown_;
  int confidenceFlashCountdown_;
  bool confidenceIncreased_;

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
  void removeArrow(Arrow* arrow);

  QPolygonF polygon_;
  QMenu* contextMenu_;
  NewModelEvent* newModelEvent_;
  QList<Arrow*> arrows_;
  core::float32 confidence_;
  QGraphicsSimpleTextItem* confidenceLabel_;
};

}

#endif
