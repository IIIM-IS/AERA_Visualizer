#include <QtWidgets>
#include "graphics-items/aera-visualizer-scene.hpp"
#include "explanation-log-window.hpp"

using namespace std;

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
}

void ExplanationLogWindow::TextBrowser::mouseMoveEvent(QMouseEvent* event)
{
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
    if (object) {
      auto item = parent_->parent_->getScene()->getAeraGraphicsItem(object);
      if (item) {
        // Flash the corresponding item.
        item->borderFlashCountdown_ = 6;
        parent_->parent_->getScene()->establishFlashTimer();
      }
    }
  }
}

}
