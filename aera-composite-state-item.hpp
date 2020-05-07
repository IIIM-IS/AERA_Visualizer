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

  NewCompositeStateEvent* getNewCompositeStateEvent() { return newCompositeStateEvent_; }

private:
  QString makeHtml();

  NewCompositeStateEvent* newCompositeStateEvent_;
  QString sourceCodeHtml_;
};

}

#endif
