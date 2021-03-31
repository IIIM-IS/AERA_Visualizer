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

#ifndef AERA_VISUALIZER_WINDOW_BASE_HPP
#define AERA_VISUALIZER_WINDOW_BASE_HPP

#include <vector>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include "aera-event.hpp"
#include "submodules/replicode/r_code/utils.h"
#include "replicode-objects.hpp"

namespace aera_visualizer {

class AeraVisulizerWindow;

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
  void mousePressEvent(QMouseEvent* event);
};

/**
 * AeraVisulizerWindowBase extends QMainWindow and is a base class for
 * visualizer windows like AeraVisulizerWindow which manages the player
 * control panel of the main window and derived windows.
 */
class AeraVisulizerWindowBase : public QMainWindow
{
  Q_OBJECT

protected:
  /**
   * Create an AeraVisulizerWindowBase and create the player control panel widget. This is 
   * called by the derived class, which should add getPlayerControlPanel() to its window.
   * \param mainWindow The main parent window for this window, or 0 if this is already
   * The main window.
   * \param runtimeOutputFilePath The file path of the runtime output,
   * typically ending in "runtime_out.txt".
   */
  AeraVisulizerWindowBase(AeraVisulizerWindow* mainWindow, ReplicodeObjects& replicodeObjects);

  /**
   * Get the player control panel widget which has a play button, slider bar and time label.
   * The derived class should add this to its window.
   */
  QWidget* getPlayerControlPanel() { return playerControlPanel_;  }

  virtual bool haveMoreEvents() = 0;

  /**
   * Perform the event at events_[iNextEvent_] and then increment iNextEvent_.
   * \param maximumTime if the time of next event is greater than maximumTime, don't perform the
   * event, and return Utils_MaxTime.
   * \return The time of the next event. If there is no next event, then
   * return Utils_MaxTime.
   */
  virtual core::Timestamp stepEvent(core::Timestamp maximumTime) = 0;

  /**
   * Decrement iNextEvent_ and undo the event at events_[iNextEvent_].
   * \param minimumTime if the time of previous event is less than minimumTime, don't 
   * decrement iNextEvent_ and don't undo, and return Utils_MaxTime.
   * \return The time of the previous event. If there is no previous event, then
   * return Utils_MaxTime.
   */
  virtual core::Timestamp unstepEvent(core::Timestamp minimumTime) = 0;

  AeraVisulizerWindow* mainWindow_;
  ReplicodeObjects& replicodeObjects_;
  // Debug: This should be in the derived class.
  std::vector<std::shared_ptr<AeraEvent> > events_;
  set<int> simulationEventTypes_;

private slots:
  void playPauseButtonClicked();
  void stepButtonClicked();
  void stepBackButtonClicked();
  void playSliderValueChanged(int value);
  void playTimeLabelClicked();

private:
  friend class AeraVisulizerWindow;
  void createPlayerControlPanel();

  QIcon playIcon_;
  QIcon pauseIcon_;
  QToolButton* playPauseButton_;
  QToolButton* stepBackButton_;
  QToolButton* stepButton_;
  QSlider* playSlider_;
  ClickableLabel* playTimeLabel_;

  std::vector<AeraVisulizerWindowBase*> children_;
  QWidget* playerControlPanel_;
};

static const std::chrono::milliseconds AeraVisulizer_playTimerTick(100);

}

#endif
