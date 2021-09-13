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
   * \param highlightArrowBasePen This pen is retrieved by getHighlightArrowBasePen(),
   * but is not otherwise used.
   * \param highlightArrowTipPen This pen is retrieved by getHighlightArrowTipPen(),
   * but is not otherwise used.
   * \param parent The parent AeraVisualizerScene.
   */
  Arrow(QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem,
    const QPen& highlightArrowBasePen, const QPen& highlightArrowTipPen,
    AeraVisualizerScene* parent);

  /**
   * Create an arrow from the startItem to the endItem.
   * getHighlightArrowBasePen( and getHighlightArrowTipPen() will return HighlightedPen.
   * \param startItem The Item for the start of the arrow.
   * \param endItem The Item for the end of the arrow.
   * \param parent The parent AeraVisualizerScene.
   */
  Arrow(QGraphicsPolygonItem* startItem, QGraphicsPolygonItem* endItem,
    AeraVisualizerScene* parent)
  : Arrow(startItem, endItem, HighlightedPen, HighlightedPen, parent) {};

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
   * You can use getHighlightArrowBasePen() and getHighlightArrowTipPen() as needed.
   */
  void setPens(const QPen& bodyPen, const QPen& arrowBasePen, const QPen& arrowTipPen)
  {
    setPen(bodyPen);
    arrowBasePen_ = arrowBasePen;
    arrowTipPen_ = arrowTipPen;
  }

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
  QPen highlightArrowBasePen_;
  QPen highlightArrowTipPen_;
  QPolygonF arrowBase_;
  QPolygonF arrowTip_;
  QPen arrowBasePen_;
  QPen arrowTipPen_;
};

}

#endif
