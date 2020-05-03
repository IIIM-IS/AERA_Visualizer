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
  void updateFromModel();

  void setEvidenceCountColor(QString color) 
  { 
    evidenceCountColor_ = color;
    setTextItemHtml();
  };

  void setSuccessRateColor(QString color)
  {
    successRateColor_ = color;
    setTextItemHtml();
  };

  int borderFlashCountdown_;
  int evidenceCountFlashCountdown_;
  bool evidenceCountIncreased_;
  int successRateFlashCountdown_;
  bool successRateIncreased_;

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
  void setTextItemHtml();
  void removeArrow(Arrow* arrow);

  static void textItemLinkActivated(const QString& link);

  QPolygonF polygon_;
  QMenu* contextMenu_;
  NewModelEvent* newModelEvent_;
  QList<Arrow*> arrows_;
  QGraphicsTextItem* textItem_;
  core::float32 evidenceCount_;
  QString evidenceCountColor_;
  core::float32 successRate_;
  QString successRateColor_;
};

}

#endif
