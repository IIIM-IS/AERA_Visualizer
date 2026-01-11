//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2023 Jeff Thompson
//_/_/ Copyright (c) 2018-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2023 Chloe Schaff
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

#ifndef INTERNAL_ENV_HPP
#define INTERNAL_ENV_HPP

#include <QDockWidget>
#include <QBrush>

#include "../aera-visualizer-window.hpp"
#include "../replicode-objects.hpp"


using namespace std;

namespace aera_visualizer {

/**
* EnvCanvas extends QWidget so we can paint on it. It's
* only here to provide a dedicated spot to paint drawings
* of the current simulation status.
*/
class EnvCanvas : public QWidget {
	Q_OBJECT

public:
	EnvCanvas(QWidget* parent);

	void setState(string identifier, std::chrono::milliseconds time, float posY, float velY, float forceY) {
		identifier_ = identifier;
		time_ = time;
		positionY_ = posY;
		velocityY_ = velY;
		forceY_ = forceY;

		update();
	}

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	// Various drawing functions
	void drawTimestamp(QPainter& painter);
	void drawHticks(int y, float min, float max, float interval, QPainter& painter);

	string identifier_;
	std::chrono::milliseconds time_;
	float positionY_;
	float velocityY_;
	float forceY_;

	QBrush foregroundBrush_;
	QBrush backgroundBrush_;
	QBrush objectBrush_;
	QBrush transparentBrush_;
	QPixmap logo_;
};


/**
* TaskEnvironmentView extends QDockWidget to allow the user to
* rearrange it as needed. It shows a graphical representation of
* the current status of a program in test_mem, Webots, or any
* other environment AERA may be working in. For now, only select
* test_mem environments are supported
*/
class TaskEnvironmentView : public QDockWidget
{
	Q_OBJECT

public:
	/**
		* Create an TaskEnvironmentView.
		* \param mainWindow The main parent window for this window.
		*/
	TaskEnvironmentView(AeraVisualizerWindow* mainWindow);

	// Called when there's an update to draw
	void refresh();

	// Used to update the replicodeObjects (if you need access to those)
	void setReplicodeObjects(ReplicodeObjects* replicodeObjects) {
		replicodeObjects_ = replicodeObjects;
	}

	// Provide a link to test_mem. Make sure we have settings->get_objects_ set
	void setAERA(AERA_interface* aera);

private:
	ReplicodeObjects* replicodeObjects_;
	EnvCanvas* canvas_;
	AERA_interface* aera_;

	string identifier_;
	float positionY_;
	float velocityY_;
	float forceY_;

	QLabel* identifierLabel_;
	QLabel* firstDataLabel_;
	QLabel* secondDataLabel_;
	QLabel* thirdDataLabel_;
};
}

#endif
