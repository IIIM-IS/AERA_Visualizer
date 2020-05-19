#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include <QRegularExpression>
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
  sourceCodeHtml_ = simplifyModelSource(replicodeObjects_.getSourceCode(newModelEvent_->object_));
  addSourceCodeHtmlLinks(newModelEvent_->object_, sourceCodeHtml_);
  highlightLhsAndRhs(sourceCodeHtml_);
  highlightVariables(sourceCodeHtml_);
  sourceCodeHtml_ = htmlify(sourceCodeHtml_);

  setTextItemAndPolygon(makeHtml());
}

QString ModelItem::simplifyModelSource(const string& modelSource)
{
  QString result = modelSource.c_str();
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  result.replace(
    QRegularExpression("[\\s\\n]+\\[[\\w\\s]+\\]([\\s\\n]+[\\d\\.]+){5}[\\s\\n]*\\)$"), ")");

  // TODO: Correctly remove wildcards.
  result.replace(QRegularExpression(" : :\\)"), ")");
  result.replace(QRegularExpression(" :\\)"), ")");

  // Get the list of template variables and replace with wildcard as needed.
  // TODO: Correctly get the set of template variables that are assigned.
  set<QString> assignedTemplateVariables;
  assignedTemplateVariables.insert("v2:");
  auto match = QRegularExpression("^(\\(mdl )(\\[[ :\\w]+\\])").match(result);
  if (match.hasMatch()) {
    // match.captured(1) is "(mdl ". match.captured(2) is, e.g., "[v0: v1:]".
    QString args = match.captured(2);
    for (auto i = assignedTemplateVariables.begin(); i != assignedTemplateVariables.end(); ++i)
      args.replace(*i, ":");

    result = match.captured(1) + args + result.mid(match.captured(0).size());
  }

  return result;
}

void ModelItem::highlightLhsAndRhs(QString& html)
{
  // Assume the LHS and RHS are the third and fourth lines, indented by three spaces.
  auto match = QRegularExpression("^(.+\\n.+\\n   )(.+)(\\n   )(.+)").match(html);
  if (match.hasMatch()) {
    // match.captured(1) is the first and second line and indentation of the third line.
    // match.captured(3) is the indentation of the fourth line.
    QString lhs = "<font style=\"background-color:#ffe8e8\">" + match.captured(2) + "</font>";
    QString rhs = "<font style=\"background-color:#e0ffe0\">" + match.captured(4) + "</font>";

    html = match.captured(1) + lhs + match.captured(3) + rhs + html.mid(match.captured(0).size());
  }
}

void ModelItem::highlightVariables(QString& html)
{
  // Debug: Use regluar expressions in case a label or string has "v1".
  // First, do variables followed by a colon.
  for (int i = 0; i <= 9; ++i) {
    QString variable = "v" + QString::number(i) + ":";
    html.replace(variable, "<font color=\"#c000c0\">" + variable + "</font>");
  }
  for (int i = 0; i <= 9; ++i) {
    QString variable = "v" + QString::number(i);
    html.replace(variable, "<font color=\"#c000c0\">" + variable + "</font>");
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
