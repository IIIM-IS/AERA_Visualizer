#ifndef AERA_VISUALIZER_WINDOW_BASE_H
#define AERA_VISUALIZER_WINDOW_BASE_H

#include <vector>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
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

  virtual bool haveMoreEvents() = 0;

  /**
   * Perform the event at events_[iNextEvent_] and then increment iNextEvent_.
   * @param if the time of next event is greater than maximumTime, don't perform the
   * event, and return uint64_MAX.
   * @return The time of the next event. If there is no next event,
   * return uint64_MAX.
   */
  virtual core::uint64 stepEvent(core::uint64 maximumTime) = 0;

  /**
   * Decrement iNextEvent_ and undo the event at events_[iNextEvent_].
   * @return The time of the previous event. If there is no previous event,
   * return uint64_MAX.
   */
  virtual core::uint64 unstepEvent() = 0;

  void timerEvent(QTimerEvent* event) override;

  // Debug: This should be in the derived class.
  std::vector<std::shared_ptr<AeraEvent> > events_;

private slots:
  void playPauseButtonClicked();
  void stepButtonClicked();
  void stepBackButtonClicked();
  void playSliderValueChanged(int value);

private:
  void createPlayerControlPanel();

  /**
   * Enable the play timer to play events and set the playPauseButton_ icon.
   * If isPlaying_ is already true, do nothing.
   */
  void startPlay();

  /**
   * Disable the play timer, set the playPauseButton_ icon and set isPlaying_ false.
   */
  void stopPlay();

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

  AeraVisulizerWindowBase* parent_;
  std::vector<AeraVisulizerWindowBase*> children_;
  QWidget* playerControlPanel_;

  // These are only used in the main window.
  core::uint64 playTime_;
  int playTimerId_;
  bool isPlaying_;
};

static const std::chrono::milliseconds AeraVisulizer_playTimerTick(100);

}

#endif
