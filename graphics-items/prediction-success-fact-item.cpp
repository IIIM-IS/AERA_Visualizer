#include <regex>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "prediction-item.hpp"
#include "prediction-success-fact-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

PredictionSuccessFactItem::PredictionSuccessFactItem(
  NewPredictionSuccessEvent* newPredictionSuccessEvent, ReplicodeObjects& replicodeObjects, 
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(newPredictionSuccessEvent, replicodeObjects, parent, "Prediction Success"),
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
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?", [=]() {
      Code* factPrediction = newPredictionSuccessEvent_->object_->get_reference(0)->get_reference(0);
      Code* input = newPredictionSuccessEvent_->object_->get_reference(0)->get_reference(1);

      string explanation = "<b>Q: What made prediction success <a href=\"#debug_oid-" +
        to_string(newPredictionSuccessEvent_->object_->get_debug_oid()) + "\">" +
        replicodeObjects_.getLabel(newPredictionSuccessEvent_->object_) + "</a> ?</b><br>" +
        "Input <a href=\"#debug_oid-" + to_string(input->get_debug_oid()) + "\">" + 
        replicodeObjects_.getLabel(input) + "</a> was matched against prediction <a href=\"#debug_oid-" + 
        to_string(factPrediction->get_debug_oid()) + "\">" +
        replicodeObjects_.getLabel(factPrediction) + "</a> with success.<br><br>";
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
