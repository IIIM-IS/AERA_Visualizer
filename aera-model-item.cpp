#include "aera-model-item.hpp"
#include "arrow.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>

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

  qreal right = textItem_->boundingRect().width();
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
  QString html = "OID: " + QString::number(newModelEvent_->model_->get_oid()) + "<br>";
  html += "<font color=\"" + evidenceCountColor_ +"\">Evidence Count: " + 
    QString::number(evidenceCount_) + "</font><br>";
  html += "<font color=\"" + successRateColor_ + "\">&nbsp;&nbsp;&nbsp;&nbsp;Success Rate: " +
    QString::number(successRate_) + "</font><br>";
  textItem_->setHtml(html);
}

void AeraModelItem::addArrow(Arrow* arrow)
{
  arrows_.append(arrow);
}

void AeraModelItem::updateFromModel()
{
  evidenceCountIncreased_ = (newModelEvent_->model_->code(MDL_CNT).asFloat() >= evidenceCount_);
  evidenceCount_ = newModelEvent_->model_->code(MDL_CNT).asFloat();
  successRateIncreased_ = (newModelEvent_->model_->code(MDL_SR).asFloat() >= successRate_);
  successRate_ = newModelEvent_->model_->code(MDL_SR).asFloat();

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
