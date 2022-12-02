//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
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

#ifndef AERA_VISUALIZER_WINDOW_BASE_HPP
#define AERA_VISUALIZER_WINDOW_BASE_HPP

#include <vector>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include "submodules/AERA/r_code/utils.h"
#include "replicode-objects.hpp"

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
 * AeraVisualizerWindowBase extends QMainWindow and is a base class for
 * visualizer windows like AeraVisualizerWindow which manages the player
 * control panel of the main window and derived windows.
 */
class AeraVisualizerWindowBase : public QMainWindow
{
  Q_OBJECT

protected:
  /**
   * Create an AeraVisualizerWindowBase and create the player control panel widget. This is 
   * called by the derived class, which should add getPlayerControlPanel() to its window.
   * \param mainWindow The main parent window for this window, or 0 if this is already
   * The main window.
   * \param runtimeOutputFilePath The file path of the runtime output,
   * typically ending in "runtime_out.txt".
   */
  AeraVisualizerWindowBase(AeraVisualizerWindow* mainWindow, ReplicodeObjects& replicodeObjects);

  /**
   * Get the player control panel widget which has a play button, slider bar and time label.
   * The derived class should add this to its window.
   */
  QWidget* getPlayerControlPanel() { return playerControlPanel_;  }

  AeraVisualizerWindow* mainWindow_;
  ReplicodeObjects& replicodeObjects_;

private slots:
  void playPauseButtonClicked();
  void stepButtonClicked();
  void stepBackButtonClicked();
  void playSliderValueChanged(int value);
  void playTimeLabelClicked();

private:
  friend class AeraVisualizerWindow;
  void createPlayerControlPanel();

  QIcon playIcon_;
  QIcon pauseIcon_;
  QToolButton* playPauseButton_;
  QToolButton* stepBackButton_;
  QToolButton* stepButton_;
  QSlider* playSlider_;
  ClickableLabel* playTimeLabel_;

  std::vector<AeraVisualizerWindowBase*> children_;
  QWidget* playerControlPanel_;
};

static const std::chrono::milliseconds AeraVisualizer_playTimerTick(100);

}

#endif
