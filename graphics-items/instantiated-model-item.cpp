//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2023 Jeff Thompson
//_/_/ Copyright (c) 2018-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2023 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2023 Chloe Schaff
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
#include "aera-visualizer-window.hpp"
#include "instantiated-model-item.hpp"
#include <QMenu>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"

using namespace std;
using namespace r_code;

namespace aera_visualizer {

ImdlItem::ImdlItem(
  NewInstantiatedModelEvent* imdl, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(imdl,
    replicodeObjects,
    parent,
    "Model " + makeHtmlLink(imdl->baseModel_, replicodeObjects) +
    " " + RightDoubleArrowHtml + "<br>&nbsp;&nbsp;Instantiated Model "),
  imdl_(imdl),
  showState_(HIDE_IMDL),
  replicodeObjects_(replicodeObjects)
{
  // Extract the 2nd set of bracketed valuesfrom an expression of the form
  //    (imdl mdl_74 [10 0s:300ms:0us 0s:400ms:0us] [b v4: v5: v6: v7:] : :)
  smatch matches;
  regex templateValsRegex("\\[[\\w\\s\\.:-]*\\] (\\[[\\w\\s\\.:-]*\\])");
  std::string sourceCode = replicodeObjects_.getSourceCode(imdl_->imdl_);
  if (regex_search(sourceCode, matches, templateValsRegex)) {
    templateVals_ = QString::fromStdString(matches[1].str());
  }

  // Build the explanation HTML
  explanation_ = "Input " + makeHtmlLink(imdl_->getCause(), replicodeObjects_) + " matched the LHS of model " +
    makeHtmlLink(imdl_->baseModel_, replicodeObjects_) + " and was instantiated with template values " +
    templateVals_ + "<br>";

  // Set up the graphics item
  setTextItemAndPolygon(makeHtml(), true);
}


QString ImdlItem::makeHtml()
{
  QString html;
  QString imdlSource = QString::fromStdString(replicodeObjects_.getSourceCode(imdl_->imdl_));
  QString baseMdlLabel = QString::fromStdString(replicodeObjects_.getLabel(imdl_->baseModel_));

  if (showState_ == HIDE_IMDL) {
    html += SelectedRadioButtonHtml + " Hide imdl" +
      " <a href=\"#what-made-this\">" + UnselectedRadioButtonHtml + " What made this?</a><br>";

    html += imdlSource.replace(baseMdlLabel, makeHtmlLink(imdl_->baseModel_, replicodeObjects_));

  }
  else if (showState_ == WHAT_MADE_THIS) {
    html += "<a href=\"#hide-imdl\">" + UnselectedRadioButtonHtml + " Hide imdl</a>" +
      " " + SelectedRadioButtonHtml + " What made this?<br>";
    
    html += explanation_;

    // Present the fact and imdl
    QString imdlLabel = QString::fromStdString(replicodeObjects_.getLabel(imdl_->imdl_));
    QString factImdl = QString::fromStdString(replicodeObjects_.getSourceCode(imdl_->factImdl_));

    html += factImdl.replace(imdlLabel, DownArrowHtml);
    html += htmlify(QString("\n      "));
    html += imdlSource.replace(baseMdlLabel, makeHtmlLink(imdl_->baseModel_, replicodeObjects_));

  }

  return html;
}


void ImdlItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("Focus on This", [=]() { parent_->focusOnItem(this); });
    menu->addAction("Center on This", [=]() { parent_->centerOnItem(this); });
    menu->addAction("What Made This?", [=]() {     
      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation_);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else if (link == "#hide-imdl") {
    showState_ = HIDE_IMDL;
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
