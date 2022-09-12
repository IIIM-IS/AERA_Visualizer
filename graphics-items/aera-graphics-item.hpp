//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
//_/_/ Copyright (c) 2021 Leonard Eberding
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
   * \param replicodeObjects The ReplicodeObjects used to get the detail OID and label.
   * \param parent The parent AeraVisualizerScene.
   * \param headerPrefix The prefix for headerHtml_ as described above.
   * \param textItemTextColor (optional) The text color when we recreate the textItem_ . If ommitted, use black.
   */
  AeraGraphicsItem(
    AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent, const QString& headerPrefix, QColor textItemTextColor = Qt::black);

  /**
   * Add the Arrow to the list of arrows. This does not take ownership
   * or add to the parent scene.
   * \param arrow The arrow to add.
   */
  void addArrow(Arrow* arrow) { arrows_.append(arrow); }

  /**
   * Remove the Arrow from the list of arrows, delete it from the scene and delete the arrow.
   * \param arrow The Arrow to remove from the list of arrows. If it is not in the
   * list, then do nothing (do not delete it or remove from the parent scene).
   */
  void removeAndDeleteArrow(Arrow* arrow);

  /**
   * Search the list of arrows for an arrow to an item whose AeraEvent object is the
   * given object and call removeAndDeleteArrow to remove and delete the Arrow.
   * \param object The object in the Aera event of the Item at the end of the Arrow.
   */
  void removeAndDeleteArrowToObject(r_code::Code* object);

  /**
   * Remove all arrows and horizontal lines and remove them from the parent scene.
   */
  void removeArrowsAndHorizontalLines();
  void addHorizontalLine(AnchoredHorizontalLine* line) { horizontalLines_.append(line); }
  void updateArrowsAndLines();
  AeraEvent* getAeraEvent() { return aeraEvent_; }
  QString getHtml() { return textItem_->toHtml(); }

  /**
   * Change the Z value of this item to be slightly greater than other colliding AeraGraphicsItems.
   */
  void bringToFront();

  /**
   * Change the Z value of this item to be slightly less than other colliding AeraGraphicsItems.
   */
  void sendToBack();

  void focus();

  void centerOn();

  /**
   * Reset the position to aeraEvent_->itemInitialTopLeftPosition_.
   */
  void resetPosition();

  void ensureVisible();

  /**
   * Replace all "\n" or "\x01" with "<br>" and extra " " with "&nbsp;".
   * \param input The input string to htmlify.
   * \param useNowrap (optional) If true, put the HTMl inside <div style="white-space: nowrap;">.
   * If omitted, use false.
   * \return The HTML string.
   */
  static QString htmlify(const QString& input, bool useNowrap = false);

  static QString htmlify(const std::string& input, bool useNowrap = false) { return htmlify(QString(input.c_str()), useNowrap); }

  /**
   * Make an HTML string with <a href="#detail_oid-XXX"></a> around the label the object. 
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
   * Go through object's references and modify html with <a href="#detail_oid-XXX"></a>
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

  /**
   * Adjust the position of the item.
   It moves the item below the lowest item under which it fits without colliding with other items from this timeframe.
   */
  void adjustItemYPosition();


  const QPen& getBorderNoHighlightPen() { return borderNoHighlightPen_;  }

  /**
   * Check if getAeraEvent()->object_ is a simulated goal or prediction. This is meant
   * to imitate the Replicode is_sim operator.
   * \return True if the object is simulated.
   */
  bool is_sim();

  static const QString DownArrowHtml;
  static const QString RightArrowHtml;
  static const QString RightDoubleArrowHtml;
  static const QString SelectedRadioButtonHtml;
  static const QString UnselectedRadioButtonHtml;
  static const QString RightPointingTriangleHtml;
  static const QString DownPointingTriangleHtml;
  static const QString HourglassHtml;
  static const QString StopSignHtml;
  static const QString CheckMarkHtml;
  static const QString RedXHtml;

  static const QColor Color_proponent_justifications;
  static const QColor Color_proponent_asm_toBeProved;
  static const QColor Color_proponent_asm;
  static const QColor Color_proponent_nonAsm_toBeProved;
  static const QColor Color_proponent_nonAsm;
  static const QColor Color_opponent_finished_justification;
  static const QColor Color_opponent_unfinished_justification;
  static const QColor Color_opponent_ms_border;
  static const QColor Color_opponent_ms_asm_culprit;
  static const QColor Color_opponent_ms_asm_culprit_text;
  static const QColor Color_opponent_ms_asm_defence;
  static const QColor Color_opponent_ms_asm_defence_text;
  static const QColor Color_opponent_ms_asm;
  static const QColor Color_opponent_ms_asm_text;
  static const QColor Color_opponent_ms_nonAsm;
  static const QColor Color_opponent_ms_nonAsm_text;
  static const QColor Color_opponent_ums_asm_defence;
  static const QColor Color_opponent_ums_asm_defence_border;
  static const QColor Color_opponent_ums_asm_defence_text;
  static const QColor Color_opponent_ums_asm_culprit;
  static const QColor Color_opponent_ums_asm_culprit_border;
  static const QColor Color_opponent_ums_asm_culprit_text;
  static const QColor Color_opponent_ums_asm;
  static const QColor Color_opponent_ums_asm_border;
  static const QColor Color_opponent_ums_asm_text;
  static const QColor Color_opponent_ums_nonAsm;
  static const QColor Color_opponent_ums_nonAsm_border;
  static const QColor Color_opponent_ums_nonAsm_text;
  static const QColor Color_attack_edge;

  int borderFlashCountdown_;

protected:
  typedef enum { SHAPE_RECTANGLE, SHAPE_GOAL, SHAPE_PRED, SHAPE_STOP } Shape;

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
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  };
  friend TextItem;

  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

  /**
   * Set the textItem_ to the given html and create the border polygon. Use the textItemTextColor given to the constructor.
   * Connect the textItem_ to textItemLinkActivated, with default behavior which a derived class can override.
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
  AeraEvent* aeraEvent_;
  QList<Arrow*> arrows_;
  QList<AnchoredHorizontalLine*> horizontalLines_;
  QColor textItemTextColor_;
};

}

#endif
