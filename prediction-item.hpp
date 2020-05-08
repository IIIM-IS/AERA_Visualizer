#ifndef PREDICTION_ITEM_HPP
#define PREDICTION_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class PredictionItem : public AeraGraphicsItem
{
public:
  PredictionItem(
    QMenu* contextMenu, NewMkValPredictionEvent* newPredictionEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

private:
  QString getPredictionSourceCodeHtml(r_code::Code* factPred);
  QString makeHtml();

  NewMkValPredictionEvent* newPredictionEvent_;
  QString sourceCodeHtml_;
};

}

#endif
