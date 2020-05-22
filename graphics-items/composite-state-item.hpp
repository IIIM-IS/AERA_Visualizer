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

  /**
   * Start with the source from replicodeObjects_.getSourceCode for a cst, and
   * remove the set of output groups and parameters, and remove trailing wildcards.
   * \param cstSource The source from replicodeObjects_.getSourceCode.
   * \return The simplified source code
   */
  static std::string simplifyCstSource(const std::string& cstSource);

private:
  NewCompositeStateEvent* newCompositeStateEvent_;
  QString sourceCodeHtml_;
};

}

#endif
