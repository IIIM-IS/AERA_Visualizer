//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2022-2023 Jeff Thompson
//_/_/ Copyright (c) 2022-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2022-2023 Icelandic Institute for Intelligent Machines
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

#ifndef AERA_GRAPHICS_ITEM_GROUP_HPP
#define AERA_GRAPHICS_ITEM_GROUP_HPP

#include <set>
#include <QGraphicsRectItem>

namespace aera_visualizer {

class AeraGraphicsItem;
class AeraVisualizerScene;

/**
 * AeraGraphicsItemGroup holds a group of AeraGraphicsItem.
 */
class AeraGraphicsItemGroup : public QGraphicsRectItem
{
public:
  /**
   * Create an AeraGraphicsItemGroup.
   * \param parent The parent AeraVisualizerScene.
   * \param title The title shown in the corner.
   * \param color The background color.
   */
  AeraGraphicsItemGroup(AeraVisualizerScene* parent, const QString& title, const QColor& color);

  /**
   * Get the next top (as computed by addChild).
   * \return The next top or qQNaN() if it hasn't been set yet.
   */
  qreal getNextTop() { return nextTop_; }

  void addChild(AeraGraphicsItem* child);

  void removeChild(AeraGraphicsItem* child);

  void fitToChildren();

  /**
   * This is called by the parent scene when its view moves.
   */
  void onParentViewMoved() {
    setItemIsMovable();
  }

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
  /**
   * Check the parent's view port and allow to grab to move the group box
   * if the scene background is visible at the top or bottom. This way the user
   * is not "trapped" if zoomed in too much.
   */
  void setItemIsMovable();

  AeraVisualizerScene* parent_;
  std::set<AeraGraphicsItem*> children_;
  qreal nextTop_;
  bool inCallback_;
};

}

#endif
