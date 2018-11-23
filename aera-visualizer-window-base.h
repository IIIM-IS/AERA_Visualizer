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

class AeraVisulizerWindowBase : public QMainWindow
{
  Q_OBJECT

protected:
  static const core::uint64 uint64_MAX = 0xFFFFFFFFFFFFFFFFull;

  /**
   * Create a new AeraVisulizerWindowBase. This is called by the derived class.
   * @param parent The main parent window for this window, or 0 if this is already
   * The main window.
   */
  AeraVisulizerWindowBase(AeraVisulizerWindowBase* parent);

  /**
   * Create and return the player widget which has a play button, slider bar and time label.
   * You should add this to your window.
   */
  QWidget* createPlayerWidget();

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
  AeraVisulizerWindowBase* parent_;
};

}

#endif
