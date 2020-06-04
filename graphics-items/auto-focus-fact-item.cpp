#include <regex>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "../aera-visualizer-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "auto-focus-fact-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AutoFocusFactItem::AutoFocusFactItem(
  AutoFocusNewObjectEvent* autoFocusNewObjectEvent, ReplicodeObjects& replicodeObjects, 
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(autoFocusNewObjectEvent, replicodeObjects, parent, "Auto Focus"),
  autoFocusNewObjectEvent_(autoFocusNewObjectEvent)
{
  setFactMkValHtml();
  setTextItemAndPolygon(factMkValHtml_, true);
}

void AutoFocusFactItem::setFactMkValHtml()
{
  auto mkVal = autoFocusNewObjectEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factMkValSource = regex_replace(replicodeObjects_.getSourceCode(
    autoFocusNewObjectEvent_->object_), confidenceAndSaliencyRegex, ")");
  string mkValSource = regex_replace(replicodeObjects_.getSourceCode(mkVal), saliencyRegex, ")");

  QString mkValLabel(replicodeObjects_.getLabel(mkVal).c_str());

  factMkValHtml_ = QString(factMkValSource.c_str()).replace(mkValLabel, DownArrowHtml);
  factMkValHtml_ += QString("\n      ") + mkValSource.c_str();

  // TODO: Show autoFocusNewObjectEvent_->syncMode_?

  factMkValHtml_ = htmlify(factMkValHtml_);
  addSourceCodeHtmlLinks(autoFocusNewObjectEvent_->object_, factMkValHtml_);
}

void AutoFocusFactItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("Zoom to This", [=]() { parent_->zoomToItem(this); });
    menu->addAction("What Made This?", [=]() {
      auto fromObject = autoFocusNewObjectEvent_->fromObject_;

      QString explanation = "<b>Q: What made auto focus " + makeHtmlLink(autoFocusNewObjectEvent_->object_) +
        " ?</b><br>The auto focus controller made this from " + makeHtmlLink(fromObject) + "<br><br>";
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
