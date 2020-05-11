#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

PredictionItem::PredictionItem(
  QMenu* contextMenu, NewMkValPredictionEvent* newPredictionEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newPredictionEvent, replicodeObjects, parent),
  newPredictionEvent_(newPredictionEvent)
{
  setFactPredFactMkValHtml();
  setTextItemAndPolygon(makeHtml());
}

void PredictionItem::setFactPredFactMkValHtml()
{
  auto pred = newPredictionEvent_->object_->get_reference(0);
  auto factMkVal = pred->get_reference(0);
  auto mkVal = factMkVal->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factPredSource = regex_replace(replicodeObjects_.getSourceCode(newPredictionEvent_->object_), confidenceAndSaliencyRegex, ")");
  string predSource = regex_replace(replicodeObjects_.getSourceCode(pred), saliencyRegex, ")");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(factMkVal), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString predLabel(replicodeObjects_.getLabel(pred).c_str());
  QString factMkValLabel(replicodeObjects_.getLabel(factMkVal).c_str());
  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  // Temporarily use "!down" which doesn't have spaces.
  QString factPredHtml = QString(factPredSource.c_str()).replace(predLabel, "!down");
  QString predHtml = QString(predSource.c_str()).replace(factMkValLabel, "!down");
  QString factMkValHtml = QString(factMkValSource.c_str()).replace(mkValLabel, "!down");
  QString mkValHtml(mkValSource.c_str());

  factPredFactMkValHtml_ = factPredHtml;
  factPredFactMkValHtml_ += QString("\n      ") + predHtml;
  factPredFactMkValHtml_ += QString("\n            ") + factMkValHtml;
  factPredFactMkValHtml_ += QString("\n                ") + mkValHtml;

  factPredFactMkValHtml_.replace("\n", "<br>");
  factPredFactMkValHtml_.replace(" ", "&nbsp;");
  factPredFactMkValHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(newPredictionEvent_->object_, factPredFactMkValHtml_);
}

QString PredictionItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Prediction</font> <a href=\"#this\">") +
    replicodeObjects_.getLabel(newPredictionEvent_->object_).c_str() + "</a></h3>";
  html += factPredFactMkValHtml_;
  return html;
}

}
