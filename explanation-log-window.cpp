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
    uint64 debug_oid = url.url().mid(24).toULongLong();
    Code* requirementFactPred = replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (!requirementFactPred)
      return;
    Code* requirementPred = requirementFactPred->get_reference(0);
    Code* factImdl = requirementPred->get_reference(0);
    Code* imdl = factImdl->get_reference(0);
    Code* predictedModel = imdl->get_reference(0);

    // Debug: Get predictingModel and cause from its model reduction which made requirementFactPred.
    Code* predictingModel;
    Code* cause;
    if (debug_oid == 1029) {
      predictingModel = replicodeObjects_.getObject(59);
      cause = replicodeObjects_.getObject(60);
    }
    else if (debug_oid == 1899) {
      predictingModel = replicodeObjects_.getObject(59);
      cause = replicodeObjects_.getObject(72);
    }
    else
      return;

    auto menu = new QMenu();
    menu->addAction("What Is This?", [=]() {
      // TODO: Handle this in a static method of PredictionItem.
      // TODO: Handle the case of an anti-fact imdl prediction.
      string explanation = "<b>Q: What is requirement prediction " + replicodeObjects_.getLabel(requirementFactPred) +
        "?</b><br>Input <a href=\"#debug_oid-" + to_string(cause->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(cause) + 
        "</a> matched the LHS of model <a href=\"#debug_oid-" + to_string(predictingModel->get_debug_oid()) +
        "\">" + replicodeObjects_.getLabel(predictingModel) + 
        "</a> and the requirement is the RHS of the model, which predicts that model <a href=\"#debug_oid-" + 
        to_string(predictedModel->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(predictedModel) + 
        "</a> will succeed when instantiated with the given template values.<br><br>";
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
