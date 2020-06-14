AERA Visualizer
===============

These are instructions to build and run the AERA Visualizer.

Prerequisites
=============

* Required: Replicode
* Required: Qt Framework, version 4.9 or greater
* Required: The code repository from https://github.com/IIIM-IS/AERA_Visualizer

Following are the detailed steps for each platform to install the prerequisites.

## Windows
To install Replicode, see https://github.com/IIIM-IS/replicode/blob/master/INSTALL.md . Be sure
to follow the instructions to set `WITH_DEBUG_OID` .

To install Qt Framework, download and install from https://www.qt.io/download-qt-installer .

To get the code repository, launch GitHub for Desktop and sign in to GitHub. In the File menu, 
click "Clone a Repository". Click the URL tab and enter `https://github.com/IIIM-IS/AERA_Visualizer` . 
It should be a recursive clone (which is the default).

Build
=====
Launch Visual Studio and open the project `Replicode.sln` from the cloned repository. E.g.:
  `C:\Users\Alice\Documents\GitHub\replicode\AERA_Visualizer.sln`

On the Build menu, click Build Solution. (Don't worry about all the compiler warnings.)

Run
===

The AERA Visualizer requires output from running AERA. See the file [INSTALL.md](https://github.com/IIIM-IS/replicode/blob/master/INSTALL.md)
for details. Be sure to change the `settings.xml` parameter `keep_invalidated_objects` to "yes", and set
`source_file_name` to a file such as "../Test/replicode_v1.2/pong.external.replicode" .

To run, in Visual Studio on the Debug menu, select "Run Without Debugging". Select the `settings.xml` file used by AERA, e.g.
`C:\Users\Alice\Documents\GitHub\replicode\Test\settings.xml` .
