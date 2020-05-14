#ifndef PROGRAM_OUTPUT_ITEM_HPP
#define PROGRAM_OUTPUT_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class ProgramOutputFactItem : public AeraGraphicsItem
{
public:
  ProgramOutputFactItem(
    QMenu* contextMenu, ProgramReductionNewObjectEvent* programReductionNewObjectEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

protected:
  void textItemLinkActivated(const QString& link) override;

private:
  /**
   * Set factMkValHtml_ to the HTML source code for the fact and mk.val from 
   * programReductionNewObjectEvent_->object_.
   */
  void setFactMkValHtml();

  /**
   * Set explanationMkRdxHtml_ to the HTML source code for the mk.rdx from
   * programReductionNewObjectEvent_->programReduction_, meant for the explanation log.
   */
  void setExplanationMkRdxHtml();

  /**
   * Make the full HTML for the textItem_ from factMkValHtml_ and the object label.
   * \return The HTML.
   */
  QString makeHtml();

  ProgramReductionNewObjectEvent* programReductionNewObjectEvent_;
  QString factMkValHtml_;
  std::string explanationMkRdxHtml_;
};

}

#endif
