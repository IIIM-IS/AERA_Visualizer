#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "aera-visualizer-scene.hpp"
#include "aera-composite-state-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraCompositeStateItem::AeraCompositeStateItem(
  QMenu* contextMenu, NewCompositeStateEvent* newCompositeStateEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newCompositeStateEvent, replicodeObjects, parent),
  newCompositeStateEvent_(newCompositeStateEvent)
{
  // Set up sourceCodeHtml_
  string sourceCode = replicodeObjects_.getSourceCode(newCompositeStateEvent->object_);
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(sourceCode.begin(), sourceCode.end(), '\n', '\x01');
  // Strip the set of output groups and parameters.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  regex modelRegex("^(.+)[\\s\\x01]+\\[[\\w\\s]+\\][\\s\\x01]+[\\d\\.]+[\\s\\x01]*\\)$");
  smatch matches;
  if (regex_search(sourceCode, matches, modelRegex))
    sourceCode = matches[1].str();
  sourceCode += ")";
  // Restore \n.
  replace(sourceCode.begin(), sourceCode.end(), '\x01', '\n');
  QString html = sourceCode.c_str();
  html.replace("\n", "<br>");
  html.replace(" ", "&nbsp;");
  addSourceCodeHtmlLinks(newCompositeStateEvent_->object_, html);
  sourceCodeHtml_ = html;

  setTextItemAndPolygon(makeHtml());
}

QString AeraCompositeStateItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Composite State</font> <a href=\"#oid-") + 
    QString::number(newCompositeStateEvent_->object_->get_oid()) + "\">" +
    replicodeObjects_.getLabel(newCompositeStateEvent_->object_).c_str() + "</h3>";
  html += sourceCodeHtml_;
  return html;
}

}
