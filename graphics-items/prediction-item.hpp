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
    ModelMkValPredictionReduction* modelReduction, ReplicodeObjects& replicodeObjects, 
    AeraVisualizerScene* parent);

protected:
  void textItemLinkActivated(const QString& link) override;

private:
  typedef enum { HIDE_IMODEL, WHAT_MADE_THIS, SHOW_MODEL } ShowState;

  /**
   * Set factPredFactMkValHtml_ to the HTML source code for the fact, pred, fact and mk.val
   * from modelReduction_->object_. Also set highlightedFactPredFactMkValHtml_ to 
   * factPredFactMkValHtml_ with text highlighted in conjunction with boundModelHtml_.
   */
  void setFactPredFactMkValHtml();

  /**
   * Set factImdlHtml_ to the HTML source code for the fact and imdl from modelReduction_->factImdl_.
   */
  void setFactImdlHtml();

  /**
   * Set unboundModelHtml_ to the HTML source code for the model from 
   * modelReduction_->factImdl_, and set boundModelHtml_ to the source 
   * with variables bound to the template parameters.
   */
  void setBoundAndUnboundModelHtml();

   /**
   * Make the full HTML for the textItem_ from factPredFactMkValHtml_ and the object label.
   * Depending on showModel_, also show boundModelHtml_.
   * \return The HTML.
   */
  QString makeHtml();

  ModelMkValPredictionReduction* modelReduction_;
  ShowState showState_;
  QString factPredFactMkValHtml_;
  QString highlightedFactPredFactMkValHtml_;
  QString factImdlHtml_;
  QString boundModelHtml_;
  QString unboundModelHtml_;
};

}

#endif
