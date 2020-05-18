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
  : AeraGraphicsItem(contextMenu, programReductionEvent, replicodeObjects, parent, "Program Reduction"),
  programReductionEvent_(programReductionEvent)
{
  // Set up sourceCodeHtml_
  sourceCodeHtml_ = htmlify(simplifyMkRdxSource(
    replicodeObjects_.getSourceCode(programReductionEvent->object_)));
  addSourceCodeHtmlLinks(programReductionEvent_->object_, sourceCodeHtml_);

  setTextItemAndPolygon(makeHtml());
}

string ProgramReductionItem::simplifyMkRdxSource(const string& mkRdxSource)
{
  string result = mkRdxSource;
  // Temporarily replace \n with \x01 so that we match the entire string, not by line.
  replace(result.begin(), result.end(), '\n', '\x01');
  // Strip the propagation of saliency threshold.
  // "[\\s\\x01]+" is whitespace "[\\d\\.]+" is a float value.
  // TODO: The original source may have comments, so need to strip these.
  result = regex_replace(result, regex("[\\s\\x01]+[\\d\\.]+[\\s\\x01]*\\)$"), ")");
  // Restore \n.
  replace(result.begin(), result.end(), '\x01', '\n');
  return result;
}

QString ProgramReductionItem::makeHtml()
{
  return headerHtml_ + sourceCodeHtml_;
}

}
