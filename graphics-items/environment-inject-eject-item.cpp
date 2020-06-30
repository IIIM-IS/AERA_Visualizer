//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020 Icelandic Institute for Intelligent Machines ses
//_/_/ Vitvelastofnun Islands ses, kt. 571209-0390
//_/_/ Author: Jeffrey Thompson <jeff@iiim.is>
//_/_/
//_/_/ -----------------------------------------------------------------------
//_/_/ Released under Open-Source BSD License with CADIA Clause v 1.0
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/   and collaboration notice, this list of conditions and the 
//_/_/   following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/   notice, this list of conditions and the following
//_/_/   disclaimer in the documentation and/or other materials provided 
//_/_/   with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/   contributors may be used to endorse or promote products 
//_/_/   derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause v 1.0: The license granted in and to the software under 
//_/_/   this agreement is a limited-use license. The software may not be used
//_/_/   in furtherance of: 
//_/_/   (i) intentionally causing bodily injury or severe emotional distress 
//_/_/   to any person; 
//_/_/   (ii) invading the personal privacy or violating the human rights of 
//_/_/   any person; or 
//_/_/   (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <regex>
#include <QMenu>
#include "explanation-log-window.hpp"
#include "../aera-visualizer-window.hpp"
#include "aera-visualizer-scene.hpp"
#include "environment-inject-eject-item.hpp"

using namespace std;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

const QString EnvironmentInjectEjectItem::UpWideArrowHtml("<b>&#129093;</b>");
const QString EnvironmentInjectEjectItem::DownWideArrowHtml("<b>&#129095;</b>");

EnvironmentInjectEjectItem::EnvironmentInjectEjectItem(
  AeraEvent* event, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent)
: AeraGraphicsItem(event, replicodeObjects, parent, "Eject"),
  event_(event)
{
  setLabelHtml();
  setFactValHtml();

  // Set up the textItem_ first to get its size.
  if (textItem_)
    delete textItem_;
  textItem_ = new TextItem(this);
  textItem_->setHtml(labelHtml_);
  // adjustSize() is needed for right-aligned text.
  textItem_->adjustSize();

  // Position the item origin on the arrow.
  qreal left = -4;
  qreal top = -textItem_->boundingRect().height() / 2 + 5;
  textItem_->setPos(left -5, top - 5);
  textItem_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  QObject::connect(textItem_, &QGraphicsTextItem::linkActivated,
    [this](const QString& link) { textItemLinkActivated(link); });

  qreal right = left + textItem_->boundingRect().width() - 5;
  qreal bottom = textItem_->boundingRect().height() / 2 - 2;

  // Make a simple rectangle.
  QPainterPath path;
  path.moveTo(right, top);
  path.lineTo(left, top);
  path.lineTo(left, bottom);
  path.lineTo(right, bottom);
  auto polygon = path.toFillPolygon();
  setPolygon(polygon);

  // Blend with the background. Below, we override paint() to use the parent's background color.
  borderNoHighlightPen_ = QPen(parent_->backgroundBrush().color(), 1);
}

void EnvironmentInjectEjectItem::setLabelHtml()
{
  labelHtml_ = (event_->eventType_ == EnvironmentInjectEvent::EVENT_TYPE ? DownWideArrowHtml : UpWideArrowHtml) +
    "<a href=\"#this\">" + replicodeObjects_.getLabel(event_->object_).c_str() + "</a>";
}

void EnvironmentInjectEjectItem::setFactValHtml()
{
  auto val = event_->object_->get_reference(0);

  // Strip the ending confidence value and propagation of saliency threshold.
  regex saliencyRegex("\\s+[\\w\\:]+\\)$");
  regex confidenceAndSaliencyRegex("\\s+\\w+\\s+[\\w\\:]+\\)$");
  string factSource = regex_replace(replicodeObjects_.getSourceCode(
    event_->object_), confidenceAndSaliencyRegex, ")");
  string valSource = regex_replace(replicodeObjects_.getSourceCode(val), saliencyRegex, ")");

  QString valLabel(replicodeObjects_.getLabel(val).c_str());

  factValHtml_ = QString(factSource.c_str()).replace(valLabel, DownArrowHtml);
  factValHtml_ += QString("\n      ") + valSource.c_str();

  addSourceCodeHtmlLinks(val, factValHtml_);
  factValHtml_ = htmlify(factValHtml_);
}

void EnvironmentInjectEjectItem::textItemLinkActivated(const QString& link)
{
  if (link == "#this") {
    auto menu = new QMenu();
    menu->addAction("What Is This?", [=]() {
      QString explanation;
      if (event_->eventType_ == EnvironmentInjectEvent::EVENT_TYPE)
        explanation = "<b>Q: What is inject " + makeHtmlLink(event_->object_) +
          " ?</b><br>This fact was injected from the environment:<br>" + factValHtml_ + "<br><br>";
      else {
        auto ejectEvent = (EnvironmentEjectEvent*)event_;
        explanation = "<b>Q: What is eject " + makeHtmlLink(ejectEvent->object_) +
          " ?</b><br>A command was ejected to the environment:<br>" + factValHtml_;
        if (ejectEvent->reduction_)
          explanation += QString("<br><br>It was ejected by instantiated program <b>") + 
            replicodeObjects_.getLabel(ejectEvent->reduction_->get_reference(0)).c_str() +
            "</b>, according to program reduction " + makeHtmlLink(ejectEvent->reduction_) + " .";

        explanation += "<br><br>";
      }

      parent_->getParent()->getExplanationLogWindow()->appendHtml(explanation);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
}

void EnvironmentInjectEjectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  // TODO: Why can't we just set the brush in the constructor?
  setBrush(parent_->backgroundBrush());
  AeraGraphicsItem::paint(painter, option, widget);
}

}
