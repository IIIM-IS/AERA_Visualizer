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

  QPolygonF polygon() const { return polygon_; }
  NewMkValPredictionEvent* getPredictionEvent() { return newPredictionEvent_; }

private:
  QString getPredictionSourceCodeHtml(r_code::Code* factPred);
  void setTextItemHtml();

  QPolygonF polygon_;
  NewMkValPredictionEvent* newPredictionEvent_;
  QGraphicsTextItem* textItem_;
  QString sourceCodeHtml_;
};

}

#endif
