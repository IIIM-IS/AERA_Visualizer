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
#include <QMenu>
#include "../explanation-log-window.hpp"
#include "../aera-visualizer-window.hpp"
#include "../submodules/AERA/r_exec/factory.h"
#include "aera-visualizer-scene.hpp"
#include "instantiated-composite-state-item.hpp"
#include "expandable-goal-or-pred-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

ExpandableGoalOrPredItem::ExpandableGoalOrPredItem(
  AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects, const QString& prefix,
  AeraVisualizerScene* parent)
: AeraGraphicsItem(aeraEvent, replicodeObjects, parent, "")
{
  setFactGoalOrPredFactValueHtml(prefix);

  // Determine the shape.
  if (getAeraEvent()->object_->get_reference(0)->code(0).asOpcode() == Opcodes::Pred)
    shape_ = SHAPE_PRED;
  else
    shape_ = SHAPE_GOAL;

  setTextItemAndPolygon(valueHtml_, false, shape_);
  setToolTip(toolTipText_);
}

void ExpandableGoalOrPredItem::setFactGoalOrPredFactValueHtml(const QString& prefix)
{
  auto goalOrPred = getAeraEvent()->object_->get_reference(0);
  auto factValue = goalOrPred->get_reference(0);
  auto value = factValue->get_reference(0);
  bool valueIsDrive = (value->code(0).asOpcode() == Opcodes::Ent);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factGoalSource = regex_replace(replicodeObjects_.getSourceCode(getAeraEvent()->object_), confidenceAndSaliencyRegex, ")");
  string goalOrPredSource = regex_replace(replicodeObjects_.getSourceCode(goalOrPred), saliencyRegex, ")");
  string factValueSource = regex_replace(replicodeObjects_.getSourceCode(factValue), confidenceAndSaliencyRegex, ")");
  string valueSource = regex_replace(replicodeObjects_.getSourceCode(value), saliencyRegex, ")");

  QString goalOrPredLabel(replicodeObjects_.getLabel(goalOrPred).c_str());
  QString factValueLabel(replicodeObjects_.getLabel(factValue).c_str());
  QString valueLabel(replicodeObjects_.getLabel(value).c_str());

  QString goalOrPredHtml = QString(goalOrPredSource.c_str()).replace(factValueLabel, DownArrowHtml);
  QString factGoalHtml = QString(factGoalSource.c_str()).replace(goalOrPredLabel, goalOrPredHtml);
  QString factValueHtml, valueHtml;
  if (valueIsDrive) {
    // The value is a single identifier.
    factValueHtml = factValueSource.c_str();
    valueHtml = valueLabel;
  }
  else {
    valueHtml = valueSource.c_str();
    factValueHtml = QString(factValueSource.c_str()).replace(valueLabel, DownArrowHtml);
  }
  
  factGoalOrPredFactValueHtml_ = prefix + " <b><a href=\"#this\">" + replicodeObjects_.getLabel(getAeraEvent()->object_).c_str() + "</a></b>\n";
  if (is_sim()) {
    // All outer facts in a simulation have the same time, so don't show it.
    factGoalOrPredFactValueHtml_ += goalOrPredHtml;
    factGoalOrPredFactValueHtml_ += "\n      " + factValueHtml;
    if (!valueIsDrive)
      factGoalOrPredFactValueHtml_ += "\n          " + valueHtml;
  }
  else {
    factGoalOrPredFactValueHtml_ += factGoalHtml;
    factGoalOrPredFactValueHtml_ += "\n              " + factValueHtml;
    if (!valueIsDrive)
      factGoalOrPredFactValueHtml_ += "\n                  " + valueHtml;
  }

  // Set toolTipText_ before adding links and buttons and other detail.
  toolTipText_ = htmlify(factGoalOrPredFactValueHtml_, true);
  // TODO: Add source code links for references at all levels.
  addSourceCodeHtmlLinks(value, factGoalOrPredFactValueHtml_);

  if (getAeraEvent()->eventType_ == CompositeStateSimulatedPredictionReduction::EVENT_TYPE) {
    // Add the icst inputs.
    auto icstEvent = (CompositeStateSimulatedPredictionReduction*)getAeraEvent();
    QString html = "<br><br>From inputs ";
    for (int i = 0; i < icstEvent->inputs_.size(); ++i) {
      if (i == icstEvent->inputs_.size() - 1)
        html += " and ";
      else if (i > 0)
        html += ", ";

      html += makeHtmlLink(icstEvent->inputs_[i]);
    }
    html += "&nbsp;.";

    factGoalOrPredFactValueHtml_ += html;
  }
  factGoalOrPredFactValueHtml_ = htmlify("down-pointing-triangle " + factGoalOrPredFactValueHtml_, true);
  factGoalOrPredFactValueHtml_.replace("down-pointing-triangle", "<a href=\"#unexpand\">" + DownPointingTriangleHtml + "</a>");

  if (value->code(0).asOpcode() == Opcodes::ICst)
    valueHtml = InstantiatedCompositeStateItem::makeIcstMembersSource(value, replicodeObjects_);
  valueHtml_ = htmlify("right-pointing-triangle " + valueHtml, true);
  if (((_Fact*)factValue)->is_anti_fact())
    valueHtml_ =  "<font color=\"#ff4040\">" + valueHtml_ + "</font>";
  valueHtml_.replace("right-pointing-triangle", "<a href=\"#expand\">" + RightPointingTriangleHtml + "</a>");
}

void ExpandableGoalOrPredItem::textItemLinkActivated(const QString& link)
{
  if (link == "#expand") {
    setTextItemAndPolygon(factGoalOrPredFactValueHtml_, false, shape_);
    setToolTip("");
    bringToFront();
  }
  else if (link == "#unexpand") {
    setTextItemAndPolygon(valueHtml_, false, shape_);
    setToolTip(toolTipText_);
    bringToFront();
  }
  else
    // For #detail_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
