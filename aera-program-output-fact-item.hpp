#ifndef AERA_PROGRAM_OUTPUT_ITEM_HPP
#define AERA_PROGRAM_OUTPUT_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class AeraProgramOutputFactItem : public AeraGraphicsItem
{
public:
  AeraProgramOutputFactItem(
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
