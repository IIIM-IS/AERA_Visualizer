#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "aera-prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraPredictionItem::AeraPredictionItem(
  QMenu* contextMenu, NewMkValPredictionEvent* newPredictionEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newPredictionEvent, replicodeObjects, parent),
  newPredictionEvent_(newPredictionEvent)
{
  sourceCodeHtml_ = getPredictionSourceCodeHtml(newPredictionEvent_->object_);
  setTextItemAndPolygon(makeHtml());
}

QString AeraPredictionItem::getPredictionSourceCodeHtml(Code* factPred)
{
  auto pred = factPred->get_reference(0);
  auto factMkVal = pred->get_reference(0);
  auto mkVal = factMkVal->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factPredSource = regex_replace(replicodeObjects_.getSourceCode(factPred), confidenceAndSaliencyRegex, ")");
  string predSource = regex_replace(replicodeObjects_.getSourceCode(pred), saliencyRegex, ")");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(factMkVal), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString predLabel(replicodeObjects_.getLabel(pred).c_str());
  QString factMkValLabel(replicodeObjects_.getLabel(factMkVal).c_str());
  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  QString factPredHtml = QString(factPredSource.c_str()).replace(predLabel, "!down");
  QString predHtml = QString(predSource.c_str()).replace(factMkValLabel, "!down");
  QString factMkValHtml = QString(factMkValSource.c_str()).replace(mkValLabel, "!down");
  QString mkValHtml(mkValSource.c_str());

  // Temporarily use "!down" which doesn't have spaces.
  QString html = factPredHtml;
  html += QString("\n      ") + predHtml;
  html += QString("\n            ") + factMkValHtml;
  html += QString("\n                ") + mkValHtml;

  html.replace("\n", "<br>");
  html.replace(" ", "&nbsp;");
  html = html.replace("!down", "<sub><font size=\"+2\"><b>&#129047;</b></font></sub>");
  addSourceCodeHtmlLinks(newPredictionEvent_->object_, html);
  return html;
}

QString AeraPredictionItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Prediction</font> <a href=\"#oid-") +
    QString::number(newPredictionEvent_->object_->get_oid()) + "\">" +
    replicodeObjects_.getLabel(newPredictionEvent_->object_).c_str() + "</h3>";
  html += sourceCodeHtml_;
  return html;
}

}
