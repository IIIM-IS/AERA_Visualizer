#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "aera-program-output-fact-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraProgramOutputFactItem::AeraProgramOutputFactItem(
  QMenu* contextMenu, ProgramReductionNewObjectEvent* programReductionNewObjectEvent,
    ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, programReductionNewObjectEvent, replicodeObjects, parent),
  programReductionNewObjectEvent_(programReductionNewObjectEvent)
{
  sourceCodeHtml_ = getSourceCodeHtml(programReductionNewObjectEvent_->object_);
  setTextItemAndPolygon(makeHtml());
}

QString AeraProgramOutputFactItem::getSourceCodeHtml(Code* factMkVal)
{
  auto mkVal = factMkVal->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(factMkVal), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  QString factMkValHtml = QString(factMkValSource.c_str()).replace(mkValLabel, "!down");
  QString mkValHtml(mkValSource.c_str());

  // Temporarily use "!down" which doesn't have spaces.
  QString html = factMkValHtml;
  html += QString("\n      ") + mkValHtml;

  html.replace("\n", "<br>");
  html.replace(" ", "&nbsp;");
  html = html.replace("!down", "<sub><font size=\"+2\"><b>&#129047;</b></font></sub>");
  addSourceCodeHtmlLinks(programReductionNewObjectEvent_->object_, html);
  return html;
}

QString AeraProgramOutputFactItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Program Output</font> <a href=\"#debug_oid-") +
    QString::number(programReductionNewObjectEvent_->object_->get_debug_oid()) + "\">" +
    replicodeObjects_.getLabel(programReductionNewObjectEvent_->object_).c_str() + "</h3>";
  html += sourceCodeHtml_;
  return html;
}


void AeraProgramOutputFactItem::textItemLinkActivated(const QString& link)
{
  if (link.startsWith("#debug_oid-")) {
    uint64 debug_oid = link.mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (object) {
      auto item = parent_->getAeraGraphicsItem(object);
      if (item) {
        string whatMadeExplanation;
        Code* programReduction = 0;
        if (object->get_oid() == 49)
          programReduction = replicodeObjects_.getObject(47);
        else if (object->get_oid() == 68)
          programReduction = replicodeObjects_.getObject(58);
        else if (object->get_debug_oid() == 2061)
          programReduction = replicodeObjects_.getObject(77);
        if (programReduction)
          whatMadeExplanation = "Q: What made " + replicodeObjects_.getLabel(object) + " ?<br>" +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Program reduction <a href=\"#debug_oid-" +
            to_string(programReduction->get_debug_oid()) + "\">" +
            replicodeObjects_.getLabel(programReduction) + "</a><br>";

        auto menu = new QMenu();
        menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
          [=]() { parent_->zoomToItem(item); });
        if (whatMadeExplanation != "")
        menu->addAction(QString("What made ") + replicodeObjects_.getLabel(object).c_str() + "?",
          [=]() { parent_->getExplanationLogWindow()->appendHtml(whatMadeExplanation); });

        menu->exec(parent_->getMouseScreenPosition() - QPoint(10, 10));
        delete menu;
      }
    }
  }
}

}
