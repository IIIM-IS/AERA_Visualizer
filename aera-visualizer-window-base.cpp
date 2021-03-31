//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020 Icelandic Institute for Intelligent Machines ses
//_/_/ Vitvelastofnun Islands ses, kt. 571209-0390
//_/_/ Author: Jeffrey Thompson <jeff@iiim.is>
//_/_/
//_/_/ -----------------------------------------------------------------------
//_/_/ Released under Open-Source BSD License with CADIA Clause v 1.0
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/   and collaboration notice, this list of conditions and the 
//_/_/   following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/   notice, this list of conditions and the following
//_/_/   disclaimer in the documentation and/or other materials provided 
//_/_/   with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/   contributors may be used to endorse or promote products 
//_/_/   derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause v 1.0: The license granted in and to the software under 
//_/_/   this agreement is a limited-use license. The software may not be used
//_/_/   in furtherance of: 
//_/_/   (i) intentionally causing bodily injury or severe emotional distress 
//_/_/   to any person; 
//_/_/   (ii) invading the personal privacy or violating the human rights of 
//_/_/   any person; or 
//_/_/   (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <ctime>
#include "aera-visualizer-window.hpp"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraVisulizerWindowBase::AeraVisulizerWindowBase(AeraVisulizerWindow* mainWindow, ReplicodeObjects& replicodeObjects)
: QMainWindow(mainWindow),
  mainWindow_(mainWindow),
  replicodeObjects_(replicodeObjects),
  timeReference_(seconds(0)),
  playTime_(seconds(0)),
  showRelativeTime_(true),
  playTimerId_(0),
  isPlaying_(false)
{
  simulationEventTypes_ = { 
    ModelGoalReduction::EVENT_TYPE, CompositeStateGoalReduction::EVENT_TYPE,
    ModelSimulatedPredictionReduction::EVENT_TYPE, CompositeStateSimulatedPredictionReduction::EVENT_TYPE };

  createPlayerControlPanel();

  if (mainWindow_)
    mainWindow_->children_.push_back(this);
}

void AeraVisulizerWindowBase::createPlayerControlPanel()
{
  QHBoxLayout* playerLayout = new QHBoxLayout();
  playIcon_ = QIcon(":/images/play.png");
  pauseIcon_ = QIcon(":/images/pause.png");
  playPauseButton_ = new QToolButton(this);
  connect(playPauseButton_, SIGNAL(clicked()), this, SLOT(playPauseButtonClicked()));
  playPauseButton_->setIcon(playIcon_);
  playerLayout->addWidget(playPauseButton_);

  stepBackButton_ = new QToolButton(this);
  stepBackButton_->setIcon(QIcon(":/images/play-step-back.png"));
  connect(stepBackButton_, SIGNAL(clicked()), this, SLOT(stepBackButtonClicked()));
  playerLayout->addWidget(stepBackButton_);
  playSlider_ = new QSlider(Qt::Horizontal, this);
  playSlider_->setMaximum(2000);
  connect(playSlider_, SIGNAL(valueChanged(int)), this, SLOT(playSliderValueChanged(int)));
  playerLayout->addWidget(playSlider_);
  stepButton_ = new QToolButton(this);
  stepButton_->setIcon(QIcon(":/images/play-step.png"));
  connect(stepButton_, SIGNAL(clicked()), this, SLOT(stepButtonClicked()));
  playerLayout->addWidget(stepButton_);

  playTimeLabel_ = new ClickableLabel("000s:000ms:000us", this);
  playTimeLabel_->setFont(QFont("Courier", 10));
  connect(playTimeLabel_, SIGNAL(clicked()), this, SLOT(playTimeLabelClicked()));
  playerLayout->addWidget(playTimeLabel_);

  playerControlPanel_ = new QWidget();
  playerControlPanel_->setLayout(playerLayout);
}

void AeraVisulizerWindowBase::startPlay()
{
  if (mainWindow_)
    mainWindow_->startPlayImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->startPlayImpl();
}

void AeraVisulizerWindowBase::stopPlay()
{
  if (mainWindow_)
    mainWindow_->stopPlayImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->stopPlayImpl();
}

void AeraVisulizerWindowBase::setPlayTime(Timestamp time)
{
  if (mainWindow_)
    mainWindow_->setPlayTimeImpl(time);
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->setPlayTimeImpl(time);
}

void AeraVisulizerWindowBase::setSliderToPlayTime()
{
  if (mainWindow_)
    mainWindow_->setSliderToPlayTimeImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->setSliderToPlayTimeImpl();
}

void AeraVisulizerWindowBase::playPauseButtonClicked()
{
  if (mainWindow_)
    mainWindow_->playPauseButtonClickedImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->playPauseButtonClickedImpl();
}

void AeraVisulizerWindowBase::stepButtonClicked()
{
  if (mainWindow_)
    mainWindow_->stepButtonClickedImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->stepButtonClickedImpl();
}

void AeraVisulizerWindowBase::stepBackButtonClicked()
{
  if (mainWindow_)
    mainWindow_->stepBackButtonClickedImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->stepBackButtonClickedImpl();
}

void AeraVisulizerWindowBase::playSliderValueChanged(int value)
{
  // TODO: Implement to check if the user moved the slider,
  // stopPlay, update the play time.
}

void AeraVisulizerWindowBase::playTimeLabelClicked()
{
  if (mainWindow_)
    mainWindow_->playTimeLabelClickedImpl();
  else
    // This is the main window.
    ((AeraVisulizerWindow*)this)->playTimeLabelClickedImpl();
}

void AeraVisulizerWindowBase::timerEvent(QTimerEvent* event)
{
  // TODO: Make sure we don't re-enter.

  if (event->timerId() != playTimerId_)
    // This timer event is not for us.
    return;

  if (events_.size() == 0) {
    stopPlay();
    return;
  }

  auto maximumEventTime = events_.back()->time_;
  // TODO: Make this track the passage of real clock time.
  auto playTime = playTime_ + AeraVisulizer_playTimerTick;

  // Step events while events_[iNextEvent_] is less than or equal to the playTime.
  // Debug: How to step the children also?
  while (stepEvent(playTime) != Utils_MaxTime);

  if (!haveMoreEvents()) {
    // We have played all events.
    playTime = maximumEventTime;
    stopPlay();
  }

  setPlayTime(playTime);
  setSliderToPlayTime();
}

ClickableLabel::ClickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
  : QLabel(text, parent) {}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* event) { emit clicked(); }

}
