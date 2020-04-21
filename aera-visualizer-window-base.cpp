#include "aera-visualizer-window-base.h"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;

namespace aera_visualizer {

AeraVisulizerWindowBase::AeraVisulizerWindowBase(AeraVisulizerWindowBase* parent)
: QMainWindow(parent),
  parent_(parent),
  playTime_(0),
  playTimerId_(0),
  isPlaying_(false)
{
  createPlayerControlPanel();

  if (parent_)
    parent_->children_.push_back(this);
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

  playTimeLabel_ = new QLabel("000s:000ms:000us", this);
  playTimeLabel_->setFont(QFont("Courier", 10));
  playerLayout->addWidget(playTimeLabel_);

  playerControlPanel_ = new QWidget();
  playerControlPanel_->setLayout(playerLayout);
}

void AeraVisulizerWindowBase::startPlay()
{
  if (parent_) {
    // Only do this from the main window.
    parent_->startPlay();
    return;
  }

  if (isPlaying_)
    // Already playing.
    return;

  playPauseButton_->setIcon(pauseIcon_);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playPauseButton_->setIcon(pauseIcon_);
  isPlaying_ = true;
  if (playTimerId_ == 0)
    playTimerId_ = startTimer(AeraVisulizer_playTimerTick.count());
}

void AeraVisulizerWindowBase::stopPlay()
{
  if (parent_) {
    // Only do this from the main window.
    parent_->stopPlay();
    return;
  }

  if (playTimerId_ != 0) {
    killTimer(playTimerId_);
    playTimerId_ = 0;
  }

  playPauseButton_->setIcon(playIcon_);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playPauseButton_->setIcon(playIcon_);
  isPlaying_ = false;
}

void AeraVisulizerWindowBase::setPlayTime(uint64 time)
{
  if (parent_) {
    // Only do this from the main window.
    parent_->setPlayTime(time);
    return;
  }

  playTime_ = time;

  uint64 us = time % 1000;
  uint64 ms = time / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  char buffer[30];
  sprintf(buffer, "%03us:%03ums:%03uus", (uint)s, (uint)ms, (uint)us);
  playTimeLabel_->setText(buffer);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playTimeLabel_->setText(buffer);
}

void AeraVisulizerWindowBase::setSliderToPlayTime()
{
  if (parent_) {
    // Only do this from the main window.
    parent_->setSliderToPlayTime();
    return;
  }

  if (events_.size() == 0) {
    playSlider_->setValue(0);
    for (size_t i = 0; i < children_.size(); ++i)
      children_[i]->playSlider_->setValue(0);
    return;
  }

  auto maximumEventTime = events_.back()->time_;
  auto debugmax = playSlider_->maximum();
  int value = playSlider_->maximum() * ((double)playTime_ / maximumEventTime);
  playSlider_->setValue(value);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->playSlider_->setValue(value);
}

void AeraVisulizerWindowBase::playPauseButtonClicked()
{
  if (parent_) {
    // Only do this from the main window.
    parent_->playPauseButtonClicked();
    return;
  }

  if (isPlaying_)
    stopPlay();
  else
    startPlay();
}

void AeraVisulizerWindowBase::stepButtonClicked()
{
  if (parent_) {
    // Only do this from the main window.
    parent_->stepButtonClicked();
    return;
  }

  stopPlay();
  uint64 newTime = stepEvent(uint64_MAX);
  // Debug: How to step the children also?
  if (newTime != uint64_MAX) {
    setPlayTime(newTime);
    setSliderToPlayTime();
  }
}

void AeraVisulizerWindowBase::stepBackButtonClicked()
{
  if (parent_) {
    // Only do this from the main window.
    parent_->stepBackButtonClicked();
    return;
  }

  stopPlay();
  uint64 newTime = unstepEvent();
  // Debug: How to step the children also?
  if (newTime != uint64_MAX) {
    setPlayTime(newTime);
    setSliderToPlayTime();
  }
}

void AeraVisulizerWindowBase::playSliderValueChanged(int value)
{
  // TODO: Implement to check if the user moved the slider,
  // stopPlay, update the play time.
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
  uint64 playTime = playTime_ + duration_cast<microseconds>(AeraVisulizer_playTimerTick).count();

  // Step events while events_[iNextEvent_] is less than or equal to the playTime.
  // Debug: How to step the children also?
  while (stepEvent(playTime) != uint64_MAX);

  if (haveMoreEvents()) {
    // We have played all events.
    playTime = maximumEventTime;
    stopPlay();
  }

  setPlayTime(playTime);
  setSliderToPlayTime();
}

}
