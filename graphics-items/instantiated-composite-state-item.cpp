//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
//_/_/ http://www.iiim.is
//_/_/
//_/_/ --- Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without
//_/_/ modification, is permitted provided that the following conditions
//_/_/ are met:
//_/_/ - Redistributions of source code must retain the above copyright
//_/_/   and collaboration notice, this list of conditions and the
//_/_/   following disclaimer.
//_/_/ - Redistributions in binary form must reproduce the above copyright
//_/_/   notice, this list of conditions and the following disclaimer 
//_/_/   in the documentation and/or other materials provided with 
//_/_/   the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its
//_/_/   contributors may be used to endorse or promote products
//_/_/   derived from this software without specific prior 
//_/_/   written permission.
//_/_/   
//_/_/ - CADIA Clause: The license granted in and to the software 
//_/_/   under this agreement is a limited-use license. 
//_/_/   The software may not be used in furtherance of:
//_/_/    (i)   intentionally causing bodily injury or severe emotional 
//_/_/          distress to any person;
//_/_/    (ii)  invading the personal privacy or violating the human 
//_/_/          rights of any person; or
//_/_/    (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
//_/_/ CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
//_/_/ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//_/_/ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//_/_/ DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//_/_/ CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//_/_/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//_/_/ SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//_/_/ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//_/_/ WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//_/_/ NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//_/_/ OF SUCH DAMAGE.
//_/_/ 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <regex>
#include <algorithm>
#include <QRegularExpression>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "model-item.hpp"
#include "composite-state-item.hpp"
#include "instantiated-composite-state-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

InstantiatedCompositeStateItem::InstantiatedCompositeStateItem(
  NewInstantiatedCompositeStateEvent* newInstantiatedCompositeStateEvent,
  ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(newInstantiatedCompositeStateEvent,
    replicodeObjects,
    parent,
    "Comp. State " + 
    makeHtmlLink(newInstantiatedCompositeStateEvent->object_->get_reference(0)->get_reference(0), replicodeObjects) +
    " " + RightDoubleArrowHtml + "<br>&nbsp;&nbsp;Instantiated Comp. State"),
  newInstantiatedCompositeStateEvent_(newInstantiatedCompositeStateEvent), showState_(HIDE_ICST)
{
  setFactIcstHtml();
  setBoundCstAndMembersHtml();
  setTextItemAndPolygon(makeHtml(), true);
}

void InstantiatedCompositeStateItem::getIcstOrImdlValues(
  const QString& source, QStringList& templateValues, QStringList& exposedValues)
{
  templateValues = QStringList();
  exposedValues = QStringList();

  smatch matches;
  // Debug: Handle the case when a value is also an array or has a string with space or '[' or ']'.
  // (icst cst_61 |[] [b 20] false 1)
  QRegExp ihlpRegEx("^\\(i\\w+ \\w+ \\|?\\[([^\\]]*)\\] \\|?\\[([^\\]]*)\\] [\\w:]+ [\\w:]+\\)$");
  if (source.indexOf(ihlpRegEx) >= 0) {
    if (ihlpRegEx.capturedTexts()[1] != "")
      templateValues = ihlpRegEx.capturedTexts()[1].split(' ');
    if (ihlpRegEx.capturedTexts()[2] != "")
      exposedValues = ihlpRegEx.capturedTexts()[2].split(' ');
  }
}

void InstantiatedCompositeStateItem::setFactIcstHtml()
{
  auto icst = newInstantiatedCompositeStateEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factIcstSource = regex_replace(
    replicodeObjects_.getSourceCode(newInstantiatedCompositeStateEvent_->object_), confidenceAndSaliencyRegex, ")");
  string icstSource = regex_replace(replicodeObjects_.getSourceCode(icst), saliencyRegex, ")");

  QString icstLabel(replicodeObjects_.getLabel(icst).c_str());

  factIcstHtml_ = QString(factIcstSource.c_str()).replace(icstLabel, DownArrowHtml);
  factIcstHtml_ += QString("\n      ") + icstSource.c_str();

  addSourceCodeHtmlLinks(icst, factIcstHtml_);
  factIcstHtml_ = htmlify(factIcstHtml_);
}

void InstantiatedCompositeStateItem::setBoundCstAndMembersHtml()
{
  auto factIcst = (_Fact*)newInstantiatedCompositeStateEvent_->object_;
  auto icst = factIcst->get_reference(0);
  auto cst = icst->get_reference(0);

  string icstSource = replicodeObjects_.getSourceCode(icst);
  QStringList templateValues;
  QStringList exposedValues;
  getIcstOrImdlValues(icstSource.c_str(), templateValues, exposedValues);
  int iAfterVariable;
  int iBeforeVariable;
  auto unpackedCst = cst->get_reference(cst->references_size() - CST_HIDDEN_REFS);
  if (!ModelItem::getTimingVariables(unpackedCst->get_reference(0), iAfterVariable, iBeforeVariable))
    // We don't expect this.
    return;

  string cstSource = CompositeStateItem::simplifyCstSource(replicodeObjects_.getSourceCode(cst));
  // Get just the set of members, which start on the second line and are indented by three spaces.
  string cstMembersSource;
  auto match = QRegularExpression("^.+\\n((   .+\\n)+)").match(cstSource.c_str());
  if (match.hasMatch())
    // Strip the ending \n .
    cstMembersSource = match.captured(1).mid(0, match.captured(1).size() - 1).toStdString();

  // Substitute variables.
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
    cstSource = regex_replace(cstSource, regex(variable), variable + boundValue.toStdString());
    // For boundCstMemberHtml_, don't include the variable.
    cstMembersSource = regex_replace(cstMembersSource, regex(variable), boundValue.toStdString());

    if (iTemplateValues < templateValues.size())
      // Still looking at templateValues.
      ++iTemplateValues;
    else
      ++iExposedValues;
  }

  auto afterVariable = "v" + to_string(iAfterVariable) + ":";
  auto beforeVariable = "v" + to_string(iBeforeVariable) + ":";
  cstSource = regex_replace(cstSource, regex(afterVariable), replicodeObjects_.relativeTime(factIcst->get_after()));
  cstSource = regex_replace(cstSource, regex(beforeVariable), replicodeObjects_.relativeTime(factIcst->get_before()));
  cstMembersSource = regex_replace(cstMembersSource, regex(afterVariable), replicodeObjects_.relativeTime(factIcst->get_after()));
  cstMembersSource = regex_replace(cstMembersSource, regex(beforeVariable), replicodeObjects_.relativeTime(factIcst->get_before()));

  boundCstHtml_ = cstSource.c_str();
  addSourceCodeHtmlLinks(cst, boundCstHtml_);
  boundCstHtml_ = htmlify(boundCstHtml_);
  ModelItem::highlightVariables(boundCstHtml_);

  boundCstMembersHtml_ = cstMembersSource.c_str();
  addSourceCodeHtmlLinks(cst, boundCstMembersHtml_);
  boundCstMembersHtml_ = htmlify(boundCstMembersHtml_);
}

QString InstantiatedCompositeStateItem::makeIcstMembersSource(
  Code* icst, ReplicodeObjects& replicodeObjects, const QString& antiFactHtmlColor)
{
  // TODO: Combine with setBoundCstAndMembersHtml.
  auto cst = icst->get_reference(0);

  string icstSource = replicodeObjects.getSourceCode(icst);
  QStringList templateValues;
  QStringList exposedValues;
  getIcstOrImdlValues(icstSource.c_str(), templateValues, exposedValues);
  int iAfterVariable;
  int iBeforeVariable;
  auto unpackedCst = cst->get_reference(cst->references_size() - CST_HIDDEN_REFS);
  if (!ModelItem::getTimingVariables(unpackedCst->get_reference(0), iAfterVariable, iBeforeVariable))
    // We don't expect this.
    return "";

  string cstSource = CompositeStateItem::simplifyCstSource(replicodeObjects.getSourceCode(cst));
  // Get just the members, which are indented by three spaces. Get the value inside the (fact value ...).
  string cstMembersSource;
  auto i = QRegularExpression("   \\((\\|?fact) (\\([^\\n]+)\\n").globalMatch(cstSource.c_str());
  while (i.hasNext()) {
    auto match = i.next();
    auto value = match.captured(2);
    // Strip the fact timings from the end.
    value = value.mid(0, value.length() - 1);
    value = value.mid(0, value.lastIndexOf(')') + 1);
    if (match.captured(1) == "|fact")
      value = "<font color=\"" + antiFactHtmlColor + "\">" + value + "</font>";

    if (cstMembersSource != "")
      cstMembersSource += "\n";
    cstMembersSource += value.toStdString();
  }

  // Substitute variables.
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
    cstSource = regex_replace(cstSource, regex(variable), variable + boundValue.toStdString());
    // For boundCstMemberHtml_, don't include the variable.
    cstMembersSource = regex_replace(cstMembersSource, regex(variable), boundValue.toStdString());

    if (iTemplateValues < templateValues.size())
      // Still looking at templateValues.
      ++iTemplateValues;
    else
      ++iExposedValues;
  }

  return QString(cstMembersSource.c_str());
}

QString InstantiatedCompositeStateItem::makeHtml()
{
  QString html = "";
  if (showState_ == HIDE_ICST) {
    html += SelectedRadioButtonHtml + " Hide icst" +
      " <a href=\"#what-made-this\">" + UnselectedRadioButtonHtml + " What Made This?</a><br>";
    html += boundCstMembersHtml_;
  }
  else {
    html += "<a href=\"#hide-icst\">" + UnselectedRadioButtonHtml + " Hide icst</a>" +
      " " + SelectedRadioButtonHtml + " What Made This?";

    auto factIcst = newInstantiatedCompositeStateEvent_->object_;
    auto icst = factIcst->get_reference(0);
    auto cst = icst->get_reference(0);

    html += "<br>Inputs ";
    for (int i = 0; i < newInstantiatedCompositeStateEvent_->inputs_.size(); ++i) {
      if (i == newInstantiatedCompositeStateEvent_->inputs_.size() - 1)
        html += " and ";
      else if (i > 0)
        html += ", ";

      html += makeHtmlLink(newInstantiatedCompositeStateEvent_->inputs_[i]);
    }

    html += " matched composite state " + makeHtmlLink(cst) + ", which made instantiated composite state <b>" +
      replicodeObjects_.getLabel(factIcst).c_str() + "</b>:<br>";
    html += factIcstHtml_ + "<br><br>" + boundCstHtml_;
  }

  return html;
}

void InstantiatedCompositeStateItem::textItemLinkActivated(const QString& link)
{
  if (link == "#hide-icst") {
    showState_ = HIDE_ICST;
    setTextItemAndPolygon(makeHtml(), true);
    bringToFront();
  }
  else if (link == "#what-made-this") {
    showState_ = WHAT_MADE_THIS;
    setTextItemAndPolygon(makeHtml(), true);
    bringToFront();
  }
  else
    // For #detail_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
