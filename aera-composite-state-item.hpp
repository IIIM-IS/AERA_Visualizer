#ifndef AERA_COMPOSITE_STATE_ITEM_HPP
#define AERA_COMPOSITE_STATE_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class AeraCompositeStateItem : public AeraGraphicsItem
{
public:
  AeraCompositeStateItem(
    QMenu* contextMenu, NewCompositeStateEvent* newCompositeStateEvent, 
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

  QPolygonF polygon() const { return polygon_; }
  NewCompositeStateEvent* getNewCompositeStateEvent() { return newCompositeStateEvent_; }

private:
  void setTextItemHtml();

  QPolygonF polygon_;
  NewCompositeStateEvent* newCompositeStateEvent_;
  QGraphicsTextItem* textItem_;
  QString sourceCodeHtml_;
};

}

#endif
