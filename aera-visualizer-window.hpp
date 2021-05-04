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

#ifndef AERA_VISUALIZER_WINDOW_HPP
#define AERA_VISUALIZER_WINDOW_HPP

#include <regex>
#include "graphics-items/aera-graphics-item.hpp"
#include "aera-event.hpp"
#include "aera-visualizer-window-base.hpp"

#include <vector>
#include <QIcon>

class AeraVisualizerScene;

class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QGraphicsView;

namespace aera_visualizer {

class ExplanationLogWindow;

/**
 * AeraVisulizerWindow extends AeraVisulizerWindowBase to present the player
 * control panel and a window for visualizing the processing of AERA objects.
 */
class AeraVisulizerWindow : public AeraVisulizerWindowBase
{
  Q_OBJECT

public:
  /**
   * Create an AeraVisulizerWindow. After creating the window, call addEvents().
   * \param replicodeObjects The ReplicodeObjects used to find objects.
   */
  AeraVisulizerWindow(ReplicodeObjects& replicodeObjects);

  /**
   * Scan the runtimeOutputFilePath and add to events_. Call this once after creating the window.
   * \param runtimeOutputFilePath The file path of the runtime output,
   * typically ending in "runtime_out.txt".
   * \return True for success, false if canceled.
   */
  bool addEvents(const std::string& runtimeOutputFilePath);

  void setExplanationLogWindow(ExplanationLogWindow* explanationLogWindow)
  {
    explanationLogWindow_ = explanationLogWindow;
  }

  ExplanationLogWindow* getExplanationLogWindow() { return explanationLogWindow_;  }

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

  static const set<int> simulationEventTypes_;

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
   * \return The time of the previous event. If there is no previous event, then
   * return Utils_MaxTime.
   */
  core::Timestamp unstepEvent(core::Timestamp minimumTime);

  ExplanationLogWindow* explanationLogWindow_;

private slots:
  void zoomIn();
  void zoomOut();
  void zoomHome();

private:
  friend class AeraVisulizerWindowBase;
  void createActions();
  void createMenus();
  void createToolbars();

  /**
   * Get the time stamp from the decimal strings of seconds, milliseconds and
   * microseconds at matches[index], matches[index + 1] and matches[index + 2], then add
   * replicodeObjects_.getTimeReference().
   * \param matches The smatch object.
   * \param index (optional) The matches index for the first number. If omitted, use 1.
   * \return The timestamp.
   */
  core::Timestamp getTimestamp(const smatch& matches, int index = 1);

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
  void setPlayTime(core::Timestamp time);

  /**
   * Set the playSlider_ position based on playTime_.
   */
  void setSliderToPlayTime();

  void playPauseButtonClickedImpl();
  void stepButtonClickedImpl();
  void stepBackButtonClickedImpl();
  void playTimeLabelClickedImpl();
  void timerEvent(QTimerEvent* event) override;

  AeraVisualizerScene* modelsScene_;
  AeraVisualizerScene* mainScene_;
  AeraVisualizerScene* selectedScene_;

  QAction* exitAction_;
  QAction* zoomInAction_;
  QAction* zoomOutAction_;
  QAction* zoomHomeAction_;

  QCheckBox* simulationsCheckBox_;
  QCheckBox* nonSimulationsCheckBox_;
  QCheckBox* essenceFactsCheckBox_;
  QCheckBox* instantiatedCompositeStatesCheckBox_;

  std::vector<std::shared_ptr<AeraEvent> > events_;
  size_t iNextEvent_;
  QPen itemBorderHighlightPen_;
  AeraGraphicsItem* hoverHighlightItem_;
  bool hoverHighlightItemWasVisible_;
  QString hoverPreviousUrl_;
  r_code::Code* essencePropertyObject_;

  bool showRelativeTime_;
  core::Timestamp playTime_;
  int playTimerId_;
  bool isPlaying_;
  // The AeraEvent types where stepEvent will create a new AeraGraphicsItem.
  static const set<int> newItemEventTypes_;
};

}

#endif
