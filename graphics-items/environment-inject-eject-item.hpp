#ifndef ENVIRONMENT_INJECT_EJECT_ITEM_HPP
#define ENVIRONMENT_INJECT_EJECT_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class EnvironmentInjectEjectItem : public AeraGraphicsItem
{
public:
  /**
   * Create an EnvironmentInjectEjectItem.
   * \param event This may be EnvironmentInjectEvent or EnvironmentEjectEvent.
   * \param replicodeObjects
   * \param parent
   */
  EnvironmentInjectEjectItem(
    AeraEvent* event, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

  static const QString UpWideArrowHtml;
  static const QString DownWideArrowHtml;

protected:
  void textItemLinkActivated(const QString& link) override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
  /**
   * Set labelHtml_ to the HTML source code for the label and inject/eject symbol from
   * event_->object_.
   */
  void setLabelHtml();

  /**
   * Set factValHtml_ to the HTML source code for the fact and value from
   * autoFocusNewObjectEvent_->object_.
   */
  void setFactValHtml();

  AeraEvent* event_;
  QString labelHtml_;
  QString factValHtml_;
};

}

#endif
