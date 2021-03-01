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
#include <QRegularExpression>
#include "../explanation-log-window.hpp"
#include "../aera-visualizer-window.hpp"
#include "../submodules/replicode/r_exec/factory.h"
#include "aera-visualizer-scene.hpp"
#include "instantiated-composite-state-item.hpp"
#include "composite-state-prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

CompositeStatePredictionItem::CompositeStatePredictionItem(
  CompositeStateSimulatedPredictionReduction* compositeStateReduction, 
  ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(compositeStateReduction, replicodeObjects, parent, "Goal"),
  compositeStateReduction_(compositeStateReduction)
{
  setFactPredFactValueHtml();

  _Fact* predFact = (_Fact*)compositeStateReduction_->object_->get_reference(0)->get_reference(0);
  qreal targetWidth = 0;
  if (is_sim())
    // Make the width span the full duration of the goal.
    targetWidth = parent->getTimelineX(predFact->get_before()) - parent->getTimelineX(predFact->get_after());

  setTextItemAndPolygon(valueHtml_, false, SHAPE_PRED);
  setToolTip(factPredFactValueHtml_);
}

void CompositeStatePredictionItem::setFactPredFactValueHtml()
{
  auto pred = compositeStateReduction_->object_->get_reference(0);
  auto factValue = pred->get_reference(0);
  auto value = factValue->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factGoalSource = regex_replace(replicodeObjects_.getSourceCode(compositeStateReduction_->object_), confidenceAndSaliencyRegex, ")");
  string predSource = regex_replace(replicodeObjects_.getSourceCode(pred), saliencyRegex, ")");
  string factValueSource = regex_replace(replicodeObjects_.getSourceCode(factValue), confidenceAndSaliencyRegex, ")");
  string valueSource = regex_replace(replicodeObjects_.getSourceCode(value), saliencyRegex, ")");

  QString predLabel(replicodeObjects_.getLabel(pred).c_str());
  QString factValueLabel(replicodeObjects_.getLabel(factValue).c_str());
  QString valueLabel(replicodeObjects_.getLabel(value).c_str());

  QString predHtml = QString(predSource.c_str()).replace(factValueLabel, DownArrowHtml);
  QString factPredHtml = QString(factGoalSource.c_str()).replace(predLabel, predHtml);
  QString factValueHtml = QString(factValueSource.c_str()).replace(valueLabel, DownArrowHtml);
  QString valueHtml = valueSource.c_str();
  
  factPredFactValueHtml_ = "Comp. State " + makeHtmlLink(compositeStateReduction_->compositeState_) + " " + RightArrowHtml + "\n";
  if (is_sim()) {
    // All outer facts in a simulation have the same time, so don't show it.
    factPredFactValueHtml_ += predHtml;
    factPredFactValueHtml_ += "\n      " + factValueHtml;
    factPredFactValueHtml_ += "\n          " + valueHtml;
  }
  else {
    factPredFactValueHtml_ += factPredHtml;
    factPredFactValueHtml_ += "\n              " + factValueHtml;
    factPredFactValueHtml_ += "\n                  " + valueHtml;
  }

  addSourceCodeHtmlLinks(compositeStateReduction_->object_, factPredFactValueHtml_);
  factPredFactValueHtml_ = htmlify(factPredFactValueHtml_, true);

  if (value->code(0).asOpcode() == Opcodes::ICst)
    valueHtml = InstantiatedCompositeStateItem::makeIcstMembersSource(value, replicodeObjects_);
  valueHtml_ = htmlify(valueHtml, true);
}

void CompositeStatePredictionItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });

#if 0
    _Fact* factSuperGoal = modelReduction_->factSuperGoal_;
    _Fact* goalFact = (_Fact*)factSuperGoal->get_reference(0)->get_reference(0);
    if (goalFact->get_reference(0)->code(0).asOpcode() == Opcodes::Ent) {
      // The super goal is a drive.
      menu->addAction("What Made This?", [=]() {

        QString explanation = "<b>Q: What made goal " + makeHtmlLink(modelReduction_->object_) +
          " ?</b><br>Model " + makeHtmlLink(modelReduction_->model_) + 
          " abduced this from drive " +
          // TODO: Make the drive goal pretty.
          QString(replicodeObjects_.getSourceCode(factSuperGoal->get_goal()).c_str()) + "<br><br>";
        parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
      });
    }
#endif

    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else
    // For #debug_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
