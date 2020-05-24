#ifndef PREDICTION_SUCCESS_FACT_ITEM_HPP
#define PREDICTION_SUCCESS_FACT_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class PredictionSuccessFactItem : public AeraGraphicsItem
{
public:
  PredictionSuccessFactItem(
    NewPredictionSuccessEvent* newPredictionSuccessEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent);

protected:
  void textItemLinkActivated(const QString& link) override;

private:
  /**
   * Set factSuccessHtml_ to the HTML source code for the fact and success from
   * newPredictionSuccessEvent_->object_.
   */
  void setFactSuccessHtml();

  NewPredictionSuccessEvent* newPredictionSuccessEvent_;
  QString factSuccessHtml_;
};

}

#endif
