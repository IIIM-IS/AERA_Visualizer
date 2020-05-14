#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "model-item.hpp"
#include "composite-state-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

CompositeStateItem::CompositeStateItem(
  QMenu* contextMenu, NewCompositeStateEvent* newCompositeStateEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newCompositeStateEvent, replicodeObjects, parent),
  newCompositeStateEvent_(newCompositeStateEvent)
{
  // Set up sourceCodeHtml_
  sourceCodeHtml_ = htmlify(simplifyCstSource(
    replicodeObjects_.getSourceCode(newCompositeStateEvent->object_)));
  addSourceCodeHtmlLinks(newCompositeStateEvent_->object_, sourceCodeHtml_);
  ModelItem::highlightVariables(sourceCodeHtml_);

  setTextItemAndPolygon(makeHtml());
}

string CompositeStateItem::simplifyCstSource(const string& cstSource)
{
  string result = cstSource;
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(result.begin(), result.end(), '\n', '\x01');
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  result = regex_replace(result,
    regex("[\\s\\x01]+\\[[\\w\\s]+\\][\\s\\x01]+[\\d\\.]+[\\s\\x01]*\\)$"), ")");
  // TODO: Correctly remove wildcards.
  result = regex_replace(result, regex(" : :\\)"), ")");
  result = regex_replace(result, regex(" :\\)"), ")");
  // Restore \n.
  replace(result.begin(), result.end(), '\x01', '\n');
  return result;
}

QString CompositeStateItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Composite State</font> <a href=\"#this\">") +
    replicodeObjects_.getLabel(newCompositeStateEvent_->object_).c_str() + "</a></h3>";
  html += sourceCodeHtml_;
  return html;
}

}
