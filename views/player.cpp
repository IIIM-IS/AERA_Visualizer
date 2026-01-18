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

#include <ctime>
#include "player.hpp"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

PlayerView::PlayerView(AeraVisualizerWindow* mainWindow)
: QDockWidget("Timeline", mainWindow),
  mainWindow_(mainWindow),
  isPlaying_(false),
  showRelativeTime_(true),
  playTime_(seconds(0)),
  aeraTime_(seconds(0)),
  playTimerId_(0)
{
  // No title bar
  setTitleBarWidget(new QWidget());

  // Set up icons
  playIcon_ = QIcon(":/images/play.png");
  pauseIcon_ = QIcon(":/images/pause.png");

  // Set up buttons
  jumpToStartButton_ = new QToolButton();
  jumpToStartButton_->setIcon(QIcon(":/images/jump-start.png"));
  connect(jumpToStartButton_, SIGNAL(clicked()), this, SLOT(vis_jumpToStartButtonClicked()));

  jumpToEndButton_ = new QToolButton();
  jumpToEndButton_->setIcon(QIcon(":/images/jump-end.png"));
  connect(jumpToEndButton_, SIGNAL(clicked()), this, SLOT(vis_jumpToEndButtonClicked()));

  playPauseButton_ = new QToolButton(this);
  playPauseButton_->setIcon(playIcon_);
  connect(playPauseButton_, SIGNAL(clicked()), this, SLOT(vis_playPauseButtonClicked()));

  stepBackButton_ = new QToolButton(this);
  stepBackButton_->setIcon(QIcon(":/images/play-step-back.png"));
  connect(stepBackButton_, SIGNAL(clicked()), this, SLOT(vis_stepBackButtonClicked()));

  stepFwdButton_ = new QToolButton(this);
  stepFwdButton_->setIcon(QIcon(":/images/play-step.png"));
  connect(stepFwdButton_, SIGNAL(clicked()), this, SLOT(vis_stepFwdButtonClicked()));

  aeraPlayPauseButton_ = new QToolButton();
  aeraPlayPauseButton_->setIcon(playIcon_);
  connect(aeraPlayPauseButton_, SIGNAL(clicked()), this, SLOT(aera_playPauseButtonClicked()));

  aeraStepButton_ = new QToolButton();
  aeraStepButton_->setIcon(QIcon(":/images/play-step.png"));
  connect(aeraStepButton_, SIGNAL(clicked()), this, SLOT(aera_stepFwdButtonClicked()));

  aeraJumpToEndButton_ = new QToolButton();
  aeraJumpToEndButton_->setIcon(QIcon(":/images/jump-end.png"));
  connect(aeraJumpToEndButton_, SIGNAL(clicked()), this, SLOT(aera_jumpToEndButtonClicked()));

  // Set up the slider
  playSlider_ = new QSlider(Qt::Horizontal, this);
  playSlider_->setMaximum(aeraBarSize);
  // Until we implement playSliderValueChanged, disable the slider so the user can't drag it.
  // See https://github.com/IIIM-IS/AERA_Visualizer/issues/3 .
  playSlider_->setEnabled(false);
  //connect(playSlider_, SIGNAL(valueChanged(int)), this, SLOT(playSliderValueChanged(int)));

  // Set up AERA's progress bar
  aeraBar_ = new QProgressBar(this);
  aeraBar_->setMaximum(2000);         // Big number for fine graduations
  aeraBar_->setTextVisible(false);
  
  // Set up labels
  playTimeLabel_ = new ClickableLabel("Visualizer 000s:000ms:000us", this);
  playTimeLabel_->setFont(QFont("Courier", 10));
  connect(playTimeLabel_, SIGNAL(clicked()), this, SLOT(timeLabelClicked()));

  aeraTimeLabel_ = new ClickableLabel("      AERA 000s:000ms:000us", this);
  aeraTimeLabel_->setFont(QFont("Courier", 10));
  connect(aeraTimeLabel_, SIGNAL(clicked()), this, SLOT(timeLabelClicked()));

  // Make a grid of buttons on the left
  QGridLayout* buttonLayout = new QGridLayout();
  buttonLayout->addWidget(jumpToStartButton_, 0, 0);
  buttonLayout->addWidget(stepBackButton_, 0, 1);
  buttonLayout->addWidget(playPauseButton_, 0, 2);
  buttonLayout->addWidget(stepFwdButton_, 0, 3);
  buttonLayout->addWidget(jumpToEndButton_, 0, 4);

  buttonLayout->addWidget(aeraPlayPauseButton_, 1, 2);
  buttonLayout->addWidget(aeraStepButton_, 1, 3);
  buttonLayout->addWidget(aeraJumpToEndButton_, 1, 4);

  // Put the slider and bar in the middle
  QVBoxLayout* barLayout = new QVBoxLayout();
  barLayout->addWidget(playSlider_);
  barLayout->addWidget(aeraBar_);

  // Put the labels on the right
  QVBoxLayout* labelLayout = new QVBoxLayout();
  labelLayout->addWidget(playTimeLabel_);
  labelLayout->addWidget(aeraTimeLabel_);

  // Make a toolbar
  playerControls_ = new QToolBar(this);

  stepTogetherCheckbox_ = new AeraCheckbox("Step timeline with AERA", "StepTogether", mainWindow_, Qt::Checked);
  playerControls_->addWidget(stepTogetherCheckbox_);
  // TO DO: Add in a spinbox to control step length

  // Put it all together
  QHBoxLayout* playerLayout = new QHBoxLayout();
  playerLayout->addLayout(buttonLayout);
  playerLayout->addLayout(barLayout);
  playerLayout->addLayout(labelLayout);

  QVBoxLayout* playerWithToolbarLayout = new QVBoxLayout();
  playerWithToolbarLayout->addLayout(playerLayout);
  playerWithToolbarLayout->addWidget(playerControls_);  

  // Put everything in a container
  QWidget* container = new QWidget();
  container->setObjectName("player_container");
  container->setLayout(playerWithToolbarLayout);
  setWidget(container);
}

void PlayerView::startPlay()
{
  // Update the state
  playPauseButton_->setIcon(pauseIcon_);
  isPlaying_ = true;

  // Start the timer
  if (playTimerId_ == 0)
    playTimerId_ = startTimer(AeraVisualizer_playTimerTick.count());
}

void PlayerView::stopPlay()
{
  // Update the state
  playPauseButton_->setIcon(playIcon_);
  isPlaying_ = false;

  // Stop the timer
  if (playTimerId_ != 0) {
    killTimer(playTimerId_);
    playTimerId_ = 0;
  }
}

void PlayerView::vis_playPauseButtonClicked()
{
  if (isPlaying_)
    stopPlay();
  else
    startPlay();
}

void PlayerView::vis_stepFwdButtonClicked()
{
  playTime_ = mainWindow_->vis_stepFwd();
  setSliderToPlayTime();
  updateLabels();
}

void PlayerView::vis_stepBackButtonClicked()
{
  playTime_ = mainWindow_->vis_stepBack();
  setSliderToPlayTime();
  updateLabels();
}

void PlayerView::vis_jumpToStartButtonClicked()
{
  playTime_ = mainWindow_->vis_jumpToStart();
  setSliderToPlayTime();
  updateLabels();
}

void PlayerView::vis_jumpToEndButtonClicked()
{
  playTime_ = mainWindow_->vis_jumpToEnd();
  setSliderToPlayTime();
  updateLabels();
}

void PlayerView::aera_playPauseButtonClicked() {
  // AERA should run live. This will be like the jump to end button
  // but it will run open-ended, not just to the end of settings.run_time
}

void PlayerView::aera_stepFwdButtonClicked() {
  aeraTime_ = mainWindow_->aera_stepFwd();

  // Update the display
  updateAERABar();
  updateLabels(); // Update the AERA label

  // If desired, step the visualizer forward, too
  QSettings settings;
  if (settings.value("StepTogether", Qt::Checked).toBool()) {
    playTime_ = mainWindow_->vis_jumpToEnd();
    setSliderToPlayTime();
    updateLabels(); // Update the Visualizer label
  }

  // If we've run to the end turn off buttons and update the status message
  if (aeraTime_ >= timeReference_ + runTime_) {
    aeraStepButton_->setEnabled(false);
    aeraJumpToEndButton_->setEnabled(false);
    mainWindow_->setAERAstatus("AERA run finished", 0);
  }
}

void PlayerView::aera_jumpToEndButtonClicked() {
  aeraTime_ = mainWindow_->aera_jumpToEnd();

  // Update the display
  updateAERABar();
  updateLabels(); // Update the AERA label

  // If desired, step the visualizer forward, too
  QSettings settings;
  if (settings.value("StepTogether", Qt::Checked).toBool()) {
    playTime_ = mainWindow_->vis_jumpToEnd();
    setSliderToPlayTime();
    updateLabels(); // Update the Visualizer label
  }

  // Turn off buttons and indicate that AERA's run to the end
  aeraStepButton_->setEnabled(false);
  aeraJumpToEndButton_->setEnabled(false);
  mainWindow_->setAERAstatus("AERA run finished", 0);
}

void PlayerView::playSliderValueChanged(int value)
{
  // TODO: Implement to check if the user moved the slider,
  // stopPlay, update the play time.
}

void PlayerView::timeLabelClicked()
{
  showRelativeTime_ = !showRelativeTime_;
  setPlayTime(playTime_);
}

void PlayerView::updateLabels() {
  // Update the play time label
  uint64 total_us;
  if (showRelativeTime_)
    total_us = duration_cast<microseconds>(playTime_ - timeReference_).count();
  else
    total_us = duration_cast<microseconds>(playTime_.time_since_epoch()).count();
  uint64 us = total_us % 1000;
  uint64 ms = total_us / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  char buffer[100];
  if (showRelativeTime_)
    sprintf(buffer, "Visualizer %03ds:%03dms:%03dus", (int)s, (int)ms, (int)us);
  else {
    // Get the UTC time.
    time_t gmtTime = s;
    struct tm* t = gmtime(&gmtTime);
    sprintf(buffer, "Visualizer %04d-%02d-%02d   UTC\n           %02d:%02d:%02d:%03d:%03d",
      t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
      t->tm_hour, t->tm_min, t->tm_sec, (int)ms, (int)us);
  }
  playTimeLabel_->setText(buffer);

  // Update the AERA time label
  // TO DO: On the first step, the AERA time label briefly flashes a large negative number
  //        before correcting to the right value. This is because aeraTime_ starts off at 0.
  //        This should be fixed to make sure that aeraTime_ starts off at timeReference_
  if (showRelativeTime_)
    total_us = duration_cast<microseconds>(aeraTime_ - timeReference_).count();
  else
    total_us = duration_cast<microseconds>(aeraTime_.time_since_epoch()).count();
  us = total_us % 1000;
  ms = total_us / 1000;
  s = ms / 1000;
  ms = ms % 1000;
  
  memset(buffer, 0x0, 100);
  if (showRelativeTime_)
    sprintf(buffer, "      AERA %03ds:%03dms:%03dus", (int)s, (int)ms, (int)us);
  else {
    // Get the UTC time.
    time_t gmtTime = s;
    struct tm* t = gmtime(&gmtTime);
    sprintf(buffer, "      AERA %04d-%02d-%02d   UTC\n           %02d:%02d:%02d:%03d:%03d",
      t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
      t->tm_hour, t->tm_min, t->tm_sec, (int)ms, (int)us);
  }
  aeraTimeLabel_->setText(buffer);
}

void PlayerView::setSliderToPlayTime()
{
  // If there are no events, fix the slider to the start
  if (mainWindow_->getNumberOfEvents() == 0) {
    playSlider_->setValue(0);
    return;
  }

  // Figure out how far along we are
  auto maximumEventTime = mainWindow_->getTimeOfLastEvent();
  auto endTime = timeReference_ + runTime_;
  
  // If we've run past AERA's end time, switch to the time of the last event
  if (maximumEventTime > endTime)
    endTime = maximumEventTime;
    
  int value = playSlider_->maximum() *
    ((double)duration_cast<microseconds>(playTime_ - timeReference_).count() /
      duration_cast<microseconds>(endTime - timeReference_).count());
  playSlider_->setValue(value);
}


void PlayerView::updateAERABar() {
  aeraBar_->setMaximum(aeraBarSize); // Reset this in case we were in setAERARunning before

  // Figure out how far along we are
  auto endTime = timeReference_ + runTime_;
  int value = aeraBar_->maximum() *
    ((double)duration_cast<microseconds>(aeraTime_ - timeReference_).count() /
      duration_cast<microseconds>(endTime - timeReference_).count());
  aeraBar_->setValue(value);
}

void PlayerView::setUIEnabled(bool enabled) {
  jumpToStartButton_->setEnabled(enabled);
  stepBackButton_->setEnabled(enabled);
  playPauseButton_->setEnabled(enabled);
  stepFwdButton_->setEnabled(enabled);
  jumpToEndButton_->setEnabled(enabled);

  aeraPlayPauseButton_->setEnabled(false);    // Needs AERA live operation
  aeraStepButton_->setEnabled(enabled);
  aeraJumpToEndButton_->setEnabled(enabled);

  playTimeLabel_->setEnabled(enabled);
  aeraTimeLabel_->setEnabled(enabled);

  stepTogetherCheckbox_->setEnabled(enabled);
}

void PlayerView::timerEvent(QTimerEvent* event)
{
  // TODO: Make sure we don't re-enter.
  // TO DO: have a separate aeraTimerId_ for live AERA operation
  if (event->timerId() != playTimerId_)
    // This timer event is not for us.
    return;

  // Anyone who should respond to timer events goes here
  mainWindow_->timerTick();

  // Keep the UI updated
  setSliderToPlayTime();
}

void PlayerView::setAERARunning() {
  aeraBar_->setMaximum(0);
  aeraBar_->setValue(0);
}

ClickableLabel::ClickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
  : QLabel(text, parent) {}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* event) { emit clicked(); }

}
