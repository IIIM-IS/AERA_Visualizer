//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2022 Jeff Thompson
//_/_/ Copyright (c) 2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2022 Icelandic Institute for Intelligent Machines
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

#include <QGraphicsView>
#include "aera-visualizer-scene.hpp"
#include "aera-graphics-item.hpp"
#include "aera-graphics-item-group.hpp"

using namespace std;

namespace aera_visualizer {

AeraGraphicsItemGroup::AeraGraphicsItemGroup(AeraVisualizerScene* parent, const QString& title, const QColor& color)
: parent_(parent),
  inCallback_(false)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setBrush(color);
  setOpacity(.75);

  auto textItem = new QGraphicsTextItem(this);
  textItem->setHtml("<font size = \"+1\">" + title + "</font>");
}

void AeraGraphicsItemGroup::addChild(AeraGraphicsItem* child)
{
  children_.insert(child);
  fitToChildren();
}

void AeraGraphicsItemGroup::removeChild(AeraGraphicsItem* child)
{
  auto member = children_.find(child);
  if (member != children_.end()) {
    children_.erase(member);
    fitToChildren();
  }
}

void AeraGraphicsItemGroup::setItemIsMovable()
{
  if (parent_->views().size() > 0) {
    auto view = parent_->views().at(0);

    int topYView = view->mapFromScene(0, pos().y() + boundingRect().top()).y();
    int bottomYView = view->mapFromScene(0, pos().y() + boundingRect().bottom()).y();
    setFlag(QGraphicsItem::ItemIsMovable, topYView > 30 || bottomYView < view->height() - 30);
  }
}

QVariant AeraGraphicsItemGroup::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (inCallback_)
    return value;

  if (change == QGraphicsItem::ItemPositionChange) {
    // Only allow change in Y.
    QPointF newToPoint(pos().x(), value.toPointF().y());
    qreal deltaY = newToPoint.y() - pos().y();

    for (auto child = children_.begin(); child != children_.end(); ++child) {
      // Deselect so that it doesn't move with the parent.
      (*child)->setSelected(false);
      (*child)->setPos((*child)->pos().x(), (*child)->pos().y() + deltaY);
      // Keep the initial position relative to this group.
      (*child)->getAeraEvent()->itemInitialTopLeftPosition_ += QPointF(0, deltaY);
    }

    // The move might have obscured the background scene, so setting ItemIsMovable false will move the scene.
    setItemIsMovable();

    return newToPoint;
  }

  return value;
}

void AeraGraphicsItemGroup::fitToChildren()
{
  if (inCallback_)
    return;
 
  inCallback_ = true;
  QRectF childrenRect;
  for (auto child = children_.begin(); child != children_.end(); ++child) {
    if ((*child)->isVisible()) {
      if (childrenRect.width() == 0)
        childrenRect = (*child)->sceneBoundingRect();
      else
        childrenRect = childrenRect.united((*child)->sceneBoundingRect());
    }
  }

  const qreal margin = 10;
  if (childrenRect.width() != 0) {
    setPos(childrenRect.left() - margin, childrenRect.top() - margin);
    setRect(0, 0, childrenRect.width() + 2*margin, childrenRect.height() + 2 * margin);
  }
  inCallback_ = false;
}

}
