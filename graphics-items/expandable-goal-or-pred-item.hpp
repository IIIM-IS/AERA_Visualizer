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

#ifndef EXPANDABLE_GOAL_OR_PRED_ITEM_HPP
#define EXPANDABLE_GOAL_OR_PRED_ITEM_HPP

#include <set>
#include <utility>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

/**
 * An ExpandableGoalOrPredItem extends AeraGraphicsItem for use in ModelGoalItem, etc. to show
 * the simplified value of a goal or prediction, with a clickable "expand triangle" which expands
 * the item to show the full fact goal/pred fact value.
 */
class ExpandableGoalOrPredItem : public AeraGraphicsItem
{
public:
  /**
   * Create an ExpandableGoalOrPredItem, and compute the text of the simplified value of
   * getAeraEvet()->object_ as well as the full  fact goal/pred fact value.
   * \param aeraEvent The AeraEvent with the object_ to display.
   * \param replicodeObjects The ReplicodeObjects used to get the detail OID and label.
   * \param prefix The prefix to put before the fact label of factGoalOrPredFactValueHtml_,
   * for example "Model M6 =>".
   * \param parent The parent AeraVisualizerScene.
   * \param textItemTextColor (optional) The text color when we recreate the textItem_ . If ommitted, use black.
   * \param antiFactHtmlColor (optional) The text color for |fact and |pgm . If ommitted, use red.
   */
  ExpandableGoalOrPredItem(
    AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects, const QString& prefix,
    AeraVisualizerScene* parent, QColor textItemTextColor = Qt::black, const QString& antiFactHtmlColor = "#ff4040");

  /**
   * Add the binding and replace all (var varNumber) with the value in factGoalOrPredFactValueHtml_,
   * toolTipText_ and valueHtml_.
   * \return true if the binding changed, false if not.
   */
  bool setBinding(int varNumber, const QString& value);

  /**
   * Remove the binding and update the text.
   */
  void removeBinding(int varNumber)
  {
    // TODO: Preprocess to know which var numbers this needs.

    auto entry = bindings_.find(varNumber);
    if (entry == bindings_.end())
      return;

    bindings_.erase(entry);
    replaceBindingsFromSaved(saveFactGoalOrPredFactValueHtml_, factGoalOrPredFactValueHtml_);
    replaceBindingsFromSaved(saveToolTipText_, toolTipText_);
    replaceBindingsFromSaved(saveValueHtml_, valueHtml_);

    // TODO: Handle the case when it is expanded.
    setTextItemAndPolygon(valueHtml_, false, shape_);
    setToolTip(toolTipText_);
  }

protected:
  void textItemLinkActivated(const QString& link) override;

private:
  /**
   * Set factGoalOrPredFactValueHtml_ to the HTML source code for the fact goal/pred fact value
   * from getAeraEvet()->object_, and set toolTipText_ to the value before adding links.
   * Also set valueHtml_ to the HTML source code for the value. These include linked triangle shapes to
   * expand and unexpand, handled by textItemLinkActivated.
   * Also set saveFactGoalOrPredFactValueHtml_, saveToolTipText_ and saveValueHtml_ to the respective initial values.
   * \param prefix The prefix to put before the fact label of factGoalOrPredFactValueHtml_, e.g. "Model M6 =>".
   */
  void setFactGoalOrPredFactValueHtml(const QString& prefix);

  void replaceBindingsFromSaved(const QString& saved, QString& result);

  QString factGoalOrPredFactValueHtml_;
  QString toolTipText_;
  QString valueHtml_;
  QString saveFactGoalOrPredFactValueHtml_;
  QString saveToolTipText_;
  QString saveValueHtml_;
  Shape shape_;
  QString antiFactHtmlColor_;
  std::map<int, QString> bindings_;
  int afterVarNumber_;
  int beforeVarNumber_;
};

}

#endif
