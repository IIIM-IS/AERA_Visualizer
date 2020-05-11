#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
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
  string cstSource = replicodeObjects_.getSourceCode(newCompositeStateEvent->object_);
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(cstSource.begin(), cstSource.end(), '\n', '\x01');
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  cstSource = regex_replace(cstSource,
    regex("[\\s\\x01]+\\[[\\w\\s]+\\][\\s\\x01]+[\\d\\.]+[\\s\\x01]*\\)$"), ")");
  // TODO: Correctly remove wildcards.
  cstSource = regex_replace(cstSource, regex(" : :\\)"), ")");
  cstSource = regex_replace(cstSource, regex(" :\\)"), ")");
  // Restore \n.
  replace(cstSource.begin(), cstSource.end(), '\x01', '\n');
  QString html = cstSource.c_str();
  html.replace("\n", "<br>");
  html.replace(" ", "&nbsp;");
  addSourceCodeHtmlLinks(newCompositeStateEvent_->object_, html);
  sourceCodeHtml_ = html;

  setTextItemAndPolygon(makeHtml());
}

QString CompositeStateItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Composite State</font> <a href=\"#this\">") +
    replicodeObjects_.getLabel(newCompositeStateEvent_->object_).c_str() + "</a></h3>";
  html += sourceCodeHtml_;
  return html;
}

}
