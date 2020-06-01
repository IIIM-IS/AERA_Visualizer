#include <regex>
#include <algorithm>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "program-reduction-item.hpp"
#include "program-output-fact-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

ProgramOutputFactItem::ProgramOutputFactItem(
  ProgramReductionNewObjectEvent* programReductionNewObjectEvent, ReplicodeObjects& replicodeObjects, 
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(programReductionNewObjectEvent, replicodeObjects, parent, "Program Output"),
  programReductionNewObjectEvent_(programReductionNewObjectEvent)
{
  setFactMkValHtml();
  setTextItemAndPolygon(factMkValHtml_, true);
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

  factMkValHtml_ = QString(factMkValSource.c_str()).replace(mkValLabel, DownArrowHtml);
  factMkValHtml_ += QString("\n      ") + mkValSource.c_str();

  factMkValHtml_ = htmlify(factMkValHtml_);
  addSourceCodeHtmlLinks(programReductionNewObjectEvent_->object_, factMkValHtml_);
}

void ProgramOutputFactItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?", [=]() {
      auto mkRdx = programReductionNewObjectEvent_->programReduction_;

      string explanation = "<b>Q: What made program output <a href=\"#debug_oid-" +
        to_string(programReductionNewObjectEvent_->object_->get_debug_oid()) + "\">" +
        replicodeObjects_.getLabel(programReductionNewObjectEvent_->object_) + "</a> ?</b><br>" + 
        "This an output of instantiated program <b>" + replicodeObjects_.getLabel(mkRdx->get_reference(0)) +
        "</b>, according to<br>program reduction <a href=\"#debug_oid-" + 
        to_string(mkRdx->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(mkRdx) + "</a><br><br>";
      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else
    // For #debug_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
