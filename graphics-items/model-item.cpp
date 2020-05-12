#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "model-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

ModelItem::ModelItem(
  QMenu* contextMenu, NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newModelEvent, replicodeObjects, parent),
  newModelEvent_(newModelEvent),
  evidenceCount_(newModelEvent_->object_->code(MDL_CNT).asFloat()),
  successRate_(newModelEvent_->object_->code(MDL_SR).asFloat()),
  evidenceCountColor_("black"), successRateColor_("black"),
  evidenceCountFlashCountdown_(0), successRateFlashCountdown_(0)
{
  // Set up sourceCodeHtml_
  sourceCodeHtml_ = htmlify(simplifyModelSource(
    replicodeObjects_.getSourceCode(newModelEvent_->object_)));
  addSourceCodeHtmlLinks(newModelEvent_->object_, sourceCodeHtml_);

  setTextItemAndPolygon(makeHtml());
}

string ModelItem::simplifyModelSource(const string& modelSource)
{
  string result = modelSource;
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(result.begin(), result.end(), '\n', '\x01');
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  result = regex_replace(result,
    regex("[\\s\\x01]+\\[[\\w\\s]+\\]([\\s\\x01]+[\\d\\.]+){5}[\\s\\x01]*\\)$"), ")");
  // TODO: Correctly remove wildcards.
  result = regex_replace(result, regex(" : :\\)"), ")");
  result = regex_replace(result, regex(" :\\)"), ")");
  // Restore \n.
  replace(result.begin(), result.end(), '\x01', '\n');
  return result;
}

QString ModelItem::makeHtml()
{
  auto model = newModelEvent_->object_;

  QString html = QString("<h3><font color=\"darkred\">Model</font> <a href=\"#this\">") + 
    replicodeObjects_.getLabel(model).c_str() + "</a></h3>";
  html += "<font color=\"" + evidenceCountColor_ + "\">Evidence Count: " +
    QString::number(evidenceCount_) + "</font><br>";
  html += "<font color=\"" + successRateColor_ + "\">&nbsp;&nbsp;&nbsp;&nbsp;Success Rate: " +
    QString::number(successRate_) + "</font><br>";
  html += sourceCodeHtml_;
  return html;
}

void ModelItem::updateFromModel()
{
  auto model = newModelEvent_->object_;
  evidenceCountIncreased_ = (model->code(MDL_CNT).asFloat() >= evidenceCount_);
  evidenceCount_ = model->code(MDL_CNT).asFloat();
  successRateIncreased_ = (model->code(MDL_SR).asFloat() >= successRate_);
  successRate_ = model->code(MDL_SR).asFloat();

  textItem_->setHtml(makeHtml());
}

}
