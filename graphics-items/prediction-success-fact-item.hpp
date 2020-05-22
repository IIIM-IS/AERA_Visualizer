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
    QMenu* contextMenu, NewPredictionSuccessEvent* newPredictionSuccessEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

protected:
  void textItemLinkActivated(const QString& link) override;

private:
  /**
   * Set factMkValHtml_ to the HTML source code for the fact and mk.val from
   * autoFocusNewObjectEvent_->object_.
   */
  void setFactMkValHtml();

  /**
   * Make the full HTML for the textItem_ from factMkValHtml_ and the object label.
   * \return The HTML.
   */
  QString makeHtml();

  NewPredictionSuccessEvent* newPredictionSuccessEvent_;
  QString factMkValHtml_;
};

}

#endif
