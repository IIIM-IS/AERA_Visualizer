#include <QtWidgets>
#include "graph-visualizer-window.h"

namespace aera_visualizer {

GraphVisulizerWindow::GraphVisulizerWindow(AeraVisulizerWindowBase* mainWindow)
  : AeraVisulizerWindowBase(mainWindow)
{
  QVBoxLayout* centralLayout = new QVBoxLayout();
  QTextEdit* textEdit = new QTextEdit(this);
  centralLayout->addWidget(textEdit);
  centralLayout->addWidget(getPlayerControlPanel());

  QWidget* centralWidget = new QWidget();
  centralWidget->setLayout(centralLayout);
  setCentralWidget(centralWidget);

  setWindowTitle(tr("Graph Visualizer"));
  setUnifiedTitleAndToolBarOnMac(true);
}

}
