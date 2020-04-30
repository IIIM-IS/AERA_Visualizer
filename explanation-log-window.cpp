#include <QtWidgets>
#include "explanation-log-window.hpp"

namespace aera_visualizer {

ExplanationLogWindow::ExplanationLogWindow(AeraVisulizerWindowBase* mainWindow)
  : AeraVisulizerWindowBase(mainWindow)
{
  auto centralLayout = new QVBoxLayout();
  auto textEdit = new QTextEdit(this);
  textEdit->setReadOnly(true);
  centralLayout->addWidget(textEdit);
  centralLayout->addWidget(getPlayerControlPanel());

  QWidget* centralWidget = new QWidget();
  centralWidget->setLayout(centralLayout);
  setCentralWidget(centralWidget);

  setWindowTitle(tr("Explanation Log"));
  setUnifiedTitleAndToolBarOnMac(true);
}

}
