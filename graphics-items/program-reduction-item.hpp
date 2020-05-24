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
    ProgramReductionEvent* programReductionEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent);

  /**
   * Start with the source from replicodeObjects_.getSourceCode for a mk.rdx, and
   * strip the propagation of saliency threshold.
   * \param mkRdxSource The source from replicodeObjects_.getSourceCode.
   * \return The simplified source code
   */
  static std::string simplifyMkRdxSource(const std::string& mkRdxSource);

private:
  QString makeHtml();

  ProgramReductionEvent* programReductionEvent_;
  QString sourceCodeHtml_;
};

}

#endif
