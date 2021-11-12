AERA Visualizer
===============

These are instructions to build and run the AERA Visualizer.

Prerequisites
=============

* Required: AERA
* Required: Qt Framework, version 5.14.2
* Required: The AERA Visualizer code repository from https://github.com/IIIM-IS/AERA_Visualizer . Do a "recursive" clone to get submodules, e.g.:

    git clone --recursive https://github.com/IIIM-IS/AERA_Visualizer

Following are the detailed steps for each platform to install the prerequisites.

## Windows
To install AERA, see https://github.com/IIIM-IS/AERA/blob/master/INSTALL.md . Be sure
to follow the instructions to set `WITH_DETAIL_OID` .

To install Qt Framework:

* Download and install from https://www.qt.io/download-qt-installer . In the
  installer, select "Custom installation". Expand "Qt" and expand version
  "5.14.2". Select your MSVC 64-bit version, e.g. "MSVC 2017 64.bit" When the installer is
  finished, you can uncheck "Launch Qt Creator" since we don't need it. 
* With Visual Studio closed, download and install the Qt Visual Studio Tools from
  https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019 .
* Launch Visual Studio. On the "splash" screen, click "continue without code". On the Extensions menu, click 
  "Qt VS Tools" then "Options". On the "Qt Versions" tab, click "<add new Qt version>". Click the folder icon and 
  select the path of the qmake executable, e.g. `C:\Qt\5.14.2\msvc2017_64\bin\qmake.exe` . Click "OK".

To get the AERA Visualizer code repository, launch GitHub for Desktop and sign in to GitHub. In the File menu, 
click "Clone a Repository". Click the URL tab and enter `https://github.com/IIIM-IS/AERA_Visualizer` . 
It should be a recursive clone (which is the default).

Build
=====
Launch Visual Studio and open the project `AERA_Visualizer.sln` from the cloned repository. E.g.:
`C:\Users\Alice\Documents\GitHub\AERA_Visualizer\AERA_Visualizer.sln` .

In the Solution Configurations drop-down, make sure you select Release (unless you plan to debug the Visualizer).
On the Build menu, click Build Solution. (Don't worry about all the compiler warnings.)

Run in Visual Studio
====================

The AERA Visualizer requires output from running AERA. See the file [INSTALL.md](https://github.com/IIIM-IS/AERA/blob/master/INSTALL.md)
for details. Be sure to change the `settings.xml` parameter `keep_invalidated_objects` to "yes", and set
`source_file_name` to a file such as "../AERA/replicode_v1.2/ball.external.replicode" .

To run, in Visual Studio on the Debug menu, select "Run Without Debugging". Select the `settings.xml` file used by AERA, e.g.
`C:\Users\Alice\Documents\GitHub\replicode\AERA\settings.xml` .

Run from AeraVisualizer.exe
===========================

As shown above, you can run AERA Visualizer from Visual Studio. To run AeraVisualizer.exe directly (after building in Visual Studio),
you need to put the Qt bin folder in the system path as follows:

* Open the System control panel.
* Click Advanced System Settings.
* Click Environment Variables
* In the "System variables" section, click Path and click Edit....
* In the "Edit environment variable" window, click New.
* Paste the path of the Qt bin folder that you used above. For example, `C:\Qt\5.14.2\msvc2017_64\bin`
* Repeatedly click OK to close all the windows.
* Restart your computer to update the Path for the whole system.
* After restart, open a Command Prompt and enter the following, changing `c:\Users\user\AERA_Visualizer` to
  the folder where you cloned the AERA Visualizer project.

    windeployqt c:\Users\user\AERA_Visualizer\x64\Release\AeraVisualizer.exe

Now, in the Windows Explorer, you can double click AeraVisualizer.exe in that folder. You can also right-click and click
"Create Shortcut" and put the shortcut on your desktop.
