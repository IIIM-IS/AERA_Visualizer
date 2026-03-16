//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2026 Jeff Thompson
//_/_/ Copyright (c) 2018-2026 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2026 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
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

#ifndef AERA_VISUALIZER_WINDOW_HPP
#define AERA_VISUALIZER_WINDOW_HPP

#include <regex>
#include "graphics-items/aera-graphics-item.hpp"
#include "aera-event.hpp"
#include "aera-checkbox.h"
#include "views/explanation-log.hpp"
#include "views/semantics.hpp"
#include "views/player.hpp"
#include "views/text-output.hpp"
#include "views/task-environment.hpp"
#include "abagraph.hpp"

#include <vector>
#include <QIcon>
#include <QDockWidget>
#include <QMainWindow>
#include <QSettings>

class AeraVisualizerScene;

class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QGraphicsView;
class QProgressDialog;
class QString;

namespace aera_visualizer {

// Some views require forward declaration
class ExplanationLogView;
class FindDialog;
class PlayerView;
class TextOutputView;
class TaskEnvironmentView;
class AbaSentenceItem;

/**
 * AeraVisualizerWindow extends AeraVisualizerWindowBase to present the player
 * control panel and a window for visualizing the processing of AERA objects.
 */
class AeraVisualizerWindow : public QMainWindow
{
  Q_OBJECT

public:
  /**
   * Create an AeraVisualizerWindow
   */
  AeraVisualizerWindow();

  class AbaSolution {
  public:
    AbaSolution() : parentSolutionId_(0), parentSolutionStep_(0) {}
    AbaSolution(int parentSolutionId, int parentSolutionStep)
      : parentSolutionId_(parentSolutionId), parentSolutionStep_(parentSolutionStep) {
    }

    int parentSolutionId_;
    int parentSolutionStep_;
    // map of step number -> list of events.
    std::map<int, std::vector<std::shared_ptr<AeraEvent> > > abaEvents_;
  };

  /**
   * Scan the runtimeOutputFilePath and add to startupEvents_ and events_. Call this once after creating the window.
   * After showing the window for the first time, you must call addStartupItems().
   * \param runtimeOutputFilePath The file path of the runtime output,
   * typically ending in "runtime_out.txt".
   * \param progress The progress dialog where you can call setLabelText, setMaximum and setValue. You should
   * periodically call QApplication::processEvents(). You can call wasCanceled and quit if true.
   * \return True for success, false if canceled.
   */
  bool addEvents(const std::string& runtimeOutputFilePath, QProgressDialog& progress);

  /**
   * Add the startup items to modelsScene_ for the startupEvents_ added by addEvents().
   */
  void addStartupItems();

  ExplanationLogView* getExplanationLogView() { return explanationLogView_;  }

  void setFindWindow(FindDialog* zoomToWindow)
  {
    findDialog_ = zoomToWindow;
  }
  
  // Modify some code from stepEvent so we can compute the current maximum visible time
  Timestamp getFrameMaxTime() {
    // Check if we've run off the end (or are not initialized)
    if (iNextEvent_ >= events_.size())
      return r_code::Utils_MaxTime;

    // Get the next event and use it to compute the current frame's max time
    AeraEvent* event = events_[iNextEvent_].get();
    auto relativeTime = std::chrono::duration_cast<std::chrono::microseconds>(event->time_ - replicodeObjects_.getTimeReference());
    auto frameStartTime = event->time_ - (relativeTime % replicodeObjects_.getSamplingPeriod());
    Timestamp thisFrameMaxTime = frameStartTime + replicodeObjects_.getSamplingPeriod() - std::chrono::microseconds(1);

    return thisFrameMaxTime;
  }
  

  /**
   * Check if one of the scenes has an AeraGraphicsItem for the object. You can use this to
   * get the item, or just check if it exists, e.g. such that zoomToAeraGraphicsItem will succeed.
   * \param object The Code* object to search for.
   * \param scene (optional) If this returns an item, set *scene to a pointer to the
   * AeraVisualizerScene where it was found. If omitted or NULL, don't set it. 
   * \return The AeraGraphicsItem, or null if not found.
   */
  AeraGraphicsItem* getAeraGraphicsItem(r_code::Code* object, AeraVisualizerScene** scene = 0);

  /**
   * Get the scene AeraGraphicsItem whose getAeraEvent() has the given object, and zoom the
   * scene to it. If the item is not found, do nothing. You can use hasAeraGraphicsItem() first to 
   * make sure this will succeed.
   * \param object The Code* object to search for.
   */
  void zoomToAeraGraphicsItem(r_code::Code* object);

  /**
   * Get the scene AeraGraphicsItem whose getAeraEvent() has the given object, and focus on it.
   * If the item is not found, do nothing. You can use hasAeraGraphicsItem() first to
   * make sure this will succeed.
   * \param object The Code* object to search for.
   */
   void focusOnAeraGraphicsItem(r_code::Code* object);

    /**
  * Get the scene AeraGraphicsItem whose getAeraEvent() has the given object, and center on it.
  * If the item is not found, do nothing. You can use hasAeraGraphicsItem() first to
  * make sure this will succeed.
  * \param object The Code* object to search for.
  */
    void centerOnAeraGraphicsItem(r_code::Code* object);

  /**
   * Handle hover move events to get the HTML link at the position and highlight the linked item
   * until the mouse leaves the link.
   * \param document This calls document->documentLayout()->anchorAt(position) to get the hovered link.
   * \param position The position relative to the text item from the move event.
   */
  void textItemHoverMoveEvent(const QTextDocument* document, QPointF position);

  /**
   * Get the AeraEvent at index i in the events list.
   * \param i The index.
   * \return A pointer to the AeraEvent.
   */
  const AeraEvent* getAeraEvent(size_t i) const { return events_[i].get(); }

  static const std::set<int> simulationEventTypes_;

  AeraVisualizerScene* getMainScene() {
    return mainScene_;
  }

  AeraVisualizerScene* getModelsScene() {
    return semanticsView_->getModelsScene();
  }

  // Step all the way back to the start
  core::Timestamp vis_jumpToStart();

  // Step back once
  core::Timestamp vis_stepBack();

  // Step forwards once
  core::Timestamp vis_stepFwd();

  // Step all the way to the end
  core::Timestamp vis_jumpToEnd();

  // Step AERA forwards
  core::Timestamp aera_stepFwd();

  // Let AERA run all the way to the end
  core::Timestamp aera_jumpToEnd();

  /**
   * Called by a PlayerView in setSliderToPlayTime
   */
  int getNumberOfEvents() {
    return events_.size();
  }

  /**
   * Called by a PlayerView in setSliderToPlayTime
   */
  core::Timestamp getTimeOfLastEvent() {
    return events_.back()->time_;
  }

  /**
   * Called by a PlayerView in setSliderToPlayTime
   */
  void scrollToTime(Timestamp time) {
    // If auto scroll is enabled, ensure the new item is visible
    QSettings settings;
    if (mainScene_ && settings.value("AutoScroll", Qt::Unchecked).toInt() == Qt::Checked) {
      mainScene_->scrollToTimestamp(time);
    }
  }

  /**
  * Called by a PlayerView to tick forwards when playing
  */
  void timerTick();

  // Get AERA's current time
  core::Timestamp getAERATime() {
    return aera_->getCurrentTime();
  }

  // These are used to set status messages for the labels in the status bar
  void setAERAstatus(QString message, bool alert) {
    AERAStatusLabel_->setText(message);

    if (alert)
      AERAStatusLabel_->setStyleSheet(statusStylesheet_red_);
    else
      AERAStatusLabel_->setStyleSheet(statusStylesheet_normal_);
  }

  enum OperatingMode { PAUSED, WORKING, BABBLING, ERR };

  void setOperatingModeStatus(QString message, OperatingMode mode) {
    operatingModeStatusLabel_->setText(message);

    switch (mode) {
      case PAUSED:
        operatingModeStatusLabel_->setStyleSheet(statusStylesheet_normal_);
        break;
      case WORKING:
        operatingModeStatusLabel_->setStyleSheet(statusStylesheet_green_);
        break;
      case BABBLING:
        operatingModeStatusLabel_->setStyleSheet(statusStylesheet_yellow_);
        break;
      case ERR:
        operatingModeStatusLabel_->setStyleSheet(statusStylesheet_red_);
        break;
    }
  }

  void abaSentenceItemClicked(AbaSentenceItem* item);

protected:
  /**
   * Set iNextStepEvent to the index in events_ of the next event that stepEvent will process.
   * \param maximumTime If the time of next event is greater than maximumTime, don't perform the
   * event, and return Utils_MaxTime.
   * \param iNextEventStart The index of the first event to consider, usually iNextEvent_ (but used
   * internally to recursively call this method.
   * \param iNextStepEvent the index in events_ of the next event that stepEvent will process. In most
   * cases, this is iNextEventStart. However, if this method returns Utils_MaxTime, then stepEvent will
   * not process a next event and iNextStepEvent is undefined.
   * \return The time of the next event. If there is no next event, then
   * return Utils_MaxTime.
   */
  Timestamp getINextStepEvent(Timestamp maximumTime, size_t iNextEventStart, size_t& iNextStepEvent);

  /**
   * Perform the event at events_[iNextEvent_] and then increment iNextEvent_.
   * \param maximumTime If the time of next event is greater than maximumTime, don't perform the
   * event, and return Utils_MaxTime.
   * \return The time of the next event. If there is no next event, then
   * return Utils_MaxTime.
   */
  core::Timestamp stepEvent(core::Timestamp maximumTime);

  /**
   * Decrement iNextEvent_ and undo the event at events_[iNextEvent_].
   * \param minimumTime if the time of previous event is less than minimumTime, don't
   * decrement iNextEvent_ and don't undo, and return Utils_MaxTime.
   * \param Set foundGraphicsItem if the graphics item for the step was found.
   * \return The time of the previous event. If there is no previous event, then
   * return Utils_MaxTime.
   */
  core::Timestamp unstepEvent(core::Timestamp minimumTime, bool& foundGraphicsItem);

  ExplanationLogView* explanationLogView_;
  FindDialog* findDialog_;

private slots:
  void loadNewSeed();
  void openOutput();
  void saveOutput();
  void saveMainWindowImage();
  void zoomIn();
  void zoomOut();
  void zoomHome();
  void find();
  void findNext();
  void findPrev();
  void fitAll();

private:
  void createDockWidgets();
  void createActions();
  void createMenus();
  void createToolbars();
  void createStatusBar();

  QToolBar* timelineControls_;
  
  /**
  * Reads new data on the run, updates replicodeObjects_ and settings_ accordingly,
  * and pushes the data to the GUI elements. Can be configured to read directly from
  * AERA's memory during a live run (the default option) or to assume settings_ and
  * replicodeObjects_ have already been filled in by openOutput(). Should be called
  * after openOutput() or after stepping AERA.
  */
  void updateObjectsAndEvents(bool live = true);

  /**
   * Get the time stamp from the decimal strings of seconds, milliseconds and
   * microseconds at matches[index], matches[index + 1] and matches[index + 2], then add
   * replicodeObjects_.getTimeReference().
   * \param matches The smatch object.
   * \param index (optional) The matches index for the first number. If omitted, use 1.
   * \return The timestamp.
   */
  core::Timestamp getTimestamp(const std::smatch& matches, int index = 1);

  // Use these to turn the UI on and off depending on whether anything is currently loaded
  void setUIEnabled(bool enabled);
  
  void closeEvent(QCloseEvent* event) override;


  // Status bar stylesheets
  QString statusStylesheet_normal_ =
    "QLabel {"
    " border-top: 1px solid;"
    "}";

  QString statusStylesheet_green_ =
    "QLabel {"
    " border-top: 1px solid;"
    " background-color: limegreen;"
    " color: white;"
    "}";

  QString statusStylesheet_yellow_ =
    "QLabel {"
    " border-top: 1px solid;"
    " background-color: khaki;"
    "}";

  QString statusStylesheet_red_ =
    "QLabel {"
    " border-top: 1px solid;"
    " background-color: tomato;"
    " color: white;"
    "}";
  

  AERA_interface* aera_;
  ReplicodeObjects replicodeObjects_;
  Settings settings_;

  SemanticsView* semanticsView_;
  PlayerView* playerView_;
  TextOutputView* textOutputView_;
  TaskEnvironmentView* taskEnvironmentView_;

  AeraVisualizerScene* modelsScene_;
  AeraVisualizerScene* mainScene_;
  AeraVisualizerScene* selectedScene_;
  
  QAction* newInstanceAction_;
  QAction* openOutputAction_;
  QAction* saveOutputAction_;

  QAction* saveMainWindowImageAction_;
  QAction* exitAction_;
  QAction* resetAERAInstanceAction_;
  QAction* configureAERAInstanceAction_;
  QAction* zoomInAction_;
  QAction* zoomOutAction_;
  QAction* zoomHomeAction_;
  QAction* findAction_;
  QAction* findNextAction_;
  QAction* findPrevAction_;
  QAction* fitAllAction_;

  static const QString SettingsKeyAutoScroll;
  static const QString SettingsKeySimulationsVisible;
  static const QString SettingsKeyAllSimulationInputsVisible;
  static const QString SettingsKeySingleStepSimulationVisible;
  static const QString SettingsKeyNonSimulationsVisible;
  static const QString SettingsKeyEssenceFactsVisible;
  static const QString SettingsKeyInstantiatedCompositeStatesVisible;
  static const QString SettingsKeyInstantiatedModelsVisible;
  static const QString SettingsKeyPredictedInstantiatedCompositeStatesVisible;
  static const QString SettingsKeyRequirementsVisible;

  AeraCheckbox* simulationsCheckBox_;
  AeraCheckbox* allSimulationInputsCheckBox_;
  AeraCheckbox* singleStepSimulationCheckBox_;
  AeraCheckbox* nonSimulationsCheckBox_;
  AeraCheckbox* essenceFactsCheckBox_;
  AeraCheckbox* instantiatedCompositeStatesCheckBox_;
  AeraCheckbox* instantiatedModelsCheckBox_;
  AeraCheckbox* predictedInstantiatedCompositeStatesCheckBox_;
  AeraCheckbox* requirementsCheckBox_;

  QLabel* AERAStatusLabel_;
  QLabel* operatingModeStatusLabel_;

  std::vector<std::shared_ptr<AeraEvent> > startupEvents_;
  std::vector<std::shared_ptr<AeraEvent> > events_;
  size_t iNextEvent_;
  QPen itemBorderHighlightPen_;
  AeraGraphicsItem* hoverHighlightItem_;
  bool hoverHighlightItemWasVisible_;
  QString hoverPreviousUrl_;
  r_code::Code* essencePropertyObject_;
  QColor phasedOutModelColor_;

  int lastLine_ = 0;      // The farthest we've read into runtime_out.txt
  std::map<int, QString> bindings_;
  // The AeraEvent types where stepEvent will create a new AeraGraphicsItem.
  static const std::set<int> newItemEventTypes_;
  AbaGraph abagraph_;
  // map of solutionId -> AbaSolution.
  std::map<int, AbaSolution> abaSolutions_;
};

}

#endif
