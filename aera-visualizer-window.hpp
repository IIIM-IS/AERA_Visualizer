#ifndef AERA_VISUALIZER_WINDOW_HPP
#define AERA_VISUALIZER_WINDOW_HPP

#include <regex>
#include "graphics-items/aera-graphics-item.hpp"
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
   * Create an AeraVisulizerWindow.
   */
  AeraVisulizerWindow(ReplicodeObjects& replicodeObjects, const std::string& debugStreamFilePath);

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

protected:
  bool haveMoreEvents() override { return iNextEvent_ < events_.size(); }

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
   * \param minimumTime if the time of previous event is less than minimumTime, don't
   * decrement iNextEvent_ and don't undo, and return Utils_MaxTime.
   * \return The time of the previous event. If there is no previous event, then
   * return Utils_MaxTime.
   */
  core::Timestamp unstepEvent(core::Timestamp minimumTime) override;

  ExplanationLogWindow* explanationLogWindow_;

private slots:
  void zoomIn();
  void zoomOut();
  void zoomHome();

private:
  void createActions();
  void createMenus();
  void createToolbars();
  /**
   * Scan the debugStreamFilePath and add to events_.
   * \param debugStreamFilePath The file path of the debug stream output,
   * typically ending in "debug_out.txt".
   */
  void addEvents(const std::string& debugStreamFilePath);

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
   * Get the scene AeraGraphicsItem whose getAeraEvent() has the given object, and set its pen.
   * If the object or item is not found, do nothing.
   * \param object The Code* object to search for.
   * \param pen The pen (for the border).
   */
  void setAeraGraphicsItemPen(r_code::Code* object, const QPen& pen);

  /**
   * Get the scene AeraGraphicsItem whose getAeraEvent() has the given object, and set its pen to
   * its getBorderNoHighlightPen().
   * If the object or item is not found, do nothing.
   * \param object The Code* object to search for.
   */
  void resetAeraGraphicsItemPen(r_code::Code* object);

  AeraVisualizerScene* modelsScene_;
  AeraVisualizerScene* mainScene_;
  AeraVisualizerScene* selectedScene_;

  QAction* exitAction_;
  QAction* zoomInAction_;
  QAction* zoomOutAction_;
  QAction* zoomHomeAction_;

  QCheckBox* essenceFactsCheckBox_;
  QCheckBox* instantiatedCompositeStatesCheckBox_;

  size_t iNextEvent_;
  ReplicodeObjects& replicodeObjects_;
  QPen itemBorderHighlightPen_;
  r_code::Code* hoverHighlightObject_;
  QString hoverPreviousUrl_;
};

}

#endif
