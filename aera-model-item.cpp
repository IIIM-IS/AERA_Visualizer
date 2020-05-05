#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "submodules/replicode/r_exec/opcodes.h"
#include "arrow.hpp"
#include "aera-visualizer-scene.hpp"
#include "aera-model-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

AeraModelItem::AeraModelItem(
  QMenu* contextMenu, NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : parent_(parent),
  newModelEvent_(newModelEvent), replicodeObjects_(replicodeObjects),
  evidenceCount_(1), successRate_(1),
  evidenceCountColor_("black"), successRateColor_("black"),
  borderFlashCountdown_(6), evidenceCountFlashCountdown_(0), successRateFlashCountdown_(0)
{
  contextMenu_ = contextMenu;

  const qreal left = -100;
  const qreal top = -50;
  const qreal diameter = 20;

  // Set up sourceCodeHtml_
  string sourceCode = replicodeObjects_.getSourceCode(newModelEvent_->model_);
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(sourceCode.begin(), sourceCode.end(), '\n', '\x01');
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  regex modelRegex("^(.+)[\\s\\x01]+\\[[\\w\\s]+\\]([\\s\\x01]+[\\d\\.]+){5}[\\s\\x01]*\\)$");
  smatch matches;
  if (regex_search(sourceCode, matches, modelRegex))
    sourceCode = matches[1].str();
  sourceCode += ")";
  // Restore \n.
  replace(sourceCode.begin(), sourceCode.end(), '\x01', '\n');
  QString html = sourceCode.c_str();
  html.replace("\n", "<br>");
  html.replace(" ", "&nbsp;");
  addSourceCodeHtmlLinks(html);
  sourceCodeHtml_ = html;

  // Set up the textItem_ first to get its size.
  textItem_ = new QGraphicsTextItem(this);
  textItem_->setPos(left + 5, top + 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated, 
    [this](const QString& link) { textItemLinkActivated(link); });
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

void AeraModelItem::addSourceCodeHtmlLinks(QString& html)
{
  Code* object = newModelEvent_->model_;
  for (int i = 0; i < object->references_size(); ++i) {
    auto referencedObject = object->get_reference(i);
    if (!(referencedObject->code(0).asOpcode() == Opcodes::Mdl ||
          referencedObject->code(0).asOpcode() == Opcodes::Cst))
      continue;

    auto referencedLabel = replicodeObjects_.getLabel(referencedObject);
    if (referencedLabel == "")
      continue;

    // Spaces are alreay replaced with &nbsp; .
    // TODO: Handle case when the label is not surrounded by spaces.
    html.replace(
      QString("&nbsp;") + referencedLabel.c_str() + "&nbsp;", 
      QString("&nbsp;<a href=\"#oid-") + QString::number(referencedObject->get_oid()) + "\">" +
              referencedLabel.c_str() + "</a>&nbsp;");
  }
}

void AeraModelItem::setTextItemHtml()
{
  auto model = newModelEvent_->model_;

  QString html = QString("<h3><font color=\"darkred\"><b>") + 
    replicodeObjects_.getLabel(model).c_str() + "</b><font color = \"black\"></h3>";
  html += sourceCodeHtml_ + "<br><br>";
  html += "<font color=\"" + evidenceCountColor_ + "\">Evidence Count: " +
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
  if (link.startsWith("#oid-")) {
    int oid = link.mid(5).toInt();
    auto object = replicodeObjects_.getObject(oid);
    if (object) {
      // TODO: Make this work for other than models.
      auto item = parent_->getAeraModelItem(object);
      if (item)
        parent_->zoomToItem(item);
    }
  }
}

}
