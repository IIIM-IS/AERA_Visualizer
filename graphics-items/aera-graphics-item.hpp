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

class AeraGraphicsItem : public QGraphicsPolygonItem
{
public:
  AeraGraphicsItem(
    QMenu* contextMenu, AeraEvent* aeraEvent, ReplicodeObjects& replicodeObjects,
    AeraVisualizerScene* parent);

  void removeArrows();
  void addArrow(Arrow* arrow) { arrows_.append(arrow); }
  AeraEvent* getAeraEvent() { return aeraEvent_; }

  /**
   * Replace all "\n" or "\x01" with "<br>" and " " or "\x02" with "&nbsp;".
   * \param input The input string to htmlify.
   * \return The HTML string.
   */
  static QString htmlify(const QString& input)
  {
    QString result = input;
    result.replace("\n", "<br>");
    result.replace("\x01", "<br>");
    result.replace(" ", "&nbsp;");
    result.replace("\x02", "&nbsp;");
    return result;
  }

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

  static const char* DownArrowHtml;

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  /**
   * Set the textItem_ to the given html and create the border polygon. Connect
   * the textItem_ to textItemLinkHovered and textItemLinkActivated, with default
   * behavior which a derived class can override.
   * \param html The HTML for the textItem_.
   */
  void setTextItemAndPolygon(QString html);

  virtual void textItemLinkHovered(const QString& link);

  virtual void textItemLinkActivated(const QString& link);

  AeraVisualizerScene* parent_;
  ReplicodeObjects& replicodeObjects_;
  QGraphicsTextItem* textItem_;

private:
  void removeArrow(Arrow* arrow);

  QMenu* contextMenu_;
  AeraEvent* aeraEvent_;
  QList<Arrow*> arrows_;
};

}

#endif
