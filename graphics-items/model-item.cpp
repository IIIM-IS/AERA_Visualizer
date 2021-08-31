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

#include <algorithm>
#include <QRegularExpression>
#include "model-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

ModelItem::ModelItem(
  NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(newModelEvent, replicodeObjects, parent, "Model"),
  newModelEvent_(newModelEvent),
  evidenceCount_(newModelEvent_->object_->code(MDL_CNT).asFloat()),
  successRate_(newModelEvent_->object_->code(MDL_SR).asFloat()),
  strength_(newModelEvent_->object_->code(MDL_STRENGTH).asFloat()),
  evidenceCountColor_("black"), successRateColor_("black"), strengthColor_("black"),
  evidenceCountFlashCountdown_(0), successRateFlashCountdown_(0), strengthFlashCountdown_(0)
{
  // Set up sourceCodeHtml_
  sourceCodeHtml_ = simplifyModelSource(replicodeObjects_.getSourceCode(newModelEvent_->object_));
  addSourceCodeHtmlLinks(newModelEvent_->object_, sourceCodeHtml_);
  highlightLhsAndRhs(sourceCodeHtml_);
  highlightVariables(sourceCodeHtml_);
  sourceCodeHtml_ = htmlify(sourceCodeHtml_);

  setTextItemAndPolygon(makeHtml(), true);
}

QString ModelItem::simplifyModelSource(const string& modelSource)
{
  QString result = modelSource.c_str();
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  result.replace(
    QRegularExpression("[\\s\\n]+\\[[\\w\\s]+\\]([\\s\\n]+[\\d\\.]+){5}[\\s\\n]*\\)$"), ")");

  // TODO: Correctly remove wildcards.
  result.replace(QRegularExpression(" : :\\)"), ")");
  result.replace(QRegularExpression(" :\\)"), ")");

  return result;
}

void ModelItem::highlightLhsAndRhs(QString& html)
{
  // Assume the LHS and RHS are the third and fourth lines, indented by three spaces.
  auto match = QRegularExpression("^(.+\\n.+\\n   )(.+)(\\n   )(.+)").match(html);
  if (match.hasMatch()) {
    // match.captured(1) is the first and second line and indentation of the third line.
    // match.captured(3) is the indentation of the fourth line.
    QString lhs = "<font style=\"background-color:#ffe8e8\">" + match.captured(2) + "</font>";
    QString rhs = "<font style=\"background-color:#e0ffe0\">" + match.captured(4) + "</font>";

    html = match.captured(1) + lhs + match.captured(3) + rhs + html.mid(match.captured(0).size());
  }
}

void ModelItem::highlightVariables(QString& html)
{
  // This won't match if a variable is at the beginning of a string, but we don't expect that.
  // Debug: We also want (\\W) at the end of the regex, but then the match would overlap in "v1 v2".
  html.replace(QRegularExpression("(\\W)(v\\d+\\:?)"), "\\1<font color=\"#c000c0\">\\2</font>");
}

bool ModelItem::getTimingVariables(Code* fact, int& iAfterVariable, int& iBeforeVariable)
{
  if (fact->code(0).asOpcode() != r_exec::Opcodes::Fact ||
    fact->code_size() <= FACT_BEFORE)
    // We don't expect this, but check anyway.
    return false;
  if (!(fact->code(FACT_AFTER).getDescriptor() == Atom::VL_PTR &&
    fact->code(FACT_BEFORE).getDescriptor() == Atom::VL_PTR))
    // They aren't variables.
    return false;

  iAfterVariable = fact->code(FACT_AFTER).asIndex();
  iBeforeVariable = fact->code(FACT_BEFORE).asIndex();
  return true;
}

QString ModelItem::makeHtml()
{
  auto model = newModelEvent_->object_;

  QString html = "";
  html += "<font style=\"color:" + strengthColor_ + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Strength: " +
    QString::number(strength_) + "</font><br>";
  html += "<font style=\"color:" + evidenceCountColor_ + "\">Evidence Count: " +
    QString::number(evidenceCount_) + "</font><br>";
  html += "<font style=\"color:" + successRateColor_ + "\">&nbsp;&nbsp;&nbsp;&nbsp;Success Rate: " +
    QString::number(successRate_) + "</font><br>";
  html += sourceCodeHtml_;
  return html;
}

void ModelItem::updateFromModel()
{
  auto model = newModelEvent_->object_;
  evidenceCountIncreased_ = (model->code(MDL_CNT).asFloat() >= evidenceCount_);
  evidenceCount_ = model->code(MDL_CNT).asFloat();
  successRateIncreased_ = (model->code(MDL_SR).asFloat() >= successRate_);
  successRate_ = model->code(MDL_SR).asFloat();
  strengthIncreased_ = (model->code(MDL_STRENGTH).asFloat() >= strength_);
  strength_ = model->code(MDL_STRENGTH).asFloat();

  refreshText();
}

}
