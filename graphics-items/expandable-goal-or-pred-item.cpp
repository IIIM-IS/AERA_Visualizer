//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2021 Icelandic Institute for Intelligent Machines ses
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
#include <QMenu>
#include "../explanation-log-window.hpp"
#include "../aera-visualizer-window.hpp"
#include "../submodules/replicode/r_exec/factory.h"
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

  // Set toolTipText_ before adding links and buttons.
  toolTipText_ = htmlify(factGoalOrPredFactValueHtml_, true);
  // TODO: Add source code links for references at all levels.
  addSourceCodeHtmlLinks(value, factGoalOrPredFactValueHtml_);
  factGoalOrPredFactValueHtml_ = htmlify("down-pointing-triangle " + factGoalOrPredFactValueHtml_, true);
  factGoalOrPredFactValueHtml_.replace("down-pointing-triangle", "<a href=\"#unexpand\">" + DownPointingTriangleHtml + "</a>");

  if (value->code(0).asOpcode() == Opcodes::ICst)
    valueHtml = InstantiatedCompositeStateItem::makeIcstMembersSource(value, replicodeObjects_);
  valueHtml_ = htmlify("right-pointing-triangle " + valueHtml, true);
  if (((_Fact*)factValue)->is_anti_fact())
    valueHtml_ =  "<font color=\"red\">" + valueHtml_ + "</font>";
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
