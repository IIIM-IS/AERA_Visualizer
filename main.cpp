#include "aera-visualizer-window.hpp"
#include "explanation-log-window.hpp"

#include <QApplication>
#include <QMessageBox>

using namespace aera_visualizer;

int main(int argv, char *args[])
{
  Q_INIT_RESOURCE(aera_visualizer);

  QApplication app(argv, args);

  string userOperatorsFilePath = "C:\\Users\\Jeff\\AERA\\replicode\\Test\\V1.2\\user.classes.replicode";
  string decompiledFilePath = "C:\\Users\\Jeff\\AERA\\replicode\\Test\\decompiled_objects.txt";

  ReplicodeObjects replicodeObjects;
  string error = replicodeObjects.init(userOperatorsFilePath, decompiledFilePath);
  if (error != "") {
    QMessageBox::information(NULL, "Compiler Error", error.c_str(), QMessageBox::Ok);
    return -1;
  }

  AeraVisulizerWindow mainWindow(replicodeObjects);
  // TODO: Use the actual screen resolution.
  const int left = 0;
  const int top = 35;
  const int width = 1620;
  const int height = 1000;
  mainWindow.setGeometry(left, top, width, height);

  auto explanationLogWindow = new ExplanationLogWindow(&mainWindow, replicodeObjects);
  mainWindow.setExplanationLogWindow(explanationLogWindow);
  // Disable the close button for the child window.
  explanationLogWindow->setWindowFlag(Qt::WindowCloseButtonHint, false);
  explanationLogWindow->setGeometry(left + width, top, 300, height);
  explanationLogWindow->show();

  mainWindow.show();

  return app.exec();
}
