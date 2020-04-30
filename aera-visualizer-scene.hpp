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
  explicit AeraVisualizerScene(QMenu* itemMenu, QObject* parent = 0);

  /**
   * Scale the first QGraphicsView by the given factor.
   * This also sets currentScaleFactor.
   */
  void scaleViewBy(double factor);
  void zoomViewHome();
  AeraModelItem* addAeraModelItem(NewModelEvent* newModelEvent);
  void addArrow(AeraModelItem* startItem, AeraModelItem* endItem);
  /**
   * Get the AeraModelItem whose NewModelEvent has the given oid.
   * \return The AeraModelItem, or null if not found.
   */
  AeraModelItem* getAeraModelItem(core::uint32 oid);
  void establishFlashTimer()
  {
    if (flashTimerId_ == 0)
      flashTimerId_ = startTimer(200);
  }

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
  void timerEvent(QTimerEvent* event) override;
#if QT_CONFIG(wheelevent)
  void wheelEvent(QGraphicsSceneWheelEvent* event) override;
#endif

private:
  QMenu* itemMenu_;
  bool leftButtonDown_;
  QPointF startPoint_;
  QGraphicsLineItem* line_;
  QColor itemColor_;
  QColor lineColor_;
  QPen borderFlashPen_;
  QColor valueUpFlashColor_;
  QColor valueDownFlashColor_;
  int flashTimerId_;
};

}

#endif
