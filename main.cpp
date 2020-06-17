#include "aera-visualizer-window.hpp"
#include "explanation-log-window.hpp"
#include "submodules/replicode/AERA/settings.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QScreen>

using namespace aera_visualizer;

int main(int argv, char *args[])
{
  Q_INIT_RESOURCE(aera_visualizer);

  QApplication app(argv, args);

  QSettings preferences("IIIM", "AERA_Visualizer");
  QString settingsFilePath0 = preferences.value("settingsFilePath").toString();
  if (settingsFilePath0 == "")
    settingsFilePath0 = "../replicode/AERA/settings.xml";
  QString settingsFilePath = QFileDialog::getOpenFileName(NULL,
    "Open AERA settings XML file", settingsFilePath0, "XML Files (*.xml);;All Files (*.*)");
  if (settingsFilePath == "")
    return 0;
  preferences.setValue("settingsFilePath", settingsFilePath);

  Settings settings;
  if (!settings.load(settingsFilePath.toStdString().c_str())) {
    QMessageBox::information(NULL, "XML Error", "Cannot load XML file " + settingsFilePath, QMessageBox::Ok);
    return -1;
  }

  // Files are relative to the directory of settingsFilePath.
  QDir settingsFileDir = QFileInfo(settingsFilePath).dir();
  ReplicodeObjects replicodeObjects;
  string error = replicodeObjects.init(
    settingsFileDir.absoluteFilePath(settings.usr_class_path_.c_str()).toStdString(), 
    settingsFileDir.absoluteFilePath(settings.decompilation_file_path_.c_str()).toStdString());
  if (error != "") {
    QMessageBox::information(NULL, "Compiler Error", error.c_str(), QMessageBox::Ok);
    return -1;
  }

  string runtimeOutputFilePath = settingsFileDir.absoluteFilePath(settings.runtime_output_file_path_.c_str()).toStdString();
  {
    // Test opening the file now so we can exit on error.
    ifstream testOpen(runtimeOutputFilePath);
    if (!testOpen) {
      QMessageBox::information(NULL, "File Error", 
        QString("Can't open debug stream output file: ") + runtimeOutputFilePath.c_str(), QMessageBox::Ok);
      return -1;
    }
  }
  AeraVisulizerWindow mainWindow(replicodeObjects, runtimeOutputFilePath);
  mainWindow.setWindowTitle(QString("AERA Visualizer - ") + QFileInfo(settings.source_file_name_.c_str()).fileName());
  QScreen* screen = QGuiApplication::primaryScreen();
  int availableHeight = screen->availableSize().height();
  int availableWidth = screen->availableSize().width();
  const int titleBarHeight = 35;
  const int explanationLogWindowWidth = 315;
  const int left = 0;
  const int top = titleBarHeight;
  const int width = availableWidth - explanationLogWindowWidth;
  const int height = availableHeight - titleBarHeight;
  mainWindow.setGeometry(left, top, width, height);

  auto explanationLogWindow = new ExplanationLogWindow(&mainWindow, replicodeObjects);
  mainWindow.setExplanationLogWindow(explanationLogWindow);
  // Disable the close button for the child window.
  explanationLogWindow->setWindowFlag(Qt::WindowCloseButtonHint, false);
  explanationLogWindow->setGeometry(left + width, top, explanationLogWindowWidth, height);
  explanationLogWindow->show();

  mainWindow.show();

  return app.exec();
}
