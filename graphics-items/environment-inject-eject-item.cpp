#include <regex>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "../aera-visualizer-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "environment-inject-eject-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

const QString EnvironmentInjectEjectItem::UpArrowHtml("&#129145;");
const QString EnvironmentInjectEjectItem::DownArrowHtml("&#129147;");

EnvironmentInjectEjectItem::EnvironmentInjectEjectItem(
  AeraEvent* event, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
: AeraGraphicsItem(event, replicodeObjects, parent, "Eject"),
  event_(event)
{
  setLabelHtml();

  // Set up the textItem_ first to get its size.
  if (textItem_)
    delete textItem_;
  textItem_ = new TextItem(this);
  textItem_->setHtml(labelHtml_);
  // adjustSize() is needed for right-aligned text.
  textItem_->adjustSize();

  // Position the item origin on the arrow.
  qreal left = -5;
  qreal top = -textItem_->boundingRect().height() / 2 + 5;
  textItem_->setPos(left -5, top - 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated,
    [this](const QString& link) { textItemLinkActivated(link); });

  qreal right = left + textItem_->boundingRect().width() - 5;
  qreal bottom = textItem_->boundingRect().height() / 2 - 2;

  // Make a simple rectangle.
  QPainterPath path;
  path.moveTo(right, top);
  path.lineTo(left, top);
  path.lineTo(left, bottom);
  path.lineTo(right, bottom);
  auto polygon = path.toFillPolygon();
  setPolygon(polygon);

  // Blend with the background. Below, we override paint() to use the parent's background color.
  borderNoHighlightPen_ = QPen(parent_->backgroundBrush().color(), 1);
}

void EnvironmentInjectEjectItem::setLabelHtml()
{
  bool debug1 = (event_->eventType_ == EnvironmentInjectEvent::EVENT_TYPE);
  labelHtml_ = (event_->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ? DownArrowHtml : UpArrowHtml) +
    replicodeObjects_.getLabel(event_->object_).c_str();
}

void EnvironmentInjectEjectItem::textItemLinkActivated(const QString& link)
{
#if 0 // debug
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?", [=]() {
      auto fromObject = autoFocusNewObjectEvent_->fromObject_;

      string explanation = "<b>Q: What made auto focus <a href=\"#debug_oid-" +
        to_string(autoFocusNewObjectEvent_->object_->get_debug_oid()) + "\">" +
        replicodeObjects_.getLabel(autoFocusNewObjectEvent_->object_) + "</a> ?</b><br>" +
        "The auto focus controller made this from <a href=\"#debug_oid-" +
        to_string(fromObject->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(fromObject) + "</a><br><br>";
      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else
    // For #debug_oid- and others, defer to the base class.
#endif
    AeraGraphicsItem::textItemLinkActivated(link);
}

void EnvironmentInjectEjectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  // TODO: Why can't we just set the brush in the constructor?
  setBrush(parent_->backgroundBrush());
  AeraGraphicsItem::paint(painter, option, widget);
}

}
