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


#include "semantics.hpp"
#include "../aera-visualizer-window.hpp"

#include <QGraphicsView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>


namespace aera_visualizer {

SemanticsView::SemanticsView(AeraVisualizerWindow* mainWindow)
	: QDockWidget("Models and Composite States", mainWindow)
{
	QWidget* container = new QWidget();
	container->setObjectName("semantics_container");
	//container->setStyleSheet("QWidget#container { background-color: rgb(255,0,0); margin:5px; border:1px solid rgb(0, 0, 0); }");

	modelsScene_ = new AeraVisualizerScene(mainWindow, false);

	
	zoomInAction_ = new QAction(QIcon(":/images/zoom-in.png"), tr("Zoom In"), this);
	zoomInAction_->setStatusTip(tr("Zoom In"));
	zoomInAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Equal));
	connect(zoomInAction_, SIGNAL(triggered()), this, SLOT(zoomIn()));

	zoomOutAction_ = new QAction(QIcon(":/images/zoom-out.png"), tr("Zoom Out"), this);
	zoomOutAction_->setStatusTip(tr("Zoom Out"));
	zoomOutAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));
	connect(zoomOutAction_, SIGNAL(triggered()), this, SLOT(zoomOut()));

	zoomHomeAction_ = new QAction(QIcon(":/images/zoom-home.png"), tr("Zoom Home"), this);
	zoomHomeAction_->setStatusTip(tr("Zoom to show all"));
	zoomHomeAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Home));
	connect(zoomHomeAction_, SIGNAL(triggered()), this, SLOT(zoomHome()));

	auto modelsSceneView = new QGraphicsView(modelsScene_, this);
	modelsSceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	modelsSceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	modelsSceneView->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

	QToolBar* zoomControls = new QToolBar(this);
	zoomControls->addAction(zoomHomeAction_);
	zoomControls->addAction(zoomInAction_);
	zoomControls->addAction(zoomOutAction_);
	zoomControls->setIconSize(QSize(16, 16));

	QVBoxLayout* stack = new QVBoxLayout(this);
	stack->addWidget(modelsSceneView);
	stack->addWidget(zoomControls);
	container->setLayout(stack);

	setWidget(container);
}

void SemanticsView::zoomIn()
{
	modelsScene_->scaleViewBy(1.09);
}

void SemanticsView::zoomOut()
{
	modelsScene_->scaleViewBy(1 / 1.09);
}

void SemanticsView::zoomHome()
{
	modelsScene_->zoomViewHome();
}

}