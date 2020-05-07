#ifndef AERA_MODEL_ITEM_HPP
#define AERA_MODEL_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class AeraModelItem : public AeraGraphicsItem
{
public:
  AeraModelItem(
    QMenu* contextMenu, NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, 
    AeraVisualizerScene* parent);

  void updateFromModel();

  void setEvidenceCountColor(QString color) 
  { 
    evidenceCountColor_ = color;
    textItem_->setHtml(makeHtml());
  };

  void setSuccessRateColor(QString color)
  {
    successRateColor_ = color;
    textItem_->setHtml(makeHtml());
  };

  int evidenceCountFlashCountdown_;
  bool evidenceCountIncreased_;
  int successRateFlashCountdown_;
  bool successRateIncreased_;

private:
  QString makeHtml();

  NewModelEvent* newModelEvent_;
  QString sourceCodeHtml_;
  core::float32 evidenceCount_;
  QString evidenceCountColor_;
  core::float32 successRate_;
  QString successRateColor_;
};

}

#endif
