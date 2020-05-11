#ifndef INSTANTIATED_COMPOSITE_STATE_ITEM_HPP
#define INSTANTIATED_COMPOSITE_STATE_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class InstantiatedCompositeStateItem : public AeraGraphicsItem
{
public:
  InstantiatedCompositeStateItem(
    QMenu* contextMenu, NewInstantiatedCompositeStateEvent* newInstantiatedCompositeStateEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

private:
  /**
   * Set factIcstHtml_ to the HTML source code for the fact and icst from newInstantiatedCompositeStateEvent_->object_.
   * \return The HTML string.
   */
  void setFactIcstHtml();
  /**
   * Make the full HTML for the textItem_ from factIcstHtml_ and the object label.
   * \return The HTML.
   */
  QString makeHtml();

  NewInstantiatedCompositeStateEvent* newInstantiatedCompositeStateEvent_;
  QString factIcstHtml_;
};

}

#endif
