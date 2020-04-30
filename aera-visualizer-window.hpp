#ifndef AERA_VISUALIZER_WINDOW_HPP
#define AERA_VISUALIZER_WINDOW_HPP

#include <regex>
#include "aera-model-item.hpp"
#include "aera-event.hpp"
#include "aera-visualizer-window-base.hpp"
#include "replicode-objects.hpp"

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

/**
 * AeraVisulizerWindow extends AeraVisulizerWindowBase to present the player
 * control panel and a window for visualizing the processing of AERA models.
 */
class AeraVisulizerWindow : public AeraVisulizerWindowBase
{
  Q_OBJECT

public:
  /**
   * Create an AeraVisulizerWindow.
   */
  AeraVisulizerWindow();

protected:
  bool haveMoreEvents() override { return iNextEvent_ >= events_.size(); }

  /**
   * Perform the event at events_[iNextEvent_] and then increment iNextEvent_.
   * \param maximumTime if the time of next event is greater than maximumTime, don't perform the
   * event, and return Utils_MaxTime.
   * \return The time of the next event. If there is no next event, then
   * return Utils_MaxTime.
   */
  core::Timestamp stepEvent(core::Timestamp maximumTime) override;

  /**
   * Decrement iNextEvent_ and undo the event at events_[iNextEvent_].
   * \return The time of the previous event. If there is no previous event, then
   * return Utils_MaxTime.
   */
  core::Timestamp unstepEvent() override;

private slots:
  void zoomIn();
  void zoomOut();
  void zoomHome();
  void bringToFront();
  void sendToBack();

private:
  void createActions();
  void createMenus();
  void createToolbars();
  /**
   * Scan the consoleOutputFilePath and add to events_.
   * \param consoleOutputFilePath The file path of the console output,
   * typically ending in "Test.out.txt".
   */
  void addEvents(const std::string& consoleOutputFilePath);

  /**
   * Get the time stamp from the decimal strings of seconds, milliseconds and
   * microseconds at matches[1], matches[2] and matches[3], then add
   * replicodeObjects_.getTimeReference().
   * \param matches The smatch object.
   * \return The timestamp.
   */
  core::Timestamp getTimestamp(const smatch& matches);

  AeraVisualizerScene* scene_;

  QAction* exitAction_;
  QAction* toFrontAction_;
  QAction* sendBackAction_;
  QAction* zoomInAction_;
  QAction* zoomOutAction_;
  QAction* zoomHomeAction_;

  QMenu* itemMenu_;

  size_t iNextEvent_;

  ReplicodeObjects replicodeObjects_;
};

}

#endif
