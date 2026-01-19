//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2023 Jeff Thompson
//_/_/ Copyright (c) 2018-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2023-2026 Chloe Schaff
//_/_/ Copyright (c) 2018-2023 Icelandic Institute for Intelligent Machines
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


#include "text-output.hpp"
#include "../aera-visualizer-window.hpp"

#include <QTabWidget>
#include <QTextBrowser>


namespace aera_visualizer {

	TextOutputView::TextOutputView(AeraVisualizerWindow* mainWindow)
		: QDockWidget("Raw Data", mainWindow)
	{

		// Set up the browsers
		decompiledObjectsBrowser_ = new QTextBrowser(this);
		runtimeOutBrowser_ = new QTextBrowser(this);
		decompiledObjectsBrowser_->setText("No AERA output to read");
		runtimeOutBrowser_->setText("No AERA output to read");

		// Put it all in a QTabWidget
		QTabWidget* tabs = new QTabWidget(this);
		tabs->setObjectName("textoutput_container");
		tabs->addTab(decompiledObjectsBrowser_, "Decompiled Objects");
		tabs->addTab(runtimeOutBrowser_, "Runtime Output");

		setWidget(tabs);
	}

	void TextOutputView::refresh() {
		// TO DO: Refreshing should only load new lines without changing what's currently
		// displayed so you don't lose your spot every time you step AERA forwards.
		
		// Check that these are set
		if (decompiledFilePath_.empty() || runtimeOutFilePath_.empty())
			return;

		// Open the files
		ifstream decompiledObjectsFile(decompiledFilePath_);
		ifstream runtimeOutputFile(runtimeOutFilePath_);

		// Load in decompiled objects
		string decompiledObjects;
		string line;
		while (getline(decompiledObjectsFile, line))
			decompiledObjects += line + "\n";

		decompiledObjectsBrowser_->setText(QString::fromStdString(decompiledObjects));

		// Load in runtime output
		string runtimeOut;
		while (getline(runtimeOutputFile, line))
			runtimeOut += line + "\n";

		runtimeOutBrowser_->setText(QString::fromStdString(runtimeOut));
	}
}
