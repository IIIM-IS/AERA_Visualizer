#ifndef AERA_PREDICTION_ITEM_HPP
#define AERA_PREDICTION_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class AeraPredictionItem : public AeraGraphicsItem
{
public:
  AeraPredictionItem(
    QMenu* contextMenu, NewMkValPredictionEvent* newPredictionEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

  NewMkValPredictionEvent* getPredictionEvent() { return newPredictionEvent_; }

private:
  QString getPredictionSourceCodeHtml(r_code::Code* factPred);
  QString makeHtml();

  NewMkValPredictionEvent* newPredictionEvent_;
  QString sourceCodeHtml_;
};

}

#endif
