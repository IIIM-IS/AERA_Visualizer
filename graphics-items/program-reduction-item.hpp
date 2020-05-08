#ifndef PROGRAM_REDUCTION_ITEM_HPP
#define PROGRAM_REDUCTION_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class ProgramReductionItem : public AeraGraphicsItem
{
public:
  ProgramReductionItem(
    QMenu* contextMenu, ProgramReductionEvent* programReductionEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

private:
  QString makeHtml();

  ProgramReductionEvent* programReductionEvent_;
  QString sourceCodeHtml_;
};

}

#endif
