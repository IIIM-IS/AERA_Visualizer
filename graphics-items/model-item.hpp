#ifndef MODEL_ITEM_HPP
#define MODEL_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class ModelItem : public AeraGraphicsItem
{
public:
  ModelItem(
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

  /**
   * Start with the source from replicodeObjects_.getSourceCode for a mdl, and
   * remove the set of output groups and parameters, and remove trailing wildcards.
   * \param cstSource The source from replicodeObjects_.getSourceCode.
   * \return The simplified source code
   */
  static std::string simplifyModelSource(const std::string& modelSource);

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
