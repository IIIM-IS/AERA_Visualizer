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
  : AeraGraphicsItem(contextMenu, newModelEvent, replicodeObjects, parent, "Model"),
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
  highlightVariables(sourceCodeHtml_);

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

  // Get the list of template variables and replace with wildcard as needed.
  // TODO: Correctly get the set of template variables that are assigned.
  set<string> assignedTemplateVariables;
  assignedTemplateVariables.insert("v2:");
  smatch matches;
  if (regex_search(result, matches, regex("^(\\(mdl )(\\[[ :\\w]+\\])"))) {
    // matches[1] is "(mdl ". matches[2] is, e.g., "[vo: v1:]".
    string args = matches[2].str();
    for (auto i = assignedTemplateVariables.begin(); i != assignedTemplateVariables.end(); ++i)
      // A varibale like "v0:" can be used as-is in a regex.
      args = regex_replace(args, regex(*i), ":");

    result = matches[1].str() + args + result.substr(matches[0].str().size());
  }


  // Restore \n.
  replace(result.begin(), result.end(), '\x01', '\n');
  return result;
}

void ModelItem::highlightVariables(QString& html)
{
  // Debug: Use regluare expressions in case a label or string has "v1".
  // First, do variables followed by a colon.
  for (int i = 0; i <= 9; ++i) {
    QString variable = "v" + QString::number(i) + ":";
    html.replace(variable, "<font color=\"green\">" + variable + "</font>");
  }
  for (int i = 0; i <= 9; ++i) {
    QString variable = "v" + QString::number(i);
    html.replace(variable, "<font color=\"green\">" + variable + "</font>");
  }
}


QString ModelItem::makeHtml()
{
  auto model = newModelEvent_->object_;

  QString html = headerHtml_;
  html += "<font style=\"color:" + evidenceCountColor_ + "\">Evidence Count: " +
    QString::number(evidenceCount_) + "</font><br>";
  html += "<font style=\"color:" + successRateColor_ + "\">&nbsp;&nbsp;&nbsp;&nbsp;Success Rate: " +
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
