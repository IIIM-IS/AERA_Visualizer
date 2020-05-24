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
  explicit AeraVisualizerScene(
    QMenu* itemMenu, ReplicodeObjects& replicodeObjects, AeraVisulizerWindow* parent = 0);

  AeraVisulizerWindow* getParent() { return parent_; }

  void zoomToItem(QGraphicsItem* item);

  // The initial value for the flash countdown;
  static const int FLASH_COUNT = 6;

  static const QPen ItemBorderNoHighlightPen;

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

  AeraVisulizerWindow* parent_;
  ReplicodeObjects& replicodeObjects_;
  QMenu* itemMenu_;
  bool didInitialFit_;
  // key: The AeraEvent eventType_, or 0 for "other". value: The top of the first item for that event type.
  std::map<int, qreal> eventTypeFirstTop_;
  // key: The AeraEvent eventType_, or 0 for "other". value: The top to use for the next item added for that event type.
  std::map<int, qreal> eventTypeNextTop_;
  Timestamp thisFrameTime_;
  qreal thisFrameLeft_;
  qreal nextFrameLeft_;
  QColor itemColor_;
  QColor lineColor_;
  QPen borderFlashPen_;
  QString noFlashColor_;
  QString valueUpFlashColor_;
  QString valueDownFlashColor_;
  int flashTimerId_;
};

}

#endif
