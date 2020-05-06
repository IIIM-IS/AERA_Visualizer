#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "aera-visualizer-scene.hpp"
#include "aera-prediction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraPredictionItem::AeraPredictionItem(
  QMenu* contextMenu, NewMkValPredictionEvent* newPredictionEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newPredictionEvent, replicodeObjects, parent),
  newPredictionEvent_(newPredictionEvent)
{
  const qreal left = -100;
  const qreal top = -50;
  const qreal diameter = 20;

  sourceCodeHtml_ = getPredictionSourceCodeHtml(newPredictionEvent_->object_);

  // Set up the textItem_ first to get its size.
  textItem_ = new QGraphicsTextItem(this);
  textItem_->setPos(left + 5, top + 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated,
    [this](const QString& link) { textItemLinkActivated(link); });
  setTextItemHtml();

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
}

QString AeraPredictionItem::getPredictionSourceCodeHtml(Code* factPred)
{
  Code* pred = factPred->get_reference(0);
  Code* factMkVal = pred->get_reference(0);
  Code* mkVal = factMkVal->get_reference(0);

  QString html = replicodeObjects_.getSourceCode(factPred).c_str();
  html += QString("\n  ") + replicodeObjects_.getSourceCode(pred).c_str();
  html += QString("\n    ") + replicodeObjects_.getSourceCode(factMkVal).c_str();
  html += QString("\n      ") + replicodeObjects_.getSourceCode(mkVal).c_str();

  html.replace("\n", "<br>");
  html.replace(" ", "&nbsp;");
  addSourceCodeHtmlLinks(newPredictionEvent_->object_, html);
  return html;
}

void AeraPredictionItem::setTextItemHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Prediction</font> <a href=\"#oid-") +
    QString::number(newPredictionEvent_->object_->get_oid()) + "\">" +
    replicodeObjects_.getLabel(newPredictionEvent_->object_).c_str() + "</h3>";
  html += sourceCodeHtml_;
  textItem_->setHtml(html);
}

}
