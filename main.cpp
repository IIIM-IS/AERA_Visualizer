#include "aera-visualizer-window.hpp"
#include "explanation-log-window.hpp"

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

  auto explanationLogWindow = new ExplanationLogWindow(&mainWindow);
  // Disable the close button for the child window.
  explanationLogWindow->setWindowFlag(Qt::WindowCloseButtonHint, false);
  explanationLogWindow->setGeometry(left + width, top, width, height);
  explanationLogWindow->show();

  mainWindow.show();

  return app.exec();
}
