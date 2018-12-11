#include "aera-visualizer-window-base.h"

#include <QtWidgets>

using namespace std;
using namespace core;

namespace aera_visualizer {

AeraVisulizerWindowBase::AeraVisulizerWindowBase(AeraVisulizerWindowBase* parent)
: QMainWindow(parent),
  parent_(parent),
  iNextEvent_(0),
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
//  connect(playPauseButton_, SIGNAL(clicked()), this, SLOT(playPauseButtonClicked()));
  playPauseButton_->setIcon(playIcon_);
  playerLayout->addWidget(playPauseButton_);

  stepBackButton_ = new QToolButton(this);
  stepBackButton_->setIcon(QIcon(":/images/play-step-back.png"));
//  connect(stepBackButton_, SIGNAL(clicked()), this, SLOT(stepBackButtonClicked()));
  playerLayout->addWidget(stepBackButton_);
  playSlider_ = new QSlider(Qt::Horizontal, this);
  playSlider_->setMaximum(2000);
//  connect(playSlider_, SIGNAL(valueChanged(int)), this, SLOT(playSliderValueChanged(int)));
  playerLayout->addWidget(playSlider_);
  stepButton_ = new QToolButton(this);
  stepButton_->setIcon(QIcon(":/images/play-step.png"));
//  connect(stepButton_, SIGNAL(clicked()), this, SLOT(stepButtonClicked()));
  playerLayout->addWidget(stepButton_);

  playTimeLabel_ = new QLabel("000s:000ms:000us", this);
  playTimeLabel_->setFont(QFont("Courier", 10));
  playerLayout->addWidget(playTimeLabel_);

  playerControlPanel_ = new QWidget();
  playerControlPanel_->setLayout(playerLayout);
}

void AeraVisulizerWindowBase::setPlayTime(uint64 time)
{
  playTime_ = time;

  uint64 us = time % 1000;
  uint64 ms = time / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  char buffer[30];
  sprintf(buffer, "%03us:%03ums:%03uus", (uint)s, (uint)ms, (uint)us);
  playTimeLabel_->setText(buffer);

  if (children_.size() > 0)
    children_[0]->playTimeLabel_->setText(buffer); // debug
}

void AeraVisulizerWindowBase::setSliderToPlayTime()
{
  if (events_.size() == 0) {
    playSlider_->setValue(0);
    return;
  }

  auto maximumEventTime = events_.back()->time_;
  auto debugmax = playSlider_->maximum();
  int value = playSlider_->maximum() * ((double)playTime_ / maximumEventTime);
  playSlider_->setValue(value);

  if (children_.size() > 0)
    children_[0]->playSlider_->setValue(value); // debug
}

}
