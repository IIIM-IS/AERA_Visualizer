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

  QPolygonF polygon() const { return polygon_; }
  NewModelEvent* getNewModelEvent() { return newModelEvent_; }
  void updateFromModel();

  void setEvidenceCountColor(QString color) 
  { 
    evidenceCountColor_ = color;
    setTextItemHtml();
  };

  void setSuccessRateColor(QString color)
  {
    successRateColor_ = color;
    setTextItemHtml();
  };

  int evidenceCountFlashCountdown_;
  bool evidenceCountIncreased_;
  int successRateFlashCountdown_;
  bool successRateIncreased_;

private:
  void addSourceCodeHtmlLinks(QString& html);
  void setTextItemHtml();
  void textItemLinkActivated(const QString& link);

  QPolygonF polygon_;
  NewModelEvent* newModelEvent_;
  QGraphicsTextItem* textItem_;
  QString sourceCodeHtml_;
  core::float32 evidenceCount_;
  QString evidenceCountColor_;
  core::float32 successRate_;
  QString successRateColor_;
};

}

#endif
