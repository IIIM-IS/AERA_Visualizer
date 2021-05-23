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

#ifndef EXPANDABLE_GOAL_OR_PRED_ITEM_HPP
#define EXPANDABLE_GOAL_OR_PRED_ITEM_HPP

#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

/**
 * An ExpandableGoaOrPredlItem extends AeraGraphicsItem for use in ModelGoalItem, etc. to show
 * the simplified value of a goal or prediction, with a clickable "expand triangle" which expands
 * the item to show the full fact goal/pred fact value.
 */
class ExpandableGoaOrPredlItem : public AeraGraphicsItem
{
public:
  /**
   * Create an ExpandableGoaOrPredlItem, and compute the text of the simplified value of
   * getAeraEvet()->object_ as well as the full  fact goal/pred fact value.
   * \param aeraEvent The AeraEvent with the object_ to display.
   * \param replicodeObjects The ReplicodeObjects used to get the debug OID and label.
   * \param prefix The prefix to put before the fact label of factGoalOrPredFactValueHtml_,
   * for example "Model M6 =>".
   * \param parent The parent AeraVisualizerScene.
   */
  ExpandableGoaOrPredlItem(
    AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects, const QString& prefix,
    AeraVisualizerScene* parent);

protected:
  void textItemLinkActivated(const QString& link) override;

private:
  /**
   * Set factGoalOrPredFactValueHtml_ to the HTML source code for the fact goal/pred fact value
   * from getAeraEvet()->object_, and set toolTipText_ to the value before adding links.
   * Also set valueHtml_ to the HTML source code for the value. These include linked triangle shapes to
   * expand and unexpand, handled by textItemLinkActivated.
   * \param prefix The prefix to put before the fact label of factGoalOrPredFactValueHtml_, e.g. "Model M6 =>".
   */
  void setFactGoalOrPredFactValueHtml(const QString& prefix);

  QString factGoalOrPredFactValueHtml_;
  QString toolTipText_;
  QString valueHtml_;
  Shape shape_;
};

}

#endif
