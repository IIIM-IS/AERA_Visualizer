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
#include "model-imdl-prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

ImdlPredictionItem::ImdlPredictionItem(
  ModelImdlPredictionEvent* modelReduction, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(modelReduction, replicodeObjects, parent, "Prediction"),
  modelReduction_(modelReduction), showState_(HIDE_IMDL)
{
  setFactPredFactImdlHtml();
  setTextItemAndPolygon(factPredFactImdlHtml_, true);
}

void ImdlPredictionItem::setFactPredFactImdlHtml()
{
  auto pred = modelReduction_->object_->get_reference(0);
  auto factImdl = pred->get_reference(0);
  auto imdl = factImdl->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factPredSource = regex_replace(replicodeObjects_.getSourceCode(modelReduction_->object_), confidenceAndSaliencyRegex, ")");
  string predSource = regex_replace(replicodeObjects_.getSourceCode(pred), saliencyRegex, ")");
  string factImdlSource = regex_replace(replicodeObjects_.getSourceCode(factImdl), confidenceAndSaliencyRegex, ")");
  string imdlSource = regex_replace(replicodeObjects_.getSourceCode(imdl), saliencyRegex, ")");

  QString predLabel(replicodeObjects_.getLabel(pred).c_str());
  QString factImdlLabel(replicodeObjects_.getLabel(factImdl).c_str());
  QString imdlLabel(replicodeObjects_.getLabel(imdl).c_str());

  QString predHtml = QString(predSource.c_str()).replace(factImdlLabel, DownArrowHtml);
  QString factPredHtml = QString(factPredSource.c_str()).replace(predLabel, predHtml);
  QString factImdlHtml = QString(factImdlSource.c_str()).replace(imdlLabel, DownArrowHtml);
  QString imdlHtml(imdlSource.c_str());

  factPredFactImdlHtml_ = factPredHtml;
  // We will replace !factImdl-start, etc. below, after highlighting.
  factPredFactImdlHtml_ += "\n              !factImdl-start" + factImdlHtml + "!factImdl-end";
  factPredFactImdlHtml_ += "\n                  !imdl-start" + imdlHtml + "!imdl-end";

  addSourceCodeHtmlLinks(modelReduction_->object_, factPredFactImdlHtml_);
  addSourceCodeHtmlLinks(imdl, factPredFactImdlHtml_);
  factPredFactImdlHtml_ = htmlify(factPredFactImdlHtml_);

  highlightedFactPredFactImdlHtml_ = factPredFactImdlHtml_;
  factPredFactImdlHtml_.replace("!factImdl-start", "");
  factPredFactImdlHtml_.replace("!factImdl-end", "");
  factPredFactImdlHtml_.replace("!imdl-start", "");
  factPredFactImdlHtml_.replace("!imdl-end", "");
  // Highlight this the same as the RHS in the instantiated model.
  highlightedFactPredFactImdlHtml_.replace("!factImdl-start", "<font style=\"background-color:#e0ffe0\">");
  highlightedFactPredFactImdlHtml_.replace("!factImdl-end", "</font>");
  highlightedFactPredFactImdlHtml_.replace("!imdl-start", "<font style=\"background-color:#e0ffe0\">");
  highlightedFactPredFactImdlHtml_.replace("!imdl-end", "</font>");
}

void ImdlPredictionItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?", [=]() {
      auto pred = modelReduction_->object_->get_reference(0);
      auto factImdl = pred->get_reference(0);
      auto imdl = factImdl->get_reference(0);

      QString explanation = "Input " + makeHtmlLink(modelReduction_->cause_) +
        " matched the LHS of model " + makeHtmlLink(modelReduction_->predictingModel_) +
        " and the predicted instantiated model on the RHS is a requirement for model " +
        makeHtmlLink(imdl->get_reference(0)) + ".<br><br>";

      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
      });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else
    // For #detail_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
