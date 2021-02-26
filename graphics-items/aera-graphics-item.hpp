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

#ifndef AERA_GRAPHICS_ITEM_HPP
#define AERA_GRAPHICS_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "../replicode-objects.hpp"
#include "aera-event.hpp"

class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
class QGraphicsSimpleTextItem;

namespace aera_visualizer {

class Arrow;
class AnchoredHorizontalLine;
class AeraVisualizerScene;

/**
 * AeraGraphicsItem is a base class for specific graphics items like ModelItem.
 */
class AeraGraphicsItem : public QGraphicsPolygonItem
{
public:
  /**
   * Create an AeraGraphicsItem, and set headerHtml_ to a header with aeraEvent_->time_ and 
   * headerPrefix + the aeraEvent label with a "#this" link.
   * \param aeraEvent The AeraEvent which is used to get the time_ for the header and the itemTopLeftPosition_.
   * \param replicodeObjects The ReplicodeObjects used to get the debug OID and label.
   * \param parent The parent AeraVisualizerScene.
   * \param headerPrefix The prefix for headerHtml_ as described above.
   */
  AeraGraphicsItem(
    AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent, const QString& headerPrefix = "");

  void removeArrowsAndHorizontalLines();
  void addArrow(Arrow* arrow) { arrows_.append(arrow); }
  void addHorizontalLine(AnchoredHorizontalLine* line) { horizontalLines_.append(line); }
  AeraEvent* getAeraEvent() { return aeraEvent_; }

  /**
   * Change the Z value of this item to be slightly greater than other colliding AeraGraphicsItems.
   */
  void bringToFront();

  /**
   * Change the Z value of this item to be slightly less than other colliding AeraGraphicsItems.
   */
  void sendToBack();

  /**
   * Reset the position to aeraEvent_->itemInitialTopLeftPosition_.
   */
  void resetPosition();

  /**
   * Replace all "\n" or "\x01" with "<br>" and extra " " with "&nbsp;".
   * \param input The input string to htmlify.
   * \return The HTML string.
   */
  static QString htmlify(const QString& input);

  static QString htmlify(const std::string& input) { return htmlify(QString(input.c_str())); }

  /**
   * Make an HTML string with <a href="#debug_oid-XXX"></a> around the label the object. 
   * When clicked, this link is handled by textItemLinkActivated().
   * \param object The object to be linked.
   * \param replicodeObjects the ReplicodeObjects for looking up labels. This is passed
   * as a param so that this can be a static method. If you have an AeraGraphicsItem object,
   * you can call the makeHtmlLink member method.
   */
  static QString makeHtmlLink(r_code::Code* object, const ReplicodeObjects& replicodeObjects);

  QString makeHtmlLink(r_code::Code* object)
  {
    return makeHtmlLink(object, replicodeObjects_);
  }

  /**
   * Go through object's references and modify html with <a href="#debug_oid-XXX"></a>
   * around the label of each referenced object. When clicked, this link is handled
   * by textItemLinkActivated().
   * \param object The object with the references.
   * \param html The HTML string to modify.
   * \param replicodeObjects the ReplicodeObjects for looking up labels. This is passed
   * as a param so that this can be a static method. If you have an AeraGraphicsItem object,
   * you can call the addSourceCodeHtmlLinks member method.
   */
  static void addSourceCodeHtmlLinks(
    r_code::Code* object, QString& html, const ReplicodeObjects& replicodeObjects);

  void addSourceCodeHtmlLinks(r_code::Code* object, QString& html)
  {
    addSourceCodeHtmlLinks(object, html, replicodeObjects_);
  }

  /**
   * Set the the visible state of this item and the connected arrows and anchored horizontal lines.
   * \param visible The visible state.
   */
  void setItemAndArrowsAndHorizontalLinesVisible(bool visible);

  const QPen& getBorderNoHighlightPen() { return borderNoHighlightPen_;  }

  /**
   * Check if getAeraEvent()->object_ is a simulated goal or prediction. This is meant
   * to imitate the Replicode is_sim operator.
   * \return True if the object is simulated.
   */
  bool is_sim();

  static const QString DownArrowHtml;
  static const QString RightArrowHtml;
  static const QString SelectedRadioButtonHtml;
  static const QString UnselectedRadioButtonHtml;

  int borderFlashCountdown_;

protected:
  typedef enum { SHAPE_RECTANGLE, SHAPE_GOAL, SHAPE_PRED } Shape;

  /**
   * AeraGraphicsItem::TextItem extends QGraphicsTextItem so that we can override its
   * hoverMoveEvent.
   */
  class TextItem : public QGraphicsTextItem {
  public:
    TextItem(AeraGraphicsItem* parent)
      : QGraphicsTextItem(parent), parent_(parent)
    {}

    AeraGraphicsItem* parent_;

  protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
  };
  friend TextItem;

  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

  /**
   * Set the textItem_ to the given html and create the border polygon. Connect
   * the textItem_ to textItemLinkActivated, with default behavior which a derived class can override.
   * \param html The HTML for the textItem_.
   * \param prependHeaderHtml If false, use html as-is. If true, first set the text 
   * to html and adjust the size, then set the text to headerHtml_+html. We do this because
   * the header has a right-aligned table cell which needs to "know" the width of the text.
   * \param shape (optional) The shape of the item. If omitted, use SHAPE_RECTANGLE.
   * \param targetWidth (optional) The target screen width of the item. If omitted of if this is 
   * less than the default width based on the item contents, then it is ignored.
   */
  void setTextItemAndPolygon(QString html, bool prependHeaderHtml, Shape shape = SHAPE_RECTANGLE, qreal targetWidth = 0);

  virtual void textItemLinkActivated(const QString& link);

  AeraVisualizerScene* parent_;
  ReplicodeObjects& replicodeObjects_;
  QString headerHtml_;
  TextItem* textItem_;
  QPen borderNoHighlightPen_;

private:
  void removeArrow(Arrow* arrow);
  void removeHorizontalLine(AnchoredHorizontalLine* line);

  AeraEvent* aeraEvent_;
  QList<Arrow*> arrows_;
  QList<AnchoredHorizontalLine*> horizontalLines_;
};

}

#endif
