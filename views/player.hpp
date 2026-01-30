//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2026 Jeff Thompson
//_/_/ Copyright (c) 2018-2026 Kristinn R. Thorisson
//_/_/ Copyright (c) 2023-2026 Chloe Schaff
//_/_/ Copyright (c) 2018-2026 Icelandic Institute for Intelligent Machines
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

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>
#include <QDockWidget>
#include <QPushButton>
#include <QToolButton>
#include <QToolBar>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include "aera-checkbox.h"
#include "../submodules/AERA/r_code/utils.h"
#include "../replicode-objects.hpp"
#include "../aera-visualizer-window.hpp"

namespace aera_visualizer {

class AeraVisualizerWindow;

// https://wiki.qt.io/Clickable_QLabel
class ClickableLabel : public QLabel {
  Q_OBJECT

public:
  explicit ClickableLabel(
    const QString& text, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  ~ClickableLabel();

signals:
  void clicked();

protected:
  void mousePressEvent(QMouseEvent* event) override;
};

/**
 * PlayerView extends QDockWidget to provide a re-arrangeable way to control
 * both AERA's and the Visualizer's timelines
 */
class PlayerView : public QDockWidget
{
  Q_OBJECT

public:
  /**
  * Create a PlayerView object.
  * \param mainWindow All button responses will call responders in mainWindow
  */
  PlayerView(AeraVisualizerWindow* mainWindow);

  // Used to control whether we're currently playing (starts/kills timers)
  void startPlay();
  void stopPlay();

  // Return current play time
  core::Timestamp getPlayTime() {
    return playTime_;
  }

  // Return current AERA time
  core::Timestamp getAERATime() {
    return aeraTime_;
  }

  // Set playTime_ and update the playTimeLabel_.
  void setPlayTime(core::Timestamp time) {
    playTime_ = time;
    updateLabels();
  }

  // Set runTime_ and update the aeraBar_
  void setRunTime(std::chrono::milliseconds run_time) {
    runTime_ = run_time;
    updateAERABar();
  }

  // Set the playSlider_ position based on playTime_.
  void setSliderToPlayTime();

  // Update AERA's progress bar to the current aeraTime_
  void updateAERABar();

  // Setting the time reference from the main window's replicodeObjects_
  void setTimeReference(core::Timestamp timeReference) {
    timeReference_ = timeReference;
  }

  // Show a busy bar while AERA runs. Reset with updateAERABar()
  void setAERARunning();

  // Used to turn the player controls on or off
  void setUIEnabled(bool enabled);

  // Disables AERA controls because the visualizer is running without an AERA instance
  void indicatePreviousRun();

private slots:
  void vis_playPauseButtonClicked();
  void vis_stepFwdButtonClicked();
  void vis_stepBackButtonClicked();
  void vis_jumpToStartButtonClicked();
  void vis_jumpToEndButtonClicked();
  void aera_playPauseButtonClicked();
  void aera_stepFwdButtonClicked();
  void aera_jumpToEndButtonClicked();
  void playSliderValueChanged(int value);
  void timeLabelClicked();

private:
  void timerEvent(QTimerEvent* event) override;

  void updateLabels();

  AeraVisualizerWindow* mainWindow_;
  bool isPlaying_;
  bool showRelativeTime_;
  int playTimerId_;
  core::Timestamp playTime_;
  core::Timestamp aeraTime_;
  core::Timestamp timeReference_;
  std::chrono::milliseconds runTime_;
  QIcon playIcon_;
  QIcon pauseIcon_;
  QToolButton* jumpToEndButton_;
  QToolButton* jumpToStartButton_;
  QToolButton* playPauseButton_;
  QToolButton* stepBackButton_;
  QToolButton* stepFwdButton_;
  QToolButton* aeraStepButton_;
  QToolButton* aeraPlayPauseButton_;
  QToolButton* aeraJumpToEndButton_;
  QSlider* playSlider_;
  QProgressBar* aeraBar_;
  ClickableLabel* playTimeLabel_;
  ClickableLabel* aeraTimeLabel_;
  QToolBar* playerControls_;
  AeraCheckbox* stepTogetherCheckbox_;

  int aeraBarSize = 2000;   // Big number for fine graduations
};

static const std::chrono::milliseconds AeraVisualizer_playTimerTick(100);

}

#endif
