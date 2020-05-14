#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "instantiated-composite-state-item.hpp"
#include "model-item.hpp"
#include "prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

PredictionItem::PredictionItem(
  QMenu* contextMenu, NewMkValPredictionEvent* newPredictionEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newPredictionEvent, replicodeObjects, parent),
  newPredictionEvent_(newPredictionEvent), showState_(HIDE_MODEL)
{
  setFactPredFactMkValHtml();
  setFactImdlHtml();
  setBoundAndUnboundModelHtml();
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
  // We will replace !mkVal-start and !mkVal-end below, after highlighting.
  factPredFactMkValHtml_ += QString("\n                !mkVal-start") + mkValHtml + "!mkVal-end";

  factPredFactMkValHtml_ = htmlify(factPredFactMkValHtml_);
  factPredFactMkValHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(newPredictionEvent_->object_, factPredFactMkValHtml_);

  highlightedFactPredFactMkValHtml_ = factPredFactMkValHtml_;
  factPredFactMkValHtml_.replace("!mkVal-start", "");
  factPredFactMkValHtml_.replace("!mkVal-end", "");
  // Highlight this the same as the RHS in the instantiated model.
  highlightedFactPredFactMkValHtml_.replace("!mkVal-start", "<font color=\"green\">");
  highlightedFactPredFactMkValHtml_.replace("!mkVal-end", "</font>");
}

void PredictionItem::setFactImdlHtml()
{
  // TODO: Share with InstantiatedCompositeStateItem::setFactIcstHtml().
  auto imdl = newPredictionEvent_->factImdl_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factImdlSource = regex_replace(
    replicodeObjects_.getSourceCode(newPredictionEvent_->factImdl_), confidenceAndSaliencyRegex, ")");
  string imdlSource = regex_replace(replicodeObjects_.getSourceCode(imdl), saliencyRegex, ")");

  QString imdlLabel(replicodeObjects_.getLabel(imdl).c_str());

  // Temporarily use "!down" which doesn't have spaces.
  factImdlHtml_ = QString(factImdlSource.c_str()).replace(imdlLabel, "!down");
  factImdlHtml_ += QString("\n      ") + imdlSource.c_str();

  factImdlHtml_ = htmlify(factImdlHtml_);
  factImdlHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(imdl, factImdlHtml_);
}

void PredictionItem::setBoundAndUnboundModelHtml()
{
  auto factImdl = (_Fact*)newPredictionEvent_->factImdl_;
  auto imdl = factImdl->get_reference(0);
  auto mdl = imdl->get_reference(0);

  string imdlSource = replicodeObjects_.getSourceCode(imdl);
  std::vector<string> templateValues;
  std::vector<string> exposedValues;
  InstantiatedCompositeStateItem::getIcstOrImdlValues(imdlSource, templateValues, exposedValues);
  // Debug: How to correctly get the timestamp variables.
  int iAfterVariable = 5;
  int iBeforeVariable = 6;

  string unboundModelSource = ModelItem::simplifyModelSource(replicodeObjects_.getSourceCode(mdl));
  string modelSource = unboundModelSource;
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(modelSource.begin(), modelSource.end(), '\n', '\x01');

  // Temporarily change the assignment variables so that they are not substituted.
  modelSource = regex_replace(modelSource, regex("\\x01   v"), "\x01   !");

  // Replace backward guards with an empty set.
  modelSource = regex_replace(modelSource, regex("\\x01\\[\\](\\x01   [^\\x01]+)+$"), "\x01|[])");

  // Substitute variables.
  // TODO: Share code with InstantiatedCompositeStateItem::setBoundCstHtml()?
  int iVariable = -1;
  size_t iTemplateValues = 0;
  size_t iExposedValues = 0;
  while (iTemplateValues < templateValues.size() || iExposedValues < exposedValues.size()) {
    // v0, v1, v2, etc. are split between templateValues and exposedValues.
    string boundValue = (iTemplateValues < templateValues.size() ?
      templateValues[iTemplateValues] : exposedValues[iExposedValues]);

    ++iVariable;
    if (iVariable == iAfterVariable)
      ++iVariable;
    if (iVariable == iBeforeVariable)
      ++iVariable;

    string variable = "v" + to_string(iVariable) + ":";
    modelSource = regex_replace(modelSource, regex(variable), variable + boundValue);

    if (iTemplateValues < templateValues.size())
      // Still looking at templateValues.
      ++iTemplateValues;
    else
      ++iExposedValues;
  }

  // Restore assignment variables.
  modelSource = regex_replace(modelSource, regex("\\x01   !"), "\x01   v");
  // Restore \n.
  replace(modelSource.begin(), modelSource.end(), '\x01', '\n');

  unboundModelHtml_ = htmlify(unboundModelSource);
  addSourceCodeHtmlLinks(mdl, unboundModelHtml_);
  boundModelHtml_ = htmlify(modelSource);
  addSourceCodeHtmlLinks(mdl, boundModelHtml_);
  // Debug: Correctly find the RHS.
  QString rhs = "(mk.val&nbsp;v3:b&nbsp;position_y&nbsp;v7:20)";
  boundModelHtml_.replace(rhs, "<font color=\"green\">" + rhs + "</font>");
}

QString PredictionItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Prediction</font> <a href=\"#this\">") +
    replicodeObjects_.getLabel(newPredictionEvent_->object_).c_str() + "</a></h3>";
  html += (showState_ == SHOW_INSTANTIATED_MODEL ? highlightedFactPredFactMkValHtml_ : factPredFactMkValHtml_);

  if (showState_ == SHOW_INSTANTIATED_MODEL ||
      showState_ == SHOW_ORIGINAL_MODEL) {
    if (showState_ == SHOW_INSTANTIATED_MODEL)
      html += "<br><a href=\"#hide-model\">" + UnselectedRadioButtonHtml + " Hide Model</a>" +
        " " + SelectedRadioButtonHtml + " What Made This?" +
        " <a href=\"#show-original-model\">" + UnselectedRadioButtonHtml + " Original Model</a>";
    else
      html += "<br><a href=\"#hide-model\">" + UnselectedRadioButtonHtml + " Hide Model</a> " +
        " <a href=\"#show-instantiated-model\">" + UnselectedRadioButtonHtml + " What Made This?</a>" +
        " " + SelectedRadioButtonHtml + " Original Model";

    html += QString("<br>This prediction was made from instantiated model <b>") +
      replicodeObjects_.getLabel(newPredictionEvent_->factImdl_).c_str() + "</b><br>";
    html += factImdlHtml_;
    html += "<br><br>" + (showState_ == SHOW_INSTANTIATED_MODEL ? boundModelHtml_ : unboundModelHtml_);
  }
  else {
    html += "<br>" + SelectedRadioButtonHtml + " Hide Model" +
      " <a href=\"#show-instantiated-model\">" + UnselectedRadioButtonHtml + " What Made This?</a>" +
      " <a href=\"#show-original-model\">" + UnselectedRadioButtonHtml + " Original Model</a>";
  }

  return html;
}

void PredictionItem::textItemLinkActivated(const QString& link)
{
  if (link == "#hide-model") {
    showState_ = HIDE_MODEL;
    setTextItemAndPolygon(makeHtml());
  }
  else if (link == "#show-instantiated-model") {
    showState_ = SHOW_INSTANTIATED_MODEL;
    setTextItemAndPolygon(makeHtml());
  }
  else if (link == "#show-original-model") {
    showState_ = SHOW_ORIGINAL_MODEL;
    setTextItemAndPolygon(makeHtml());
  }
  else
    // For #debug_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
