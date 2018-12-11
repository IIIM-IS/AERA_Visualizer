#ifndef AERA_VISUALIZER_WINDOW_BASE_H
#define AERA_VISUALIZER_WINDOW_BASE_H

#include <vector>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
// TODO: Include this directly from the Replicode source.
#include "types.h"
#include "aera-event.h"

namespace aera_visualizer {

/**
 * AeraVisulizerWindowBase extends QMainWindow and is a base class for
 * visualizer windows like AeraVisulizerWindow which manages the player
 * control panel of the main window and derived windows.
 */
class AeraVisulizerWindowBase : public QMainWindow
{
  Q_OBJECT

protected:
  static const core::uint64 uint64_MAX = 0xFFFFFFFFFFFFFFFFull;

  /**
   * Create an AeraVisulizerWindowBase and create the player control panel widget. This is 
   * called by the derived class, which should add getPlayerControlPanel() to its window.
   * @param parent The main parent window for this window, or 0 if this is already
   * The main window.
   */
  AeraVisulizerWindowBase(AeraVisulizerWindowBase* parent);

  /**
   * Get the player control panel widget which has a play button, slider bar and time label.
   * The derived class should add this to its window.
   */
  QWidget* getPlayerControlPanel() { return playerControlPanel_;  }

  /**
   * Set playTime_ and update the playTimeLabel_.
   */
  void setPlayTime(core::uint64 time);

  /**
   * Set the playSlider_ position based on playTime_.
   */
  void setSliderToPlayTime();

  QIcon playIcon_;
  QIcon pauseIcon_;
  QToolButton* playPauseButton_;
  QToolButton* stepBackButton_;
  QToolButton* stepButton_;
  QSlider* playSlider_;
  QLabel* playTimeLabel_;

  size_t iNextEvent_;
  core::uint64 playTime_;
  int playTimerId_;
  bool isPlaying_;

  static const core::uint64 playTimerMicroseconds_ = 100000;
  std::vector<AeraVisulizerWindowBase*> children_;
  std::vector<std::shared_ptr<AeraEvent> > events_;

private:
  void createPlayerControlPanel();

  AeraVisulizerWindowBase* parent_;
  QWidget* playerControlPanel_;
};

}

#endif
