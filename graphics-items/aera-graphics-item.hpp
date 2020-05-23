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
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
class QGraphicsSimpleTextItem;

namespace aera_visualizer {

class Arrow;
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
   * \param contextMenu The right-click context menu from the parent.
   * \param aeraEvent The AeraEvent which is used to get the time_ for the header and the itemTopLeftPosition_.
   * \param replicodeObjects The ReplicodeObjects used to get the debug OID and label.
   * \param parent The parent AeraVisualizerScene.
   * \param headerPrefix The prefix for headerHtml_ as described above.
   */
  AeraGraphicsItem(
    QMenu* contextMenu, AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent, const QString& headerPrefix = "");

  void removeArrows();
  void addArrow(Arrow* arrow) { arrows_.append(arrow); }
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
   * Replace all "\n" or "\x01" with "<br>" and extra " " with "&nbsp;".
   * \param input The input string to htmlify.
   * \return The HTML string.
   */
  static QString htmlify(const QString& input);

  static QString htmlify(const std::string& input) { return htmlify(QString(input.c_str())); }

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

  int borderFlashCountdown_;

  static const QString DownArrowHtml;
  static const QString SelectedRadioButtonHtml;
  static const QString UnselectedRadioButtonHtml;

protected:
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

  /**
   * Set the textItem_ to the given html and create the border polygon. Connect
   * the textItem_ to textItemLinkActivated, with default behavior which a derived class can override.
   * \param html The HTML for the textItem_.
   * \param prependHeaderHtml If false, use html as-is. If true, first set the text 
   * to html and adjust the size, then set the text to headerHtml_+html. We do this because
   * the header has a right-aligned table cell which needs to "know" the width of the text.
   */
  void setTextItemAndPolygon(QString html, bool prependHeaderHtml);

  virtual void textItemLinkActivated(const QString& link);

  AeraVisualizerScene* parent_;
  ReplicodeObjects& replicodeObjects_;
  QString headerHtml_;
  TextItem* textItem_;

private:
  void removeArrow(Arrow* arrow);

  QMenu* contextMenu_;
  AeraEvent* aeraEvent_;
  QList<Arrow*> arrows_;
};

}

#endif
