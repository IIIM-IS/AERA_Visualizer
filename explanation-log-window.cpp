#include <QtWidgets>
#include "submodules/replicode/r_exec/opcodes.h"
#include "graphics-items/program-reduction-item.hpp"
#include "graphics-items/aera-visualizer-scene.hpp"
#include "aera-visualizer-window.hpp"
#include "explanation-log-window.hpp"

using namespace std;
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

        string explanation = "<u>Q: What is program reduction " + replicodeObjects_.getLabel(object) +
          "?</u><br>This the notification of a reduction of instantiated program <b>" + 
          replicodeObjects_.getLabel(object->get_reference(0)) + 
          "</b><br>It has the following sets of input objects and output actions.<br>" + 
          reductionHtml.toStdString() + "<br><br>";
        appendHtml(explanation);
      });
      menu->exec(textBrowser_->mouseScreenPosition_ - QPoint(10, 10));
      delete menu;
    }
    else {
      auto item = parent_->getScene()->getAeraGraphicsItem(object);
      if (!item)
        return;

      // Show the menu.
      auto menu = new QMenu();
      menu->addAction(QString("Zoom to ") + replicodeObjects_.getLabel(object).c_str(),
        [=]() { parent_->getScene()->zoomToItem(item); });
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
      parent_->parent_->flashItem(object);
  }
}

}
