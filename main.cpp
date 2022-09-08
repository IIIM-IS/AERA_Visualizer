//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
//_/_/ http://www.iiim.is
//_/_/
//_/_/ --- Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without
//_/_/ modification, is permitted provided that the following conditions
//_/_/ are met:
//_/_/ - Redistributions of source code must retain the above copyright
//_/_/   and collaboration notice, this list of conditions and the
//_/_/   following disclaimer.
//_/_/ - Redistributions in binary form must reproduce the above copyright
//_/_/   notice, this list of conditions and the following disclaimer 
//_/_/   in the documentation and/or other materials provided with 
//_/_/   the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its
//_/_/   contributors may be used to endorse or promote products
//_/_/   derived from this software without specific prior 
//_/_/   written permission.
//_/_/   
//_/_/ - CADIA Clause: The license granted in and to the software 
//_/_/   under this agreement is a limited-use license. 
//_/_/   The software may not be used in furtherance of:
//_/_/    (i)   intentionally causing bodily injury or severe emotional 
//_/_/          distress to any person;
//_/_/    (ii)  invading the personal privacy or violating the human 
//_/_/          rights of any person; or
//_/_/    (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
//_/_/ CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
//_/_/ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//_/_/ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//_/_/ DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//_/_/ CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//_/_/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//_/_/ SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//_/_/ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//_/_/ WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//_/_/ NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//_/_/ OF SUCH DAMAGE.
//_/_/ 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include "aera-visualizer-window.hpp"
#include "explanation-log-window.hpp"
#include "submodules/AERA/AERA/settings.h"

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QScreen>
#include <QProxyStyle>
#include <QProgressDialog>

using namespace std;
using namespace std::chrono;
using namespace aera_visualizer;

int main(int argv, char *args[])
{
  Q_INIT_RESOURCE(aera_visualizer);

  QApplication app(argv, args);

  // Override the tool tip style with 0 delay.
  class MyProxyStyle : public QProxyStyle
  {
  public:
    using QProxyStyle::QProxyStyle;
    int styleHint(StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const override {
      if (hint == QStyle::SH_ToolTip_WakeUpDelay) { return 0; }
      else if (hint == QStyle::SH_ToolTip_FallAsleepDelay) { return 0; }
      return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
  };
  app.setStyle(new MyProxyStyle(qApp->style()));

  // Globally remove the '?' from the QInputDialog title bar.
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

  // Set the organization and application name
  // Enables using the settings from anywhere
  QCoreApplication::setOrganizationName("IIIM");
  QCoreApplication::setApplicationName("AERA_Visualizer");

  // Configure QSettings to use .ini files to store settings
  QSettings::setDefaultFormat(QSettings::IniFormat);

  QSettings preferences;

  QString settingsFilePath0 = preferences.value("settingsFilePath").toString();
  if (settingsFilePath0 == "")
    settingsFilePath0 = "../AERA/AERA/settings.xml";
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

  // Create the progress dialog to show while compiling and reading the runtime output.
  QProgressDialog progress("", "Cancel", 0, 100);
  progress.setWindowModality(Qt::WindowModal);
  // Remove the '?' in the title.
  progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  progress.setWindowIcon(QIcon(":/images/app.ico"));
  progress.setWindowTitle("Initializing");
  progress.setAutoReset(false);
  progress.setAutoClose(false);
  progress.show();
  QApplication::processEvents();

  ReplicodeObjects replicodeObjects;
  string error = replicodeObjects.init(
    settingsFileDir.absoluteFilePath(settings.usr_class_path_.c_str()).toStdString(), 
    settingsFileDir.absoluteFilePath(settings.decompilation_file_path_.c_str()).toStdString(),
    microseconds(settings.base_period_), progress);
  if (error == "cancel")
    return -1;
  if (error != "") {
    QMessageBox::information(NULL, "Compiler Error", error.c_str(), QMessageBox::Ok);
    return -1;
  }

  AeraVisulizerWindow mainWindow(replicodeObjects);
  mainWindow.setWindowIcon(QIcon(":/images/app.ico"));

  if (!mainWindow.addEvents(runtimeOutputFilePath, progress))
    return -1;

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

  progress.close();
  mainWindow.show();
  mainWindow.addStartupItems();

  return app.exec();
}
