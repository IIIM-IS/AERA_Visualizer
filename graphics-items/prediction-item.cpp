//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020 Icelandic Institute for Intelligent Machines ses
//_/_/ Vitvelastofnun Islands ses, kt. 571209-0390
//_/_/ Author: Jeffrey Thompson <jeff@iiim.is>
//_/_/
//_/_/ -----------------------------------------------------------------------
//_/_/ Released under Open-Source BSD License with CADIA Clause v 1.0
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/   and collaboration notice, this list of conditions and the 
//_/_/   following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/   notice, this list of conditions and the following
//_/_/   disclaimer in the documentation and/or other materials provided 
//_/_/   with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/   contributors may be used to endorse or promote products 
//_/_/   derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause v 1.0: The license granted in and to the software under 
//_/_/   this agreement is a limited-use license. The software may not be used
//_/_/   in furtherance of: 
//_/_/   (i) intentionally causing bodily injury or severe emotional distress 
//_/_/   to any person; 
//_/_/   (ii) invading the personal privacy or violating the human rights of 
//_/_/   any person; or 
//_/_/   (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

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
  modelReduction_(modelReduction), showState_(HIDE_IMDL)
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

  addSourceCodeHtmlLinks(modelReduction_->object_, factPredFactMkValHtml_);
  factPredFactMkValHtml_ = htmlify(factPredFactMkValHtml_);

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

  addSourceCodeHtmlLinks(imdl, factImdlHtml_);
  factImdlHtml_ = htmlify(factImdlHtml_);
}

void PredictionItem::setBoundAndUnboundModelHtml()
{
  auto imdl = modelReduction_->getFactImdl()->get_reference(0);
  auto mdl = imdl->get_reference(0);

  string imdlSource = replicodeObjects_.getSourceCode(imdl);
  QStringList templateValues;
  QStringList exposedValues;
  InstantiatedCompositeStateItem::getIcstOrImdlValues(imdlSource.c_str(), templateValues, exposedValues);
  int iAfterVariable;
  int iBeforeVariable;
  auto unpackedMdl = mdl->get_reference(mdl->references_size() - MDL_HIDDEN_REFS);
  if (!ModelItem::getTimingVariables(unpackedMdl->get_reference(0), iAfterVariable, iBeforeVariable))
    // We don't expect this.
    return;

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
    QString boundValue = (iTemplateValues < templateValues.size() ?
      templateValues[iTemplateValues] : exposedValues[iExposedValues]);

    ++iVariable;
    if (iVariable == iAfterVariable)
      ++iVariable;
    if (iVariable == iBeforeVariable)
      ++iVariable;

    string variable = "v" + to_string(iVariable) + ":";
    modelSource = regex_replace(modelSource, regex(variable), variable + boundValue.toStdString());

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

  unboundModelHtml_ = unboundModelSource;
  addSourceCodeHtmlLinks(mdl, unboundModelHtml_);
  unboundModelHtml_ = htmlify(unboundModelHtml_);

  boundModelHtml_ = modelSource.c_str();
  addSourceCodeHtmlLinks(mdl, boundModelHtml_);
  boundModelHtml_ = htmlify(boundModelHtml_);

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
      html += "<br><a href=\"#hide-imdl\">" + UnselectedRadioButtonHtml + " Hide imdl</a>" +
        " " + SelectedRadioButtonHtml + " What Made This?" +
        " <a href=\"#show-model\">" + UnselectedRadioButtonHtml + " Show Model</a>";
    else
      html += "<br><a href=\"#hide-imdl\">" + UnselectedRadioButtonHtml + " Hide imdl</a> " +
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
    html += "<br>" + SelectedRadioButtonHtml + " Hide imdl" +
      " <a href=\"#what-made-this\">" + UnselectedRadioButtonHtml + " What Made This?</a>" +
      " <a href=\"#show-model\">" + UnselectedRadioButtonHtml + " Show Model</a>";
  }

  return html;
}

void PredictionItem::textItemLinkActivated(const QString& link)
{
  if (link == "#hide-imdl") {
    showState_ = HIDE_IMDL;
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
    addSourceCodeHtmlLinks(imdl, factPredFactImdlHtml);
    factPredFactImdlHtml = htmlify(factPredFactImdlHtml);

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
