#ifndef COMPOSITE_STATE_ITEM_HPP
#define COMPOSITE_STATE_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class CompositeStateItem : public AeraGraphicsItem
{
public:
  CompositeStateItem(
    QMenu* contextMenu, NewCompositeStateEvent* newCompositeStateEvent, 
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

private:
  QString makeHtml();

  NewCompositeStateEvent* newCompositeStateEvent_;
  QString sourceCodeHtml_;
};

}

#endif
