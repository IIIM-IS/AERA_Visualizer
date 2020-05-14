#include <regex>
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>
#include "explanation-log-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "model-item.hpp"
#include "composite-state-item.hpp"
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

void InstantiatedCompositeStateItem::getIcstOrImdlValues(
  string source, std::vector<string>& templateValues, std::vector<string>& exposedValues)
{
  templateValues.clear();
  exposedValues.clear();

  // Debug: Generalize from these formats.
  smatch matches;
  if (regex_search(source, matches, regex("^\\(i\\w+ \\w+ \\|\\[\\] \\[([:\\.\\w]+) ([:\\.\\w]+)\\] \\w+ \\w+\\)$"))) {
    exposedValues.push_back(matches[1].str());
    exposedValues.push_back(matches[2].str());
  }
  else if (regex_search(source, matches, regex("^\\(i\\w+ \\w+ \\[([:\\.\\w]+) ([:\\.\\w]+) ([:\\.\\w]+)\\] \\[([:\\.\\w]+) ([:\\.\\w]+) ([:\\.\\w]+) ([:\\.\\w]+)\\] \\w+ \\w+\\)$"))) {
    templateValues.push_back(matches[1].str());
    templateValues.push_back(matches[2].str());
    templateValues.push_back(matches[3].str());
    exposedValues.push_back(matches[4].str());
    exposedValues.push_back(matches[5].str());
    exposedValues.push_back(matches[6].str());
    exposedValues.push_back(matches[7].str());
  }
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

  factIcstHtml_ = htmlify(factIcstHtml_);
  factIcstHtml_.replace("!down", DownArrowHtml);
  addSourceCodeHtmlLinks(icst, factIcstHtml_);
}

void InstantiatedCompositeStateItem::setBoundCstHtml()
{
  auto factIcst = (_Fact*)newInstantiatedCompositeStateEvent_->object_;
  auto icst = factIcst->get_reference(0);
  auto cst = icst->get_reference(0);

  string icstSource = replicodeObjects_.getSourceCode(icst);
  std::vector<string> templateValues;
  std::vector<string> exposedValues;
  getIcstOrImdlValues(icstSource, templateValues, exposedValues);

  string cstSource = CompositeStateItem::simplifyCstSource(replicodeObjects_.getSourceCode(cst));
  // Substitute variables.
  int iVariable = -1;
  size_t iTemplateValues = 0;
  size_t iExposedValues = 0;
  while (iTemplateValues < templateValues.size() || iExposedValues < exposedValues.size()) {
    ++iVariable;
    // v0, v1, v2, etc. are split between templateValues and exposedValues.
    string boundValue = (iTemplateValues < templateValues.size() ? 
      templateValues[iTemplateValues] : exposedValues[iExposedValues]);

    string variable = "v" + to_string(iVariable) + ":";
    cstSource = regex_replace(cstSource, regex(variable), variable + boundValue);

    if (iTemplateValues < templateValues.size())
      // Still looking at templateValues.
      ++iTemplateValues;
    else
      ++iExposedValues;
  }

  // Debug: How to correctly get the timestamp variables.
  auto afterVariable = "v" + to_string(++iVariable) + ":";
  auto beforeVariable = "v" + to_string(++iVariable) + ":";
  cstSource = regex_replace(cstSource, regex(afterVariable), replicodeObjects_.relativeTime(factIcst->get_after()));
  cstSource = regex_replace(cstSource, regex(beforeVariable), replicodeObjects_.relativeTime(factIcst->get_before()));

  boundCstHtml_ = htmlify(cstSource);
  addSourceCodeHtmlLinks(cst, boundCstHtml_);
  ModelItem::highlightVariables(boundCstHtml_);
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
