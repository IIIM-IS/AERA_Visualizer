#include <regex>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "prediction-success-fact-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

PredictionSuccessFactItem::PredictionSuccessFactItem(
  QMenu* contextMenu, NewPredictionSuccessEvent* newPredictionSuccessEvent,
  ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newPredictionSuccessEvent, replicodeObjects, parent, "Prediction Success"),
  newPredictionSuccessEvent_(newPredictionSuccessEvent)
{
  setFactMkValHtml();
  setTextItemAndPolygon(makeHtml());
}

void PredictionSuccessFactItem::setFactMkValHtml()
{
  auto mkVal = newPredictionSuccessEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(
    newPredictionSuccessEvent_->object_), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  // Temporarily use "!down" which doesn't have spaces.
  factMkValHtml_ = QString(factMkValSource.c_str()).replace(mkValLabel, "!down");
  factMkValHtml_ += QString("\n      ") + mkValSource.c_str();

  // TODO: Show autoFocusNewObjectEvent_->syncMode_?

  factMkValHtml_ = htmlify(factMkValHtml_);
  factMkValHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(newPredictionSuccessEvent_->object_, factMkValHtml_);
}

QString PredictionSuccessFactItem::makeHtml()
{
  return headerHtml_ + factMkValHtml_;
}

void PredictionSuccessFactItem::textItemLinkActivated(const QString& link)
{
#if 0
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?", [=]() {
      auto mkRdx = programReductionNewObjectEvent_->programReduction_;

      string explanation = "<u>Q: What made program output <a href=\"#debug_oid-" +
        to_string(programReductionNewObjectEvent_->object_->get_debug_oid()) + "\">" +
        replicodeObjects_.getLabel(programReductionNewObjectEvent_->object_) + "</a> ?</u><br>" +
        "This an output of instantiated program <b>" + replicodeObjects_.getLabel(mkRdx->get_reference(0)) +
        "</b>, according to<br>program reduction <a href=\"#debug_oid-" +
        to_string(mkRdx->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(mkRdx) + "</a><br><br>";
      parent_->getExplanationLogWindow()->appendHtml(explanation);
    });
    menu->exec(parent_->getMouseScreenPosition() - QPoint(10, 10));
    delete menu;
  }
  else
#endif
    // For #debug_oid- and others, defer to the base class.
    AeraGraphicsItem::textItemLinkActivated(link);
}

}
