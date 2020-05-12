#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "program-reduction-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

ProgramReductionItem::ProgramReductionItem(
  QMenu* contextMenu, ProgramReductionEvent* programReductionEvent, ReplicodeObjects& replicodeObjects,
  AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, programReductionEvent, replicodeObjects, parent),
  programReductionEvent_(programReductionEvent)
{
  // Set up sourceCodeHtml_
  string sourceCode = replicodeObjects_.getSourceCode(programReductionEvent->object_);
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(sourceCode.begin(), sourceCode.end(), '\n', '\x01');
  // Strip the propagation of saliency threshold.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  sourceCode = regex_replace(sourceCode, regex("[\\s\\x01]+[\\d\\.]+[\\s\\x01]*\\)$"), ")");
  // Restore \n.
  replace(sourceCode.begin(), sourceCode.end(), '\x01', '\n');
  QString html = htmlify(sourceCode);
  addSourceCodeHtmlLinks(programReductionEvent_->object_, html);
  sourceCodeHtml_ = html;

  setTextItemAndPolygon(makeHtml());
}

QString ProgramReductionItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Program Reduction</font> <a href=\"#this\">") +
    replicodeObjects_.getLabel(programReductionEvent_->object_).c_str() + "</a></h3>";
  html += sourceCodeHtml_;
  return html;
}

}
