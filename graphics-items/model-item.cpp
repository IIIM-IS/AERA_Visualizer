//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2025 Jeff Thompson
//_/_/ Copyright (c) 2018-2025 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2025 Icelandic Institute for Intelligent Machines
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
  strength_(newModelEvent_->object_->code(MDL_STRENGTH).asFloat()),
  evidenceCount_(newModelEvent_->object_->code(MDL_CNT).asFloat()),
  successRate_(newModelEvent_->object_->code(MDL_SR).asFloat()),
  strengthColor_("black"), evidenceCountColor_("black"), successRateColor_("black"),
  strengthFlashCountdown_(0), evidenceCountFlashCountdown_(0), successRateFlashCountdown_(0)
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
  // "[\\s\\n]+" is whitespace "[\\d\\.]+" is a float value.
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
  // Assume the LHS and RHS are the second and third lines, indented by three spaces.
  auto match = QRegularExpression("^(.+\\n   )(.+)(\\n   )(.+)").match(html);
  if (match.hasMatch()) {
    // match.captured(1) is the first line and indentation of the second line.
    // match.captured(3) is the indentation of the third line.
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
  strengthIncreased_ = (model->code(MDL_STRENGTH).asFloat() >= strength_);
  strength_ = model->code(MDL_STRENGTH).asFloat();
  evidenceCountIncreased_ = (model->code(MDL_CNT).asFloat() >= evidenceCount_);
  evidenceCount_ = model->code(MDL_CNT).asFloat();
  successRateIncreased_ = (model->code(MDL_SR).asFloat() >= successRate_);
  successRate_ = model->code(MDL_SR).asFloat();

  refreshText();
}

}
