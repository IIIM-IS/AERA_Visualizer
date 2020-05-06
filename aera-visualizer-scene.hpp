#ifndef AERA_VISUALIZER_SCENE_HPP
#define AERA_VISUALIZER_SCENE_HPP

#include <map>
#include "aera-model-item.hpp"
#include "aera-event.hpp"

#include <QGraphicsScene>

class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QColor;

namespace aera_visualizer {

class AeraVisualizerScene : public QGraphicsScene
{
  Q_OBJECT

public:
  explicit AeraVisualizerScene(QMenu* itemMenu, ReplicodeObjects& replicodeObjects, QObject* parent = 0);

  /**
   * Scale the first QGraphicsView by the given factor.
   * This also sets currentScaleFactor.
   */
  void scaleViewBy(double factor);
  void zoomViewHome();
  void zoomToItem(QGraphicsItem* item);
  AeraModelItem* addAeraModelItem(NewModelEvent* newModelEvent);
  void addArrow(AeraGraphicsItem* startItem, AeraGraphicsItem* endItem);
  /**
   * Get the getAeraGraphicsItem whose getNewObjectEvent() has the given object.
   * \param object The Code* object to search for.
   * \return The AeraGraphicsItem, or null if not found.
   */
  AeraGraphicsItem* getAeraGraphicsItem(r_code::Code* object);
  void establishFlashTimer()
  {
    if (flashTimerId_ == 0)
      flashTimerId_ = startTimer(200);
  }
  QPoint getMouseScreenPosition() { return mouseScreenPosition_;  }

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  void timerEvent(QTimerEvent* event) override;
#if QT_CONFIG(wheelevent)
  void wheelEvent(QGraphicsSceneWheelEvent* event) override;
#endif

private:
  ReplicodeObjects& replicodeObjects_;
  QMenu* itemMenu_;
  bool leftButtonDown_;
  QPointF startPoint_;
  QGraphicsLineItem* line_;
  QColor itemColor_;
  QColor lineColor_;
  QPen borderFlashPen_;
  QString noFlashColor_;
  QString valueUpFlashColor_;
  QString valueDownFlashColor_;
  int flashTimerId_;
  QPoint mouseScreenPosition_;
};

}

#endif
