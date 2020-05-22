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
  setFactSuccessHtml();
  setTextItemAndPolygon(factSuccessHtml_, true);
}

void PredictionSuccessFactItem::setFactSuccessHtml()
{
  auto success = newPredictionSuccessEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factSuccessSource = regex_replace(replicodeObjects_.getSourceCode(
    newPredictionSuccessEvent_->object_), confidenceAndSaliencyRegex, ")");
  string successSource = regex_replace(replicodeObjects_.getSourceCode(success), saliencyRegex, ")");

  QString successLabel(replicodeObjects_.getLabel(success).c_str());

  // Temporarily use "!down" which doesn't have spaces.
  factSuccessHtml_ = QString(factSuccessSource.c_str()).replace(successLabel, "!down");
  factSuccessHtml_ += QString("\n      ") + successSource.c_str();

  factSuccessHtml_ = htmlify(factSuccessHtml_);
  factSuccessHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(newPredictionSuccessEvent_->object_->get_reference(0), factSuccessHtml_);
}

void PredictionSuccessFactItem::textItemLinkActivated(const QString& link)
{
#if 0
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
