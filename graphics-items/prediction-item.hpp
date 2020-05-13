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
  /**
   * Set factPredFactMkValHtml_ to the HTML source code for the fact, pred, fact and mk.val
   * from newPredictionEvent_->object_.
   * \return The HTML string.
   */
  void setFactPredFactMkValHtml();
  /**
   * Set factImdlHtml_ to the HTML source code for the fact and imdl from newPredictionEvent_->factImdl_.
   * \return The HTML string.
   */
  void setFactImdlHtml();
   /**
   * Set boundModelHtml_ to the HTML source code for the model from 
   * newPredictionEvent_->factImdl_ with variables bound to the template parameters.
   * \return The HTML string.
   */
  void setBoundModelHtml();

   /**
   * Make the full HTML for the textItem_ from factPredFactMkValHtml_ and the object label.
   * \return The HTML.
   */
  QString makeHtml();

  NewMkValPredictionEvent* newPredictionEvent_;
  QString factPredFactMkValHtml_;
  QString factImdlHtml_;
  QString boundModelHtml_;
};

}

#endif
