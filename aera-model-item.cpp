#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "arrow.hpp"
#include "aera-model-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraModelItem::AeraModelItem(
  QMenu* contextMenu, NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, QGraphicsItem* parent)
  : QGraphicsPolygonItem(parent),
  newModelEvent_(newModelEvent), replicodeObjects_(replicodeObjects),
  evidenceCount_(1), successRate_(1),
  evidenceCountColor_("black"), successRateColor_("black")
{
  contextMenu_ = contextMenu;

  const qreal left = -100;
  const qreal top = -50;
  const qreal diameter = 20;

  // Set up the textItem_ first to get its size.
  textItem_ = new QGraphicsTextItem(this);
  textItem_->setPos(left + 5, top + 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated, &AeraModelItem::textItemLinkActivated);
  updateFromModel();

  qreal right = textItem_->boundingRect().width() - 50;
  qreal bottom = textItem_->boundingRect().height() - 30;

  QPainterPath path;
  path.moveTo(right, diameter / 2);
  path.arcTo(right - diameter, top, diameter, diameter, 0, 90);
  path.arcTo(left, top, diameter, diameter, 90, 90);
  path.arcTo(left, bottom - diameter, diameter, diameter, 180, 90);
  path.arcTo(right - diameter, bottom - diameter, diameter, diameter, 270, 90);
  polygon_ = path.toFillPolygon();

  setPolygon(polygon_);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

  borderFlashCountdown_ = 6;
  evidenceCountFlashCountdown_ = 0;
  successRateFlashCountdown_ = 0;
}

void AeraModelItem::removeArrows()
{
  foreach(Arrow* arrow, arrows_) {
    qgraphicsitem_cast<AeraModelItem*>(arrow->startItem())->removeArrow(arrow);
    qgraphicsitem_cast<AeraModelItem*>(arrow->endItem())->removeArrow(arrow);
    scene()->removeItem(arrow);
    delete arrow;
  }
}

void AeraModelItem::removeArrow(Arrow* arrow)
{
  int index = arrows_.indexOf(arrow);
  if (index != -1)
    arrows_.removeAt(index);
}

void AeraModelItem::setTextItemHtml()
{
  auto model = newModelEvent_->model_;

  // TODO: Cache some of this.
  string label = replicodeObjects_.getLabel(model);
  QString labelHtml = "";
  if (label != "")
    labelHtml = QString("<font color=\"blue\"><b>") + label.c_str() + "</b><font color = \"black\">:";

  string sourceCode = replicodeObjects_.getSourceCode(model);
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(sourceCode.begin(), sourceCode.end(), '\n', '\x01');
  // Strip the parameters. We will add them below. 
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  regex modelRegex("^(.+)([\\s\\x01]+[\\d\\.]+){5}[\\s\\x01]*\\)$");
  smatch matches;
  if (regex_search(sourceCode, matches, modelRegex))
    sourceCode = matches[1].str();
  // Restore \n.
  replace(sourceCode.begin(), sourceCode.end(), '\x01', '\n');

  QString sourceCodeHtml = sourceCode.c_str();
  sourceCodeHtml.replace("\n", "<br>");
  sourceCodeHtml.replace(" ", "&nbsp;");

  sourceCodeHtml += "<br>" + QString::number(model->code(MDL_STRENGTH).asFloat()) + " ; Strength";
  sourceCodeHtml += "<br><font color=\"" + evidenceCountColor_ +"\">" +
    QString::number(evidenceCount_) + " ; Evidence Count</font>";
  sourceCodeHtml += "<br><font color=\"" + successRateColor_ + "\">" + 
    QString::number(successRate_) + " ; Success Rate</font>";
  sourceCodeHtml += "<br>" + QString::number(model->code(MDL_DSR).asFloat()) + " ; Derivative of Success Rate";
  sourceCodeHtml += "<br>1)";
  textItem_->setHtml(labelHtml + sourceCodeHtml);
}

void AeraModelItem::addArrow(Arrow* arrow)
{
  arrows_.append(arrow);
}

void AeraModelItem::updateFromModel()
{
  auto model = newModelEvent_->model_;
  evidenceCountIncreased_ = (model->code(MDL_CNT).asFloat() >= evidenceCount_);
  evidenceCount_ = model->code(MDL_CNT).asFloat();
  successRateIncreased_ = (model->code(MDL_SR).asFloat() >= successRate_);
  successRate_ = model->code(MDL_SR).asFloat();

  setTextItemHtml();
}

void AeraModelItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  scene()->clearSelection();
  setSelected(true);
  contextMenu_->exec(event->screenPos());
}

QVariant AeraModelItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange) {
    newModelEvent_->itemPosition_ = value.toPointF();

    foreach(Arrow* arrow, arrows_)
      arrow->updatePosition();
  }

  return value;
}

void AeraModelItem::textItemLinkActivated(const QString& link)
{
}

}
