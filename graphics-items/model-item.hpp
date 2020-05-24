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
    NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

  void updateFromModel();

  void setEvidenceCountColor(QString color) 
  { 
    evidenceCountColor_ = color;
    refreshText();
  };

  void setSuccessRateColor(QString color)
  {
    successRateColor_ = color;
    refreshText();
  };

  /**
   * Start with the source from replicodeObjects_.getSourceCode for a model, and
   * remove the set of output groups and parameters, and remove trailing wildcards, and
   * replace a template variable with a wildcard if it is also assigned.
   * \param modelSource The source from replicodeObjects_.getSourceCode.
   * \return The simplified source code
   */
  static QString simplifyModelSource(const std::string& modelSource);

  /**
   * Assume the html is a model with \n line endings, and highlight the left-hand-side
   * and right-hand-side expressions with red and green background.s
   * \param html The model HTML string to modify.
   */
  static void highlightLhsAndRhs(QString& html);

  /**
   * Modify the HTML string to change the font color of variables.
   * \param html The HTML string to modify.
   */
  static void highlightVariables(QString& html);

  int evidenceCountFlashCountdown_;
  bool evidenceCountIncreased_;
  int successRateFlashCountdown_;
  bool successRateIncreased_;

private:
  QString makeHtml();

  /**
   * Set textItem_ to headerHtml_ + makeHtml().
   */
  void refreshText()
  {
    auto html = makeHtml();
    textItem_->setHtml(html);
    // adjustSize() is needed for right-aligned text.
    textItem_->adjustSize();
    textItem_->setHtml(headerHtml_ + html);
    textItem_->adjustSize();
  }

  NewModelEvent* newModelEvent_;
  QString sourceCodeHtml_;
  core::float32 evidenceCount_;
  QString evidenceCountColor_;
  core::float32 successRate_;
  QString successRateColor_;
};

}

#endif
