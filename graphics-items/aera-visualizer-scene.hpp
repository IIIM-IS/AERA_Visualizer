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

#ifndef AERA_VISUALIZER_SCENE_HPP
#define AERA_VISUALIZER_SCENE_HPP

#include <map>
#include "../aera-event.hpp"
#include "aera-graphics-item.hpp"
#include "../replicode-objects.hpp"

#include <QGraphicsScene>

class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QColor;

namespace aera_visualizer {

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
  void addArrow(AeraGraphicsItem* startItem, AeraGraphicsItem* endItem);
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

  AeraVisulizerWindow* parent_;
  ReplicodeObjects& replicodeObjects_;
  bool isMainScene_;
  OnSceneSelected onSceneSelected_;
  r_code::Code* essencePropertyObject_;
  bool didInitialFit_;
  // key: The AeraEvent eventType_, or 0 for "other". value: The top of the first item for that event type.
  std::map<int, qreal> eventTypeFirstTop_;
  // key: The AeraEvent eventType_, or 0 for "other". value: The top to use for the next item added for that event type.
  std::map<int, qreal> eventTypeNextTop_;
  qreal selectedSimulationNextTop_;
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
  static const int frameWidth_ = 330;
};

}

#endif
