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
  QString getSourceCodeHtml(r_code::Code* factMkVal);
  QString makeHtml();

  ProgramReductionNewObjectEvent* programReductionNewObjectEvent_;
  QString sourceCodeHtml_;
};

}

#endif
