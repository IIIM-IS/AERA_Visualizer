#include <regex>
#include <algorithm>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "instantiated-composite-state-item.hpp"
#include "model-item.hpp"
#include "prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

PredictionItem::PredictionItem(
  ModelMkValPredictionReduction* modelReduction, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(modelReduction, replicodeObjects, parent, "Prediction"),
  modelReduction_(modelReduction), showState_(HIDE_IMODEL)
{
  setFactPredFactMkValHtml();
  setFactImdlHtml();
  setBoundAndUnboundModelHtml();
  setTextItemAndPolygon(makeHtml(), true);
}

void PredictionItem::setFactPredFactMkValHtml()
{
  auto pred = modelReduction_->object_->get_reference(0);
  auto factMkVal = pred->get_reference(0);
  auto mkVal = factMkVal->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factPredSource = regex_replace(replicodeObjects_.getSourceCode(modelReduction_->object_), confidenceAndSaliencyRegex, ")");
  string predSource = regex_replace(replicodeObjects_.getSourceCode(pred), saliencyRegex, ")");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(factMkVal), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString predLabel(replicodeObjects_.getLabel(pred).c_str());
  QString factMkValLabel(replicodeObjects_.getLabel(factMkVal).c_str());
  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  QString predHtml = QString(predSource.c_str()).replace(factMkValLabel, DownArrowHtml);
  QString factPredHtml = QString(factPredSource.c_str()).replace(predLabel, predHtml);
  QString factMkValHtml = QString(factMkValSource.c_str()).replace(mkValLabel, DownArrowHtml);
  QString mkValHtml(mkValSource.c_str());

  factPredFactMkValHtml_ = factPredHtml;
  // We will replace !factMkVal-start, etc. below, after highlighting.
  factPredFactMkValHtml_ += "\n              !factMkVal-start" + factMkValHtml + "!factMkVal-end";
  factPredFactMkValHtml_ += "\n                  !mkVal-start" + mkValHtml + "!mkVal-end";

  factPredFactMkValHtml_ = htmlify(factPredFactMkValHtml_);
  addSourceCodeHtmlLinks(modelReduction_->object_, factPredFactMkValHtml_);

  highlightedFactPredFactMkValHtml_ = factPredFactMkValHtml_;
  factPredFactMkValHtml_.replace("!factMkVal-start", "");
  factPredFactMkValHtml_.replace("!factMkVal-end", "");
  factPredFactMkValHtml_.replace("!mkVal-start", "");
  factPredFactMkValHtml_.replace("!mkVal-end", "");
  // Highlight this the same as the RHS in the instantiated model.
  highlightedFactPredFactMkValHtml_.replace("!factMkVal-start", "<font style=\"background-color:#e0ffe0\">");
  highlightedFactPredFactMkValHtml_.replace("!factMkVal-end", "</font>");
  highlightedFactPredFactMkValHtml_.replace("!mkVal-start", "<font style=\"background-color:#e0ffe0\">");
  highlightedFactPredFactMkValHtml_.replace("!mkVal-end", "</font>");
}

void PredictionItem::setFactImdlHtml()
{
  // TODO: Share with InstantiatedCompositeStateItem::setFactIcstHtml().
  auto imdl = modelReduction_->getFactImdl()->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factImdlSource = regex_replace(
    replicodeObjects_.getSourceCode(modelReduction_->getFactImdl()), confidenceAndSaliencyRegex, ")");
  string imdlSource = regex_replace(replicodeObjects_.getSourceCode(imdl), saliencyRegex, ")");

  // Find the set of template values.
  regex templateRegex("^(\\(imdl \\w+ )(\\[[^\\]]+\\])( .+)$");
  smatch matches;
  if (regex_search(imdlSource, matches, templateRegex))
    // Add the link for the template values.
    imdlSource = matches[1].str() + "<a href=\"#imdl-template-values\">" + matches[2].str() + "</a>" + matches[3].str();

  QString imdlLabel(replicodeObjects_.getLabel(imdl).c_str());

  factImdlHtml_ = QString(factImdlSource.c_str()).replace(imdlLabel, DownArrowHtml);
  factImdlHtml_ += QString("\n      ") + imdlSource.c_str();

  factImdlHtml_ = htmlify(factImdlHtml_);
  addSourceCodeHtmlLinks(imdl, factImdlHtml_);
}

void PredictionItem::setBoundAndUnboundModelHtml()
{
  auto imdl = modelReduction_->getFactImdl()->get_reference(0);
  auto mdl = imdl->get_reference(0);

  string imdlSource = replicodeObjects_.getSourceCode(imdl);
  std::vector<string> templateValues;
  std::vector<string> exposedValues;
  InstantiatedCompositeStateItem::getIcstOrImdlValues(imdlSource, templateValues, exposedValues);
  // Debug: How to correctly get the timestamp variables.
  int iAfterVariable = 5;
  int iBeforeVariable = 6;

  QString unboundModelSource = ModelItem::simplifyModelSource(replicodeObjects_.getSourceCode(mdl));
  ModelItem::highlightLhsAndRhs(unboundModelSource);
  string modelSource = unboundModelSource.toStdString();
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(modelSource.begin(), modelSource.end(), '\n', '\x01');

  // Temporarily change the assignment variables so that they are not substituted.
  modelSource = regex_replace(modelSource, regex("\\x01   v"), "\x01   !");

  // Replace backward guards with an empty set.
  modelSource = regex_replace(modelSource, regex("\\x01\\[\\](\\x01   [^\\x01]+)+$"), "\x01|[])");

  // Substitute variables.
  // TODO: Share code with InstantiatedCompositeStateItem::setBoundCstAndMembersHtml()?
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

  ModelItem::highlightVariables(boundModelHtml_);
  ModelItem::highlightVariables(unboundModelHtml_);
}

QString PredictionItem::makeHtml()
{
  QString html = "";
  html += (showState_ == WHAT_MADE_THIS ? highlightedFactPredFactMkValHtml_ : factPredFactMkValHtml_);

  if (showState_ == WHAT_MADE_THIS ||
      showState_ == SHOW_MODEL) {
    if (showState_ == WHAT_MADE_THIS)
      html += "<br><a href=\"#hide-imodel\">" + UnselectedRadioButtonHtml + " Hide iModel</a>" +
        " " + SelectedRadioButtonHtml + " What Made This?" +
        " <a href=\"#show-model\">" + UnselectedRadioButtonHtml + " Show Model</a>";
    else
      html += "<br><a href=\"#hide-imodel\">" + UnselectedRadioButtonHtml + " Hide iModel</a> " +
        " <a href=\"#what-made-this\">" + UnselectedRadioButtonHtml + " What Made This?</a>" +
        " " + SelectedRadioButtonHtml + " Show Model";

    auto imdl = modelReduction_->getFactImdl()->get_reference(0);
    auto mdl = imdl->get_reference(0);
    html += "<br>Input " + makeHtmlLink(modelReduction_->getCause()) +
      " matched the LHS of model " + makeHtmlLink(mdl) + " and the prediction is the RHS of instantiated model <b>" +
      replicodeObjects_.getLabel(modelReduction_->getFactImdl()).c_str() + "</b>:<br>";
    html += factImdlHtml_;
    html += "<br><br>" + (showState_ == WHAT_MADE_THIS ? boundModelHtml_ : unboundModelHtml_);
  }
  else {
    html += "<br>" + SelectedRadioButtonHtml + " Hide iModel" +
      " <a href=\"#what-made-this\">" + UnselectedRadioButtonHtml + " What Made This?</a>" +
      " <a href=\"#show-model\">" + UnselectedRadioButtonHtml + " Show Model</a>";
  }

  return html;
}

void PredictionItem::textItemLinkActivated(const QString& link)
{
  if (link == "#hide-imodel") {
    showState_ = HIDE_IMODEL;
    setTextItemAndPolygon(makeHtml(), true);
    bringToFront();
  }
  else if (link == "#what-made-this") {
    showState_ = WHAT_MADE_THIS;
    setTextItemAndPolygon(makeHtml(), true);
    bringToFront();
  }
  else if (link == "#show-model") {
    showState_ = SHOW_MODEL;
    setTextItemAndPolygon(makeHtml(), true);
    bringToFront();
  }
  else if (link == "#imdl-template-values") {
    Code* requirementFactPred = modelReduction_->getRequirement();
    if (!requirementFactPred)
      return;
    Code* requirementPred = requirementFactPred->get_reference(0);
    Code* factImdl = requirementPred->get_reference(0);
    Code* imdl = factImdl->get_reference(0);
    auto imdlPredictionEvent = (ModelImdlPredictionEvent*)parent_->getParent()->getAeraEvent(
      modelReduction_->imdlPredictionEventIndex_);
    Code* predictingModel = imdlPredictionEvent->predictingModel_;
    // TODO: Share code with setFactPredFactMkValHtml()?
    QString predLabel = replicodeObjects_.getLabel(requirementPred).c_str();
    QString factImdlLabel = replicodeObjects_.getLabel(factImdl).c_str();
    QString imdlLabel = replicodeObjects_.getLabel(imdl).c_str();

    // Strip the ending confidence value and propagation of saliency threshold.
    regex saliencyRegex("\\s+[\\w\\:]+\\)$");
    regex confidenceAndSaliencyRegex("\\s+[\\w\\:]\\s+[\\w\\:]+\\)$");
    string factPredSource = regex_replace(replicodeObjects_.getSourceCode(requirementFactPred), confidenceAndSaliencyRegex, ")");
    string predSource = regex_replace(replicodeObjects_.getSourceCode(requirementPred), saliencyRegex, ")");
    string factImdlSource = regex_replace(replicodeObjects_.getSourceCode(factImdl), confidenceAndSaliencyRegex, ")");
    string imdlSource = regex_replace(replicodeObjects_.getSourceCode(imdl), confidenceAndSaliencyRegex, ")");

    QString predHtml = QString(predSource.c_str()).replace(factImdlLabel, DownArrowHtml);
    QString factPredHtml = QString(factPredSource.c_str()).replace(predLabel, predHtml);
    QString factImdlHtml = QString(factImdlSource.c_str()).replace(imdlLabel, DownArrowHtml);
    QString imdlHtml(imdlSource.c_str());

    QString factPredFactImdlHtml = factPredHtml;
    factPredFactImdlHtml += "\n    <font style=\"background-color:#e0ffe0\">" + factImdlHtml + "</font>";
    factPredFactImdlHtml += "\n        <font style=\"background-color:#e0ffe0\">" + imdlHtml + "</font>";
    factPredFactImdlHtml = htmlify(factPredFactImdlHtml);
    // Debug: Use addSourceCodeHtmlLinks with the prediction object.
    factPredFactImdlHtml.replace("mdl_63", "<a href=\"#debug_oid-592\">mdl_63</a>");

    auto menu = new QMenu();
    menu->addAction("What Made This?", [=]() {
      // TODO: Enable a link to the instantiated model inside the prediction.
      QString explanation = QString("<b>Q: What made the template values of instantiated model <b>") +
        replicodeObjects_.getLabel(modelReduction_->getFactImdl()).c_str() + "</b> ?</b><br>" +
        "The template values were made when model " + makeHtmlLink(predictingModel) +
        " made requirement prediction <a href=\"#requirement_prediction-" +
        QString::number(modelReduction_->imdlPredictionEventIndex_) + "\">" +
        replicodeObjects_.getLabel(requirementFactPred).c_str() + "</a>:<br>" +
        factPredFactImdlHtml + "<br><br>";
      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else
    // For #debug_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
