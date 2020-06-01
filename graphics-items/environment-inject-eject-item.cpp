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

const QString EnvironmentInjectEjectItem::UpWideArrowHtml("<b>&#129093;</b>");
const QString EnvironmentInjectEjectItem::DownWideArrowHtml("<b>&#129095;</b>");

EnvironmentInjectEjectItem::EnvironmentInjectEjectItem(
  AeraEvent* event, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
: AeraGraphicsItem(event, replicodeObjects, parent, "Eject"),
  event_(event)
{
  setLabelHtml();
  setFactValHtml();

  // Set up the textItem_ first to get its size.
  if (textItem_)
    delete textItem_;
  textItem_ = new TextItem(this);
  textItem_->setHtml(labelHtml_);
  // adjustSize() is needed for right-aligned text.
  textItem_->adjustSize();

  // Position the item origin on the arrow.
  qreal left = -4;
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
  labelHtml_ = (event_->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ? DownWideArrowHtml : UpWideArrowHtml) +
    "<a href=\"#this\">" + replicodeObjects_.getLabel(event_->object_).c_str() + "</a>";
}

void EnvironmentInjectEjectItem::setFactValHtml()
{
  auto val = event_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factSource = regex_replace(replicodeObjects_.getSourceCode(
    event_->object_), confidenceAndSaliencyRegex, ")");
  string valSource = regex_replace(replicodeObjects_.getSourceCode(val), saliencyRegex, ")");

  QString valLabel(replicodeObjects_.getLabel(val).c_str());

  factValHtml_ = QString(factSource.c_str()).replace(valLabel, DownArrowHtml);
  factValHtml_ += QString("\n      ") + valSource.c_str();

  factValHtml_ = htmlify(factValHtml_);
  addSourceCodeHtmlLinks(event_->object_, factValHtml_);
}

void EnvironmentInjectEjectItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("What Is This?", [=]() {
      QString explanation;
      if (event_->eventType_ == EnvironmentInjectEvent::EVENT_TYPE)
        explanation = "<b>Q: What is inject <a href=\"#debug_oid-" +
          QString::number(event_->object_->get_debug_oid()) + "\">" +
          replicodeObjects_.getLabel(event_->object_).c_str() + "</a> ?</b><br>" +
          "This fact was injected from the environment:<br>" + factValHtml_ + "<br><br>";
      else {
        explanation = "<b>Q: What is eject <a href=\"#debug_oid-" +
          QString::number(event_->object_->get_debug_oid()) + "\">" +
          replicodeObjects_.getLabel(event_->object_).c_str() + "</a> ?</b><br>" +
          "A command was ejected to the environment:<br>" + factValHtml_;

        // Debug: Properly find the related program reduction.
        Code* mkRdx = NULL;
        if (event_->object_->get_oid() == 44)
          mkRdx = replicodeObjects_.getObject(45);
        if (mkRdx)
          explanation += QString("<br>It was ejected by instantiated program <b>") + 
            replicodeObjects_.getLabel(mkRdx->get_reference(0)).c_str() +
            "</b>, according to program reduction <a href=\"#debug_oid-" + QString::number(mkRdx->get_debug_oid()) + 
            "\">" + replicodeObjects_.getLabel(mkRdx).c_str() + "</a> .";

        explanation += "<br><br>";
      }

      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
}

void EnvironmentInjectEjectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  // TODO: Why can't we just set the brush in the constructor?
  setBrush(parent_->backgroundBrush());
  AeraGraphicsItem::paint(painter, option, widget);
}

}
