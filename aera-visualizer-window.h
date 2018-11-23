#ifndef AERA_VISUALIZER_WINDOW_H
#define AERA_VISUALIZER_WINDOW_H

#include "aera-model-item.h"
#include "aera-event.h"
#include "aera-visualizer-window-base.h"

#include <vector>
#include <QIcon>

class AeraVisualizerScene;

class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QGraphicsView;

namespace aera_visualizer {

class AeraVisulizerWindow : public AeraVisulizerWindowBase
{
  Q_OBJECT

public:
  AeraVisulizerWindow();

private slots:
  void zoomIn();
  void zoomOut();
  void zoomHome();
  void bringToFront();
  void sendToBack();
  void playPauseButtonClicked();
  void stepButtonClicked();
  void stepBackButtonClicked();
  void playSliderValueChanged(int value);

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  /**
   * Perform the event at events_[iNextEvent_] and then increment iNextEvent_.
   * @param if the time of next event is greater than maximumTime, don't perform the
   * event, and return uint64_MAX.
   * @return The time of the next event. If there is no next event,
   * return uint64_MAX.
   */
  core::uint64 stepEvent(core::uint64 maximumTime);
  /**
   * Decrement iNextEvent_ and undo the event at events_[iNextEvent_].
   * @return The time of the previous event. If there is no previous event, 
   * return uint64_MAX.
   */
  core::uint64 unstepEvent();
  /**
   * Enable the play timer to play events and set the playPauseButton_ icon.
   * If isPlaying_ is already true, do nothing.
   */
  void startPlay();
  /**
   * Disable the play timer, set the playPauseButton_ icon and set isPlaying_ false.
   */
  void stopPlay();
  void createActions();
  void createMenus();
  void createToolbars();

  AeraVisualizerScene* scene_;

  QAction* exitAction_;

  QAction* toFrontAction_;
  QAction* sendBackAction_;

  QAction* zoomInAction_;
  QAction* zoomOutAction_;
  QAction* zoomHomeAction_;

  QMenu* itemMenu_;
};

}

#endif
