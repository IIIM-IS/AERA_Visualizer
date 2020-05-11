#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "program-output-fact-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

ProgramOutputFactItem::ProgramOutputFactItem(
  QMenu* contextMenu, ProgramReductionNewObjectEvent* programReductionNewObjectEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, programReductionNewObjectEvent, replicodeObjects, parent),
  programReductionNewObjectEvent_(programReductionNewObjectEvent)
{
  setFactMkValHtml();
  setTextItemAndPolygon(makeHtml());
}

void ProgramOutputFactItem::setFactMkValHtml()
{
  auto mkVal = programReductionNewObjectEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(
    programReductionNewObjectEvent_->object_), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  // Temporarily use "!down" which doesn't have spaces.
  factMkValHtml_ = QString(factMkValSource.c_str()).replace(mkValLabel, "!down");
  factMkValHtml_ += QString("\n      ") + mkValSource.c_str();

  factMkValHtml_.replace("\n", "<br>");
  factMkValHtml_.replace(" ", "&nbsp;");
  factMkValHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(programReductionNewObjectEvent_->object_, factMkValHtml_);
}

QString ProgramOutputFactItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Program Output</font> <a href=\"#this""\">") +
    replicodeObjects_.getLabel(programReductionNewObjectEvent_->object_).c_str() + "</a></h3>";
  html += factMkValHtml_;
  return html;
}

void ProgramOutputFactItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    string whatMadeExplanation = "<u>Q: What made <a href=\"#debug_oid-" +
      to_string(programReductionNewObjectEvent_->object_->get_debug_oid()) + "\">" + 
      replicodeObjects_.getLabel(programReductionNewObjectEvent_->object_) +
      "</a> ?</u><br>" + "Program reduction <a href=\"#debug_oid-" +
      to_string(programReductionNewObjectEvent_->programReduction_->get_debug_oid()) + "\">" +
      replicodeObjects_.getLabel(programReductionNewObjectEvent_->programReduction_) + "</a><br><br>";

    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?",
      [=]() { parent_->getExplanationLogWindow()->appendHtml(whatMadeExplanation); });
    menu->exec(parent_->getMouseScreenPosition() - QPoint(10, 10));
    delete menu;
  }
  else if (link.startsWith("#debug_oid-")) {
    uint64 debug_oid = link.mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (object) {
      auto item = parent_->getAeraGraphicsItem(object);
      if (item) {
        auto menu = new QMenu();
        menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
          [=]() { parent_->zoomToItem(item); });
        menu->exec(parent_->getMouseScreenPosition() - QPoint(10, 10));
        delete menu;
      }
    }
  }
}

}
