#ifndef AERA_PROGRAM_REDUCTION_ITEM_HPP
#define AERA_PROGRAM_REDUCTION_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class AeraProgramReductionItem : public AeraGraphicsItem
{
public:
  AeraProgramReductionItem(
    QMenu* contextMenu, ProgramReductionEvent* programReductionEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

private:
  QString makeHtml();

  ProgramReductionEvent* programReductionEvent_;
  QString sourceCodeHtml_;
};

}

#endif
