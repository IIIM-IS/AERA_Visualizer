#include "aera-visualizer-window.hpp"
#include "graph-visualizer-window.hpp"

#include <QApplication>

using namespace aera_visualizer;

int main(int argv, char *args[])
{
  Q_INIT_RESOURCE(aera_visualizer);

  QApplication app(argv, args);
  AeraVisulizerWindow mainWindow;
  const int left = 10;
  const int top = 100;
  const int width = 800;
  const int height = 500;
  mainWindow.setGeometry(left, top, width, height);

  GraphVisulizerWindow* graphWindow = new GraphVisulizerWindow(&mainWindow);
  // Disable the close button for the child window.
  graphWindow->setWindowFlag(Qt::WindowCloseButtonHint, false);
  graphWindow->setGeometry(left + width, top, width, height);
  graphWindow->show();

  mainWindow.show();

  return app.exec();
}
