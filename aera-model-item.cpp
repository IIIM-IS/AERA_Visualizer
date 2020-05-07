#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "aera-visualizer-scene.hpp"
#include "aera-model-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraModelItem::AeraModelItem(
  QMenu* contextMenu, NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newModelEvent, replicodeObjects, parent),
  newModelEvent_(newModelEvent),
  evidenceCount_(1), successRate_(1),
  evidenceCountColor_("black"), successRateColor_("black"),
  evidenceCountFlashCountdown_(0), successRateFlashCountdown_(0)
{
  // Set up sourceCodeHtml_
  string sourceCode = replicodeObjects_.getSourceCode(newModelEvent_->object_);
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
  addSourceCodeHtmlLinks(newModelEvent_->object_, html);
  sourceCodeHtml_ = html;

  const qreal left = -100;
  const qreal top = -50;

  // Set up the textItem_ first to get its size.
  textItem_ = new QGraphicsTextItem(this);
  textItem_->setPos(left + 5, top + 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated, 
    [this](const QString& link) { textItemLinkActivated(link); });
  updateFromModel();

  qreal right = textItem_->boundingRect().width() - 50;
  qreal bottom = textItem_->boundingRect().height() - 30;
  const qreal diameter = 20;

  QPainterPath path;
  path.moveTo(right, diameter / 2);
  path.arcTo(right - diameter, top, diameter, diameter, 0, 90);
  path.arcTo(left, top, diameter, diameter, 90, 90);
  path.arcTo(left, bottom - diameter, diameter, diameter, 180, 90);
  path.arcTo(right - diameter, bottom - diameter, diameter, diameter, 270, 90);
  setPolygon(path.toFillPolygon());
}

QString AeraModelItem::makeHtml()
{
  auto model = newModelEvent_->object_;

  QString html = QString("<h3><font color=\"darkred\">Model</font> <a href=\"#oid-") + 
    QString::number(newModelEvent_->object_->get_oid()) + "\">" + 
    replicodeObjects_.getLabel(model).c_str() + "</h3>";
  html += sourceCodeHtml_ + "<br><br>";
  html += "<font color=\"" + evidenceCountColor_ + "\">Evidence Count: " +
    QString::number(evidenceCount_) + "</font><br>";
  html += "<font color=\"" + successRateColor_ + "\">&nbsp;&nbsp;&nbsp;&nbsp;Success Rate: " +
    QString::number(successRate_) + "</font><br>";
  return html;
}

void AeraModelItem::updateFromModel()
{
  auto model = newModelEvent_->object_;
  evidenceCountIncreased_ = (model->code(MDL_CNT).asFloat() >= evidenceCount_);
  evidenceCount_ = model->code(MDL_CNT).asFloat();
  successRateIncreased_ = (model->code(MDL_SR).asFloat() >= successRate_);
  successRate_ = model->code(MDL_SR).asFloat();

  textItem_->setHtml(makeHtml());
}

}
