//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
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

#ifndef AERA_VISUALIZER_SCENE_HPP
#define AERA_VISUALIZER_SCENE_HPP

#include <map>
#include "../aera-event.hpp"
#include "../replicode-objects.hpp"

#include <QGraphicsScene>

class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QColor;

namespace aera_visualizer {

class AeraGraphicsItem;
class AeraVisulizerWindow;
class ExplanationLogWindow;

class AeraVisualizerScene : public QGraphicsScene
{
public:
  typedef std::function<void()> OnSceneSelected;

  explicit AeraVisualizerScene(
    ReplicodeObjects& replicodeObjects, AeraVisulizerWindow* parent, bool isMainScene,
    const OnSceneSelected& onSceneSelected);

  AeraVisulizerWindow* getParent() { return parent_; }

  void zoomToItem(QGraphicsItem* item);
  void focusOnItem(QGraphicsItem* item);
  void centerOnItem(QGraphicsItem* item);
  void scrollToTimestamp(core::Timestamp timestamp);

  /**
   * Get the X position on the timeline for the given timestamp.
   * \param timestamp The timestamp.
   * \return The X position.
   */
  qreal getTimelineX(core::Timestamp timestamp)
  {
    double microsecondsPerPixel = (double)replicodeObjects_.getSamplingPeriod().count() / frameWidth_;
    auto relativeTime = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - replicodeObjects_.getTimeReference());
    return relativeTime.count() / microsecondsPerPixel;
  }

  /**
   * This is called by the QGraphicsView instance when the view is moved.
   */
  void onViewMoved();

  /**
   * Set the set of detail OIDs for simulation items which should be shown at the top.
   * \param focusSimulationDetailOids The set of detail OIDs, which is copied.
   */
  void setFocusSimulationDetailOids(const std::set<int>& focusSimulationDetailOids)
  {
    focusSimulationDetailOids_ = focusSimulationDetailOids;
  }

  // The initial value for the flash countdown;
  static const int FLASH_COUNT = 6;

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  void timerEvent(QTimerEvent* event) override;
#if QT_CONFIG(wheelevent)
  void wheelEvent(QGraphicsSceneWheelEvent* event) override;
#endif

private:
  friend class AeraVisulizerWindow;

  /**
   * Scale the first QGraphicsView by the given factor.
   * This also sets currentScaleFactor.
   */
  void scaleViewBy(double factor);
  void zoomViewHome();
  void addAeraGraphicsItem(AeraGraphicsItem* item);
  void removeAeraGraphicsItem(AeraGraphicsItem* item);
  /**
   * Add an Arrow to the scene.
   * \param startItem The Item for the start of the arrow.
   * \param endItem The Item for the end of the arrow.
   * \param lhsItem (optional) This is either startItem or endItem. The arrowhead 
   * next to lhsItem will have highlight pen RedArrowheadPen and the other item
   * will have highlight pen Green.ArrowheadPen . If omitted, both arrowheads will
   * have highlight pens HighlightedPen.
   */
  void addArrow(AeraGraphicsItem* startItem, AeraGraphicsItem* endItem, AeraGraphicsItem* lhsItem = 0);
  void addHorizontalLine(AeraGraphicsItem* item);
  /**
   * Get the AeraGraphicsItem whose getAeraEvent() has the given object.
   * \param object The Code* object to search for.
   * \return The AeraGraphicsItem, or null if not found.
   */
  AeraGraphicsItem* getAeraGraphicsItem(r_code::Code* object);
  void establishFlashTimer()
  {
    if (flashTimerId_ == 0)
      flashTimerId_ = startTimer(200);
  }

  /**
   * Find all items with the given event type, and call setItemAndArrowsAndHorizontalLinesVisible.
   * \param eventType The event type of the item's getAeraEvent().
   * \param visible The visible state for setItemAndArrowsAndHorizontalLinesVisible.
   */
  void setItemsVisible(int eventType, bool visible);

  /**
   * Find all items where the event type is not any of the given values, and call setItemAndArrowsAndHorizontalLinesVisible.
   * \param notEventTypes Call setItemAndArrowsAndHorizontalLinesVisible if the event type of the item's getAeraEvent() is not any of these values.
   * \param visible The visible state for setItemAndArrowsAndHorizontalLinesVisible.
   */
  void setNonItemsVisible(const std::set<int>& notEventTypes, bool visible);

  /**
   * Find all AutoFocusFactItem whose object_ is (fact (mk.val X property Y)), and call setItemAndArrowsAndHorizontalLinesVisible.
   * \param property The event type of the item's object_ mk.val.
   * \param visible The visible state for setItemAndArrowsAndHorizontalLinesVisible.
   */
  void setAutoFocusItemsVisible(const std::string& property, bool visible);
  
  /**
   * Find all items where the event type is any of the given values, and call removeArrowsAndHorizontalLines,
   * removeItem and delete the item.
   * \param eventTypes Remove if the event type of the item's getAeraEvent() is any of these values.
   */
  void removeAllItemsByEventType(const std::set<int>& eventTypes);

  AeraVisulizerWindow* parent_;
  ReplicodeObjects& replicodeObjects_;
  bool isMainScene_;
  OnSceneSelected onSceneSelected_;
  r_code::Code* essencePropertyObject_;
  bool didInitialFit_;
  QList<QGraphicsTextItem*> timestampTexts_;
  // key: The AeraEvent eventType_, or 0 for "other". value: The top of the first item for that event type.
  std::map<int, qreal> eventTypeFirstTop_;
  // key: The AeraEvent eventType_, or 0 for "other". value: The top to use for the next item added for that event type.
  std::map<int, qreal> eventTypeNextTop_;
  qreal focusSimulationNextTop_;
  qreal otherSimulationNextTop_;
  Timestamp thisFrameTime_;
  qreal thisFrameLeft_;
  QColor itemColor_;
  QColor simulatedItemColor_;
  QColor lineColor_;
  QPen borderFlashPen_;
  QString noFlashColor_;
  QString valueUpFlashColor_;
  QString valueDownFlashColor_;
  int flashTimerId_;
  std::set<int> focusSimulationDetailOids_;
  static const int frameWidth_ = 330;
};

}

#endif
