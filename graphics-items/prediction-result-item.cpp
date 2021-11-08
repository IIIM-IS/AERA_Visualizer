//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2021 Jeff Thompson
//_/_/ Copyright (c) 2021 Kristinn R. Thorisson
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
//_/_/ Copyright (c) 2021 Icelandic Institute for Intelligent Machines
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
#include "../submodules/AERA/r_exec/opcodes.h"
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "prediction-item.hpp"
#include "prediction-result-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

PredictionResultItem::PredictionResultItem(
  PredictionResultEvent* predictionResultEvent, ReplicodeObjects& replicodeObjects, 
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(predictionResultEvent, replicodeObjects, parent, 
      predictionResultEvent->isSuccess() ? "Prediction Success" : "Prediction Failure"),
  predictionResultEvent_(predictionResultEvent)
{
  setFactOrAntiFactSuccessHtml();
  setTextItemAndPolygon(factOrAntiFactSuccessHtml_, true);
}

void PredictionResultItem::setFactOrAntiFactSuccessHtml()
{
  auto success = predictionResultEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factOrAntiFactSuccessSource = regex_replace(replicodeObjects_.getSourceCode(
    predictionResultEvent_->object_), confidenceAndSaliencyRegex, ")");
  string successSource = regex_replace(replicodeObjects_.getSourceCode(success), saliencyRegex, ")");

  QString successLabel(replicodeObjects_.getLabel(success).c_str());

  factOrAntiFactSuccessHtml_ = QString(factOrAntiFactSuccessSource.c_str()).replace(successLabel, DownArrowHtml);
  factOrAntiFactSuccessHtml_ += QString("\n      ") + successSource.c_str();

  addSourceCodeHtmlLinks(predictionResultEvent_->object_->get_reference(0), factOrAntiFactSuccessHtml_);
  factOrAntiFactSuccessHtml_ = htmlify(factOrAntiFactSuccessHtml_);
}

void PredictionResultItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("Focus on This", [=]() { parent_->focusOnItem(this); });
    menu->addAction("Center on This", [=]() { parent_->centerOnItem(this); });
    menu->addAction("What Made This?", [=]() {
      QString explanation;
      Code* factPrediction = predictionResultEvent_->object_->get_reference(0)->get_reference(0);
      Code* input = predictionResultEvent_->object_->get_reference(0)->get_reference(1);

      if (predictionResultEvent_->isSuccess()) {
        explanation = "<b>Q: What made prediction success " +
          makeHtmlLink(predictionResultEvent_->object_) + " ?</b><br>Input " +
          makeHtmlLink(input) + " was matched against prediction " + makeHtmlLink(factPrediction) +
          " with success.<br><br>";
      }
      else {
        if (input->code(0).asOpcode() == Opcodes::Fact)
          explanation = "<b>Q: What made prediction failure " +
            makeHtmlLink(predictionResultEvent_->object_) + " ?</b><br>The value of input " +
            makeHtmlLink(input) + " failed to match the value in prediction " + makeHtmlLink(factPrediction) +
            ".<br><br>";
        else if (input->code(0).asOpcode() == Opcodes::AntiFact)
          explanation = "<b>Q: What made prediction failure " +
            makeHtmlLink(predictionResultEvent_->object_) + " ?</b><br>Anti-fact " +
            makeHtmlLink(input) + " asserts the absence of a fact to match prediction " + makeHtmlLink(factPrediction) +
            ".<br><br>";
        else
          // We don't expect this.
          return;
      }

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
