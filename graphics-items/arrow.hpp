//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
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

#ifndef ARROW_HPP
#define ARROW_HPP

#include "aera-visualizer-scene.hpp"

#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QPen>

class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QRectF;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneContextMenuEvent;
class QPainterPath;

namespace aera_visualizer {

class Arrow : public QGraphicsLineItem
{
public:
  enum { Type = UserType + 4 };

  /**
   * Create an arrow from the startItem to the endItem.
   * \param startItem The Item for the start of the arrow.
   * \param endItem The Item for the end of the arrow.
   * \param highlightBodyPen This pen is retrieved by getHighlightBodyPen(),
   * but is not otherwise used.
   * \param highlightArrowBasePen This pen is retrieved by getHighlightArrowBasePen(),
   * but is not otherwise used.
   * \param highlightArrowTipPen This pen is retrieved by getHighlightArrowTipPen(),
   * but is not otherwise used.
   * \param parent The parent AeraVisualizerScene.
   */
  Arrow(QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem,
    const QPen& highlightBodyPen, const QPen& highlightArrowBasePen,
    const QPen& highlightArrowTipPen,
    AeraVisualizerScene* parent);

  int type() const override { return Type; }
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  QGraphicsPolygonItem* startItem() const { return startItem_; }
  QGraphicsPolygonItem* endItem() const { return endItem_; }

  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  void showBothSides();
  void moveEndsSideBySide();
  void updatePosition();

  /**
   * Set the pens for the arrow body, base arrowhead and tip arrowhead.
   * You can use getHighlightBodyPen(), getHighlightArrowBasePen() and
   * getHighlightArrowTipPen() as needed.
   */
  void setPens(const QPen& bodyPen, const QPen& arrowBasePen, const QPen& arrowTipPen)
  {
    setPen(bodyPen);
    arrowBasePen_ = arrowBasePen;
    arrowTipPen_ = arrowTipPen;
  }

  /**
   * Get the highlightBodyPen given to the constructor.
   */
  const QPen& getHighlightBodyPen() { return highlightBodyPen_; }

  /**
   * Get the highlightArrowBasePen given to the constructor.
   */
  const QPen& getHighlightArrowBasePen() { return highlightArrowBasePen_; }

  /**
   * Get the highlightArrowTipPen given to the constructor.
   */
  const QPen& getHighlightArrowTipPen() { return highlightArrowTipPen_; }

  static const QPen DefaultPen;
  static const QPen HighlightedPen;
  static const QPen GreenArrowheadPen;
  static const QPen RedArrowheadPen;

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

private:
  /**
   * Return the point where the line intersects the polygon of the item.
   * \param line The line.
   * \param item The item to intersect.
   * \return The intersection point, or QPointF() if the line doesn't intersect the iterm.
   */
  static QPointF intersectItem(const QLineF& line, const QGraphicsPolygonItem& item);

  /**
   * Clear polygon and set it to a polygon with the three points of an arrowhead. 
   * \param polygon The QPolygonF to set. This first clears the polygon.
   * \param point The tip of the arrowhead.
   * \param tip The angle of the arrowhead.
   */
  static void setArrowhead(QPolygonF& polygon, const QPointF& tip, double angle);

  static const int arrowSize_ = 6;
  AeraVisualizerScene* parent_;
  QGraphicsPolygonItem* startItem_;
  QGraphicsPolygonItem* endItem_;
  QPen highlightBodyPen_;
  QPen highlightArrowBasePen_;
  QPen highlightArrowTipPen_;
  QPolygonF arrowBase_;
  QPolygonF arrowTip_;
  QPen arrowBasePen_;
  QPen arrowTipPen_;
  bool wasSelected_;        // Use this to track a "falling edge" on the select state
  QPen startItemLastPen_;   // Used to restore pen settings after deselection
  QPen endItemLastPen_;     // Used to restore pen settings after deselection
};

}

#endif
