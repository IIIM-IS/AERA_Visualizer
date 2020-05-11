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
using namespace r_exec;

namespace aera_visualizer {

InstantiatedCompositeStateItem::InstantiatedCompositeStateItem(
  QMenu* contextMenu, NewInstantiatedCompositeStateEvent* newInstantiatedCompositeStateEvent,
  ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
  : AeraGraphicsItem(contextMenu, newInstantiatedCompositeStateEvent, replicodeObjects, parent),
  newInstantiatedCompositeStateEvent_(newInstantiatedCompositeStateEvent)
{
  setFactIcstHtml();
  setBoundCstHtml();
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

void InstantiatedCompositeStateItem::setBoundCstHtml()
{
  auto factIcst = (_Fact*)newInstantiatedCompositeStateEvent_->object_;
  auto icst = factIcst->get_reference(0);
  auto cst = icst->get_reference(0);

  std::vector<string> templateValues;
  std::vector<string> outputValues;
  // Debug: Generalize from cst_52.
  string icstSource = replicodeObjects_.getSourceCode(icst);
  smatch matches;
  if (regex_search(icstSource, matches, regex("^\\(icst cst_52 \\|\\[\\] \\[(\\w+) (\\w+)\\] \\w+ \\w+\\)$"))) {
    outputValues.push_back(matches[1].str());
    outputValues.push_back(matches[2].str());
  }

  string cstSource = replicodeObjects_.getSourceCode(cst);
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

  // Substitute variables.
  int iVariable = -1;
  size_t iTemplateValues = 0;
  size_t iOutputValues = 0;
  while (iTemplateValues < templateValues.size() || iOutputValues < outputValues.size()) {
    ++iVariable;
    // v0, v1, v2, etc. are split between templateValues and outputValues.
    string boundValue = (iTemplateValues < templateValues.size() ? 
      templateValues[iTemplateValues] : outputValues[iOutputValues]);

    string variable = "v" + to_string(iVariable) + ":";
    cstSource = regex_replace(cstSource, regex(variable), boundValue);

    if (iTemplateValues < templateValues.size())
      // Still looking at templateValues.
      ++iTemplateValues;
    else
      ++iOutputValues;
  }

  // Debug: How to correctly get the timestamp variables.
  auto afterVariable = "v" + to_string(++iVariable) + ":";
  auto beforeVariable = "v" + to_string(++iVariable) + ":";
  cstSource = regex_replace(cstSource, regex(afterVariable), replicodeObjects_.relativeTime(factIcst->get_after()));
  cstSource = regex_replace(cstSource, regex(beforeVariable), replicodeObjects_.relativeTime(factIcst->get_before()));

  // Restore \n.
  replace(cstSource.begin(), cstSource.end(), '\x01', '\n');

  boundCstHtml_ = cstSource.c_str();
  boundCstHtml_.replace("\n", "<br>");
  boundCstHtml_.replace(" ", "&nbsp;");
  addSourceCodeHtmlLinks(cst, boundCstHtml_);
}

QString InstantiatedCompositeStateItem::makeHtml()
{
  QString html = QString("<h3><font color=\"darkred\">Instantiated Composite State</font> <a href=\"#this""\">") +
    replicodeObjects_.getLabel(newInstantiatedCompositeStateEvent_->object_).c_str() + "</a></h3>";
  html += factIcstHtml_;
  html += "<br><br>" + boundCstHtml_;
  return html;
}

}
