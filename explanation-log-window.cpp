#include <QtWidgets>
#include "submodules/replicode/r_exec/opcodes.h"
#include "graphics-items/program-reduction-item.hpp"
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
  if (url.url().startsWith("#debug_oid-")) {
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
          "</b><br>It has the following sets of input objects and output actions.<br>" + 
          reductionHtml.toStdString() + "<br><br>";
        appendHtml(explanation);
      });
      menu->exec(textBrowser_->mouseScreenPosition_ - QPoint(10, 10));
      delete menu;
    }
    // Debug: Replace this with a search for the MkRdx output.
    else if (object->get_oid() == 49 || object->get_oid() == 68) {
      // There is no item for an program output, so show a menu for a "What Made This" explanation.
      auto menu = new QMenu();
      menu->addAction("What Made This?", [=]() {
        // TODO: Handle this in a static method of ProgramOutputFactItem.
        Code* mkRdx;
        if (object->get_oid() == 49)
          mkRdx = replicodeObjects_.getObject(47);
        else if (object->get_oid() == 68)
          mkRdx = replicodeObjects_.getObject(58);

        string explanation = "<b>Q: What made program output <a href=\"#debug_oid-" +
          to_string(object->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(object) + "</a> ?</b><br>" +
          "This is an output of instantiated program <b>" + replicodeObjects_.getLabel(mkRdx->get_reference(0)) +
          "</b>, according to<br>program reduction <a href=\"#debug_oid-" +
          to_string(mkRdx->get_debug_oid()) + "\">" + replicodeObjects_.getLabel(mkRdx) + "</a><br><br>";
        appendHtml(explanation);
      });
      menu->exec(textBrowser_->mouseScreenPosition_ - QPoint(10, 10));
      delete menu;
    }
    else {
      if (!parent_->hasAeraGraphicsItem(object))
        return;

      // Show the menu.
      auto menu = new QMenu();
      menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
        [=]() { parent_->zoomToAeraGraphicsItem(object); });
      menu->exec(textBrowser_->mouseScreenPosition_ - QPoint(10, 10));
      delete menu;
    }
  }
}

void ExplanationLogWindow::TextBrowser::mouseMoveEvent(QMouseEvent* event)
{
  mouseScreenPosition_ = event->globalPos();

  auto url = anchorAt(event->pos());
  if (url == "") {
    // The mouse cursor exited the link.
    previousUrl_ = "";
    return;
  }
  if (url == previousUrl_)
    // Still hovering the same link, so do nothing.
    return;

  previousUrl_ = url;
  if (url.startsWith("#debug_oid-")) {
    uint64 debug_oid = url.mid(11).toULongLong();
    auto object = parent_->replicodeObjects_.getObjectByDebugOid(debug_oid);
    if (object)
      parent_->parent_->flashAeraGraphicsItem(object);
  }
}

}
