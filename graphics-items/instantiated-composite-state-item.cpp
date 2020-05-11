#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "instantiated-composite-state-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

InstantiatedCompositeStateItem::InstantiatedCompositeStateItem(
  QMenu* contextMenu, NewInstantiatedCompositeStateEvent* newInstantiatedCompositeStateEvent,
  ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newInstantiatedCompositeStateEvent, replicodeObjects, parent),
  newInstantiatedCompositeStateEvent_(newInstantiatedCompositeStateEvent)
{
  setFactIcstHtml();
  setTextItemAndPolygon(makeHtml());
}

void InstantiatedCompositeStateItem::setFactIcstHtml()
{
  auto icst = newInstantiatedCompositeStateEvent_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factIcstSource = regex_replace(
    replicodeObjects_.getSourceCode(newInstantiatedCompositeStateEvent_->object_), confidenceAndSaliencyRegex, ")");
  string icstSource = regex_replace(replicodeObjects_.getSourceCode(icst), saliencyRegex, ")");

  QString icstLabel(replicodeObjects_.getLabel(icst).c_str());

  // Temporarily use "!down" which doesn't have spaces.
  factIcstHtml_ = QString(factIcstSource.c_str()).replace(icstLabel, "!down");
  factIcstHtml_ += QString("\n      ") + icstSource.c_str();

  factIcstHtml_.replace("\n", "<br>");
  factIcstHtml_.replace(" ", "&nbsp;");
  factIcstHtml_ = factIcstHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(newInstantiatedCompositeStateEvent_->object_->get_reference(0), factIcstHtml_);
}

QString InstantiatedCompositeStateItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Instantiated Composite State</font> <a href=\"#this""\">") +
    replicodeObjects_.getLabel(newInstantiatedCompositeStateEvent_->object_).c_str() + "</a></h3>";
  html += factIcstHtml_;
  return html;
}

}
