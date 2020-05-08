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
    QMenu* contextMenu, AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent);

  void removeArrows();
  void addArrow(Arrow* arrow) { arrows_.append(arrow); }
  AeraEvent* getAeraEvent() { return aeraEvent_; }

  int borderFlashCountdown_;

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  /**
   * Set the textItem_ to the given html and create the border polygon.
   * \param html The HTML for the textItem_.
   */
  void setTextItemAndPolygon(QString html);

  /**
   * Go through object's references and modify html with <a href="#debug_oid-XXX"></a>
   * around the label of each referenced object. When clicked, this link is handled
   * by textItemLinkActivated().
   * \param object The object with the references.
   * \param html The HTML string to modify.
   */
  void addSourceCodeHtmlLinks(r_code::Code* object, QString& html);
  
  virtual void textItemLinkActivated(const QString& link);

  AeraVisualizerScene* parent_;
  ReplicodeObjects& replicodeObjects_;
  QGraphicsTextItem* textItem_;

private:
  void removeArrow(Arrow* arrow);

  QMenu* contextMenu_;
  AeraEvent* aeraEvent_;
  QList<Arrow*> arrows_;
};

}

#endif
