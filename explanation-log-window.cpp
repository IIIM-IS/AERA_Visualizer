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

#include <QtWidgets>
#include "submodules/replicode/r_exec/opcodes.h"
#include "graphics-items/program-reduction-item.hpp"
#include "graphics-items/environment-inject-eject-item.hpp"
#include "aera-visualizer-window.hpp"
#include "explanation-log-window.hpp"

using namespace std;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

ExplanationLogWindow::ExplanationLogWindow(AeraVisulizerWindow* mainWindow, ReplicodeObjects& replicodeObjects)
  : AeraVisulizerWindowBase(mainWindow),
  parent_(mainWindow), replicodeObjects_(replicodeObjects)
{
  auto centralLayout = new QVBoxLayout();
  textBrowser_ = new TextBrowser(this);
  connect(textBrowser_, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(textBrowserAnchorClicked(const QUrl&)));
  centralLayout->addWidget(textBrowser_);
  centralLayout->addWidget(getPlayerControlPanel());

  QWidget* centralWidget = new QWidget();
  centralWidget->setLayout(centralLayout);
  setCentralWidget(centralWidget);

  setWindowTitle(tr("Explanation Log"));
  setUnifiedTitleAndToolBarOnMac(true);
}

void ExplanationLogWindow::textBrowserAnchorClicked(const QUrl& url)
{
  if (url.url().startsWith("#requirement_prediction-")) {
    // There is no item for a requirement prediction, so show a menu for a "What Is" explanation.
    // TODO: This should be the debug_oid of an mk.rdx, which Replicode currently doesn't make. 
    int imdlPredictionEventIndex = url.url().mid(24).toULongLong();
    auto imdlPredictionEvent = (ModelImdlPredictionEvent*)parent_->getAeraEvent(imdlPredictionEventIndex);
    Code* predictingModel = imdlPredictionEvent->predictingModel_;
    Code* cause = imdlPredictionEvent->cause_;
    Code* requirementFactPred = imdlPredictionEvent->object_;
    Code* requirementPred = requirementFactPred->get_reference(0);
    Code* factImdl = requirementPred->get_reference(0);
    Code* imdl = factImdl->get_reference(0);
    Code* predictedModel = imdl->get_reference(0);

    auto menu = new QMenu();
    menu->addAction("What Is This?", [=]() {
      // TODO: Handle this in a static method of PredictionItem.
      // TODO: Handle the case of an anti-fact imdl prediction.
      QString explanation = QString("<b>Q: What is requirement prediction ") + replicodeObjects_.getLabel(requirementFactPred).c_str() +
        "?</b><br>Input " + AeraGraphicsItem::makeHtmlLink(cause, replicodeObjects_) +
        " matched the LHS of model " + AeraGraphicsItem::makeHtmlLink(predictingModel, replicodeObjects_) +
        " and the requirement is the RHS of the model, which predicts that model " + 
        AeraGraphicsItem::makeHtmlLink(predictedModel, replicodeObjects_) +
        " will succeed when instantiated with the given template values.<br><br>";
      appendHtml(explanation);
    });
    menu->exec(QCursor::pos() - QPoint(10, 10));
    delete menu;
  }
  else if (url.url().startsWith("#debug_oid-")) {
    uint64 debug_oid = url.url().mid(11).toULongLong();
    auto object = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (!object)
      return;

    if (object->code(0).asOpcode() == Opcodes::MkRdx) {
      // There is no item for an mk.rdx, so show a menu for a "What Is" explanation.
      auto menu = new QMenu();
      menu->addAction("What Is This?", [=]() {
        // TODO: Handle this in a static method of ProgramReductionItem.
        QString reductionHtml = AeraGraphicsItem::htmlify(ProgramReductionItem::simplifyMkRdxSource(
          replicodeObjects_.getSourceCode(object)));
        AeraGraphicsItem::addSourceCodeHtmlLinks(object, reductionHtml, replicodeObjects_);

        string explanation = "<b>Q: What is program reduction " + replicodeObjects_.getLabel(object) +
          "?</b><br>This the notification of a reduction of instantiated program <b>" + 
          replicodeObjects_.getLabel(object->get_reference(0)) + 
          "</b> . It has the following sets of input objects and output actions.<br>" + 
          reductionHtml.toStdString() + "<br><br>";
        appendHtml(explanation);
      });
      menu->exec(QCursor::pos() - QPoint(10, 10));
      delete menu;
    }
    else {
      auto item = parent_->getAeraGraphicsItem(object);
      if (!item)
        return;
      if (dynamic_cast<EnvironmentInjectEjectItem*>(item))
        // The inject/eject items are too small to zoom to.
        return;

      // Show the menu.
      auto menu = new QMenu();
      menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
        [=]() { parent_->zoomToAeraGraphicsItem(object); });
      menu->exec(QCursor::pos() - QPoint(10, 10));
      delete menu;
    }
  }
}

void ExplanationLogWindow::TextBrowser::mouseMoveEvent(QMouseEvent* event)
{
  parent_->parent_->textItemHoverMoveEvent(document(), event->pos());

  QTextBrowser::mouseMoveEvent(event);
}

}
