#include <ctime>
#include "aera-visualizer-window-base.hpp"

#include <QtWidgets>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;

namespace aera_visualizer {

AeraVisulizerWindowBase::AeraVisulizerWindowBase(AeraVisulizerWindowBase* parent)
: QMainWindow(parent),
  parent_(parent),
  timeReference_(seconds(0)),
  playTime_(seconds(0)),
  showRelativeTime_(true),
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

  playTimeLabel_ = new ClickableLabel("000s:000ms:000us", this);
  playTimeLabel_->setFont(QFont("Courier", 10));
  connect(playTimeLabel_, SIGNAL(clicked()), this, SLOT(playTimeLabelClicked()));
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

void AeraVisulizerWindowBase::setPlayTime(Timestamp time)
{
  if (parent_) {
    // Only do this from the main window.
    parent_->setPlayTime(time);
    return;
  }

  playTime_ = time;

  uint64 total_us;
  if (showRelativeTime_)
    total_us = duration_cast<microseconds>(time - timeReference_).count();
  else
    total_us = duration_cast<microseconds>(time.time_since_epoch()).count();
  uint64 us = total_us % 1000;
  uint64 ms = total_us / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  char buffer[100];
  if (showRelativeTime_)
    sprintf(buffer, "%03ds:%03dms:%03dus", (int)s, (int)ms, (int)us);
  else {
    // Get the UTC time.
    time_t gmtTime = s;
    struct tm* t = gmtime(&gmtTime);
    sprintf(buffer, "%04d-%02d-%02d   UTC\n%02d:%02d:%02d:%03d:%03d", 
      t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, 
      t->tm_hour, t->tm_min, t->tm_sec, (int)ms, (int)us);
  }
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
  int value = playSlider_->maximum() * ((double)duration_cast<microseconds>(playTime_ - timeReference_).count() / 
                                                duration_cast<microseconds>(maximumEventTime - timeReference_).count());
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
  auto newTime = stepEvent(Utils_MaxTime);
  // Debug: How to step the children also?
  if (newTime != Utils_MaxTime) {
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
  auto newTime = max(unstepEvent(), timeReference_);
  // Debug: How to step the children also?
  if (newTime != Utils_MaxTime) {
    setPlayTime(newTime);
    setSliderToPlayTime();
  }
}

void AeraVisulizerWindowBase::playSliderValueChanged(int value)
{
  // TODO: Implement to check if the user moved the slider,
  // stopPlay, update the play time.
}

void AeraVisulizerWindowBase::playTimeLabelClicked()
{
  showRelativeTime_ = !showRelativeTime_;
  setPlayTime(playTime_);
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

  if (haveMoreEvents()) {
    // We have played all events.
    playTime = maximumEventTime;
    stopPlay();
  }

  setPlayTime(playTime);
  setSliderToPlayTime();
}

ClickableLabel::ClickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
  : QLabel(text, parent) {

}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
  emit clicked();
}

}
