//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2025 Jeff Thompson
//_/_/ Copyright (c) 2018-2025 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2025 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2021 Karl Asgeir Geirsson
//_/_/ Copyright (c) 2021 Leonard Eberding
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

#include <algorithm>
#include "../submodules/AERA/r_exec/factory.h"
#include "aera-visualizer-window.hpp"
#include "arrow.hpp"
#include "anchored-horizontal-line.hpp"
#include "model-item.hpp"
#include "auto-focus-fact-item.hpp"
#include "aera-graphics-item.hpp"
#include "aera-graphics-item-group.hpp"
#include "aba-sentence-item.hpp"
#include "aera-visualizer-scene.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QApplication>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;
using namespace r_exec;

namespace aera_visualizer {

AeraVisualizerScene::AeraVisualizerScene(
  ReplicodeObjects& replicodeObjects, AeraVisualizerWindow* parent, bool isMainScene,
  const OnSceneSelected& onSceneSelected)
: QGraphicsScene(parent),
  parent_(parent),
  replicodeObjects_(replicodeObjects),
  isMainScene_(isMainScene),
  onSceneSelected_(onSceneSelected),
  essencePropertyObject_(replicodeObjects.getObject("essence")),
  didInitialFit_(false),
  thisFrameTime_(seconds(0)),
  thisFrameLeft_(0),
  borderFlashPen_(Qt::green, 3),
  noFlashColor_("black"),
  valueUpFlashColor_("green"),
  valueDownFlashColor_("red")
{
  lineColor_ = Qt::black;
  setBackgroundBrush(QColor(245, 245, 245));
  flashTimerId_ = 0;
  setSceneRect(QRectF(0, 0, 20000, 100000));

  if (isMainScene_) {
    eventTypeFirstTop_[IoDeviceEjectEvent::EVENT_TYPE] = 20;
    eventTypeFirstTop_[IoDeviceInjectEvent::EVENT_TYPE] = 45;
    eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE] = 120;
    eventTypeFirstTop_[ModelMkValPredictionReduction::EVENT_TYPE] = 555;
    eventTypeFirstTop_[PredictionResultEvent::EVENT_TYPE] = 830;
    eventTypeFirstTop_[NewInstantiatedCompositeStateEvent::EVENT_TYPE] = 1000;
    eventTypeFirstTop_[NewPredictedInstantiatedCompositeStateEvent::EVENT_TYPE] = 1520;
    eventTypeFirstTop_[0] = 580;
  }
  else
    // The default, which is used for the models scene.
    eventTypeFirstTop_[0] = 5;
}

void AeraVisualizerScene::addAeraGraphicsItem(AeraGraphicsItem* item)
{
  auto aeraEvent = item->getAeraEvent();

  if (!didInitialFit_) {
    didInitialFit_ = true;
    // Set the height of the view. The width will be set accordingly.
    QGraphicsView* view = views().at(0);
    view->fitInView(QRectF(0, 0, 1, 800), Qt::KeepAspectRatio);

    if (isMainScene_) {
      // Adjust the position to align the first item to the left side.
      int firstFrameNumber = duration_cast<microseconds>(aeraEvent->time_ - replicodeObjects_.getTimeReference()).count() /
        replicodeObjects_.getSamplingPeriod().count();
      int firstFrameLeft = frameWidth_ * firstFrameNumber;
      // Temporarily set to NoAnchor to override other controls.
      auto saveAnchor = view->transformationAnchor();
      view->setTransformationAnchor(QGraphicsView::NoAnchor);
      view->translate(10 - firstFrameLeft, 0);
      view->setTransformationAnchor(saveAnchor);

      // Separate from the region of I/O device eject/inject events.
      auto y = eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE] - 5;
      auto line = addLine(sceneRect().left(), y, sceneRect().right(), y, QPen(Qt::darkGray, 1));
      line->setZValue(-2000);

      // Add all the frame boundary lines and timestamps.
      for (auto frameTime = replicodeObjects_.getTimeReference(); true; frameTime += replicodeObjects_.getSamplingPeriod()) {
        int frameLeft = getTimelineX(frameTime);
        if (frameLeft > sceneRect().right())
          break;

        auto line = addLine(frameLeft, sceneRect().top(), frameLeft, sceneRect().bottom(),
          QPen(Qt::lightGray, 1, Qt::DashLine));
        line->setZValue(-2000);
        auto text = addText(replicodeObjects_.relativeTime(frameTime).c_str());
        text->setZValue(-100);
        text->setDefaultTextColor(Qt::darkGray);
        text->setPos(frameLeft, 0);
        // Save the text so that we can adjust the position.
        timestampTexts_.push_back(text);
      }
    }
  }

  bool isFocusSimulation = (item->getAeraEvent()->object_ &&
                            focusSimulationDetailOids_.find(item->getAeraEvent()->object_->get_detail_oid())
                            != focusSimulationDetailOids_.end());
  bool isSimulationEventType = 
    (AeraVisualizerWindow::simulationEventTypes_.find(item->getAeraEvent()->eventType_) !=
     AeraVisualizerWindow::simulationEventTypes_.end());

  AeraGraphicsItemGroup* itemGroup = 0;
  if (aeraEvent->eventType_ == AbaAddSentence::EVENT_TYPE) {
    auto addSentence = (AbaAddSentence*)aeraEvent;

    itemGroup = getItemGroup(addSentence->graphId_);
    if (!itemGroup) {
      int solutionId = addSentence->graphId_ / 100;
      if (addSentence->graphId_ % 100 != 0)
        itemGroup = new AeraGraphicsItemGroup(
          this, "O" + QString::number(solutionId) + ":" + QString::number(addSentence->graphId_ % 100),
          AeraGraphicsItem::Color_opponent_unfinished_justification);
      else
        itemGroup = new AeraGraphicsItemGroup(
          this, "P" + QString::number(solutionId), AeraGraphicsItem::Color_proponent_justifications);

      itemGroups_[addSentence->graphId_] = itemGroup;
      // Put in back of the grid lines.
      itemGroup->setZValue(-2100);
      addItem(itemGroup);
    }
  }

  if (qIsNaN(aeraEvent->itemTopLeftPosition_.x())) {
    // Assign an initial position.
    // Only update positions based on time for the main scene.
    if (isMainScene_ && aeraEvent->time_ >= thisFrameTime_ + replicodeObjects_.getSamplingPeriod()) {
      // Start a new frame (or the first frame).
      auto relativeTime = duration_cast<microseconds>(aeraEvent->time_ - replicodeObjects_.getTimeReference());
      thisFrameTime_ = aeraEvent->time_ - (relativeTime % replicodeObjects_.getSamplingPeriod());
      thisFrameLeft_ = getTimelineX(thisFrameTime_);
      // Reset the top.
      eventTypeNextTop_.clear();
      focusSimulationNextTop_ = eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE];
      otherSimulationNextTop_ = 3000 + eventTypeFirstTop_[AutoFocusNewObjectEvent::EVENT_TYPE];
    }

    int eventType = 0;
    if (eventTypeFirstTop_.find(aeraEvent->eventType_) != eventTypeFirstTop_.end())
      // This is a recognized event type.
      eventType = aeraEvent->eventType_;

    if (aeraEvent->eventType_ == AutoFocusNewObjectEvent::EVENT_TYPE) {
      auto mkVal = aeraEvent->object_->get_reference(0);
      if (essencePropertyObject_ && mkVal->references_size() >= 2 && mkVal->get_reference(1) == essencePropertyObject_)
        // Override to group essence facts with non-assigned types at the bottom.
        eventType = 0;
    }

    // Compute top.
    qreal top;
    if (itemGroup) {
      if (!qIsNaN(itemGroup->getNextTop()))
        top = itemGroup->getNextTop();
      else {
        // First position for this group. Get the lowest nextTop of all the item groups.
        top = focusSimulationNextTop_;
        for (auto otherGroup = itemGroups_.begin(); otherGroup != itemGroups_.end(); ++otherGroup) {
          if (!qIsNaN(otherGroup->second->getNextTop()))
            top = max(top, otherGroup->second->getNextTop());
        }
      }

      top += 10;
    }
    else {
      if (isSimulationEventType)
        // Ignore eventType and stack the simulated items in order.
        top = (isFocusSimulation ? focusSimulationNextTop_ : otherSimulationNextTop_);
      else {
        if (eventTypeNextTop_.find(eventType) != eventTypeNextTop_.end())
          top = eventTypeNextTop_[eventType];
        else {
          top = eventTypeFirstTop_[eventType];

          if (thisFrameTime_ - replicodeObjects_.getTimeReference() < milliseconds(150) &&
              eventType == AutoFocusNewObjectEvent::EVENT_TYPE &&
              aeraEvent->object_->get_reference(0)->references_size() >= 2 &&
              aeraEvent->object_->get_reference(0)->get_reference(1) == replicodeObjects_.getObject("essence"))
            // Debug: The first essence item. Override to make the same types of values line up. Should use a layout algorithm.
            top = 296;
        }
      }

      // Set up eventTypeNextTop_ or simulation next top for the next item.
      int verticalMargin = 15;
      if (aeraEvent->eventType_ == IoDeviceInjectEvent::EVENT_TYPE ||
          aeraEvent->eventType_ == IoDeviceEjectEvent::EVENT_TYPE)
        verticalMargin = 5;
      qreal nextTop = top + item->boundingRect().height() + verticalMargin;
      if (isSimulationEventType) {
        if (isFocusSimulation)
          focusSimulationNextTop_ = nextTop;
        else
          otherSimulationNextTop_ = nextTop;
      }
      else
        eventTypeNextTop_[eventType] = nextTop;
    }

    qreal left;
    if (isSimulationEventType) {
      // Position simulated items exactly.
      if (aeraEvent->eventType_ == ModelPredictionFromRequirementDisabledEvent::EVENT_TYPE)
        // Special case. aeraEvent->object_ is null, so use the strong_requirement.
        left = getTimelineX(((_Fact*)((ModelPredictionFromRequirementDisabledEvent*)aeraEvent)->strong_requirement_
                                       ->get_reference(0)->get_reference(0))->get_after());
      else if (aeraEvent->eventType_ == PromotedSimulatedPredictionDefeatEvent::EVENT_TYPE)
        // Special case. aeraEvent->object_ is null, so use the input_.
        left = getTimelineX(((_Fact*)((PromotedSimulatedPredictionDefeatEvent*)aeraEvent)->input_
                                       ->get_reference(0)->get_reference(0))->get_after());
      else {
        // We know that a simulated item's object usually has the form (fact (goal_or_pred (fact ...)))
        if (((_Fact*)aeraEvent->object_)->get_goal())
          // Position a goal at the time it needs to be achieved by.
          left = getTimelineX(((_Fact*)aeraEvent->object_->get_reference(0)->get_reference(0))->get_before()) -
            item->boundingRect().width();
        else if (((_Fact*)aeraEvent->object_)->get_pred())
          left = getTimelineX(((_Fact*)aeraEvent->object_->get_reference(0)->get_reference(0))->get_after());
        else
          // No goal or pred, just a solo fact. Position like a goal.
          left = getTimelineX(((_Fact*)aeraEvent->object_)->get_after());
      }
    }
    else {
      if (aeraEvent->eventType_ == IoDeviceInjectEvent::EVENT_TYPE ||
          aeraEvent->eventType_ == IoDeviceEjectEvent::EVENT_TYPE)
        // Allow inject/eject items to be on the frame boundary.
        left = thisFrameLeft_ + item->boundingRect().left();
      else
        left = thisFrameLeft_ + 5;
    }
    aeraEvent->itemTopLeftPosition_ = QPointF(left, top);
  }

  if (qIsNaN(aeraEvent->itemInitialTopLeftPosition_.x()))
    // Save the initial position for "Reset Position".
    aeraEvent->itemInitialTopLeftPosition_ = aeraEvent->itemTopLeftPosition_;

  addItem(item);
  // Adjust the position from the topLeft.
  item->setPos(aeraEvent->itemTopLeftPosition_ - item->boundingRect().topLeft());
  item->adjustItemYPosition();
  if (itemGroup) {
    qreal saveHeight = itemGroup->boundingRect().height();
    itemGroup->addChild(item);
    
    qreal deltaHeight = itemGroup->boundingRect().height() - saveHeight;
    if (saveHeight > 1 && deltaHeight > 0) {
      // Shift overlapping groups down by deltaHeight.
      for (auto otherGroup = itemGroups_.begin(); otherGroup != itemGroups_.end(); ++otherGroup) {
        if (otherGroup->second->pos().y() > itemGroup->pos().y())
          otherGroup->second->setPos(otherGroup->second->pos().x(),
                                     otherGroup->second->pos().y() + deltaHeight);
      }
    }
  }
}

void AeraVisualizerScene::removeAeraGraphicsItem(AeraGraphicsItem* item) {
  if (item->getAeraEvent()->eventType_ == AbaAddSentence::EVENT_TYPE) {
    auto itemGroup = getItemGroup(((AbaAddSentence*)item->getAeraEvent())->graphId_);
    if (itemGroup) {
      itemGroup->removeChild(item);
      // TODO: Remove itemGroup if it is empty. (Remember its position for it we step forward again?)
    }
  }

  removeItem(item);
}

void AeraVisualizerScene::onViewMoved()
{
  if (views().size() >= 1) {
    // Always position the timestamps at the top of the screen.
    qreal sceneY = views().at(0)->mapToScene(0, 0).y();
    foreach(QGraphicsTextItem * text, timestampTexts_)
      text->setPos(text->x(), sceneY);

    for (auto itemGroup = itemGroups_.begin(); itemGroup != itemGroups_.end(); ++itemGroup)
      itemGroup->second->onParentViewMoved();
  }
}

void AeraVisualizerScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  if (mouseEvent->button() != Qt::LeftButton)
    return;

  views().at(0)->setDragMode(QGraphicsView::ScrollHandDrag);
  if (onSceneSelected_)
    // Notify the parent that this scene was selected.
    onSceneSelected_();
  QGraphicsScene::mousePressEvent(mouseEvent);
}

void AeraVisualizerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  QGraphicsView *qGraphicsView = views().at(0);

  AeraGraphicsItem *aeraGraphicsItem = qgraphicsitem_cast<AeraGraphicsItem *>(mouseGrabberItem());
  if (aeraGraphicsItem)
    aeraGraphicsItem->ensureVisible();

  // Reset the drag mode.
  qGraphicsView->setDragMode(QGraphicsView::NoDrag);
  QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void AeraVisualizerScene::addArrow(
  AeraGraphicsItem* startItem, AeraGraphicsItem* endItem, AeraGraphicsItem* lhsItem)
{
  if (startItem == endItem)
    return;

  QPen highlightArrowBasePen = Arrow::HighlightedPen;
  QPen highlightArrowTipPen = Arrow::HighlightedPen;
  if (lhsItem == startItem) {
    highlightArrowBasePen = Arrow::RedArrowheadPen;
    highlightArrowTipPen = Arrow::GreenArrowheadPen;
  }
  else if (lhsItem == endItem) {
    highlightArrowBasePen = Arrow::GreenArrowheadPen;
    highlightArrowTipPen = Arrow::RedArrowheadPen;
  }
  addArrow(startItem, endItem, Arrow::HighlightedPen, highlightArrowBasePen, highlightArrowTipPen);
}

void AeraVisualizerScene::addArrow(
    AeraGraphicsItem* startItem, AeraGraphicsItem* endItem, const QPen& highlightBodyPen,
    const QPen& highlightArrowBasePen, const QPen& highlightArrowTipPen)
{
  auto arrow = new Arrow(startItem, endItem, highlightBodyPen, highlightArrowBasePen, highlightArrowTipPen, this);

  startItem->addArrow(arrow);
  endItem->addArrow(arrow);
  arrow->setZValue(-1000.0);
  addItem(arrow);
  arrow->updatePosition();
}

void AeraVisualizerScene::addHorizontalLine(AeraGraphicsItem* item)
{
  if (item->getAeraEvent()->object_ &&
      (item->getAeraEvent()->object_->code(0).asOpcode() == r_exec::Opcodes::Fact ||
       item->getAeraEvent()->object_->code(0).asOpcode() == r_exec::Opcodes::AntiFact)) {
    auto fact = (_Fact*)item->getAeraEvent()->object_;
    Timestamp after, before;

    auto goal = fact->get_goal();
    if (goal) {
      // Use the timings of the goal target.
      after = goal->get_target()->get_after();
      before = goal->get_target()->get_before();
    }
    else {
      auto pred = fact->get_pred();
      if (pred) {
        // Use the timings of the prediction target.
        after = pred->get_target()->get_after();
        before = pred->get_target()->get_before();
      }
      else {
        after = fact->get_after();
        before = fact->get_before();
      }
    }

    auto line = new AnchoredHorizontalLine(item, getTimelineX(after), getTimelineX(before));
    item->setHorizontalLine(line);
    line->setZValue(-1001.0);
    addItem(line);
    line->updatePosition();
  }
}

AeraGraphicsItem* AeraVisualizerScene::getAeraGraphicsItem(Code* object)
{
  foreach(QGraphicsItem* item, items()) {
    auto graphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (graphicsItem) {
      if (graphicsItem->getAeraEvent()->object_ == object)
        return graphicsItem;
    }
  }

  return 0;
}

// Redo highlights in case something's changed
void AeraVisualizerScene::updateHighlights() {
  double currentScale = views().at(0)->transform().m11();
  double strokeScale = (std::max)(1.0, 1/currentScale);
  
  QPen scaledAllMatches(Qt::blue, 3 * strokeScale);
  QPen scaledCurrentMatch(Qt::cyan, 3 * strokeScale);

  // Do all the highlights
  foreach(AeraGraphicsItem * item, allMatches_) {
    if (item)
      item->setPen(scaledAllMatches);
  }

  if (currentMatch_) {
    currentMatch_->setPen(scaledCurrentMatch);
  }
}

// Reset highlights and wipe the list
void AeraVisualizerScene::unhighlightAll() {
  foreach(AeraGraphicsItem * item, allMatches_) {
    if (item)
      item->restorePen();
  }

  if (currentMatch_)
    currentMatch_->restorePen();

  currentMatch_ = NULL;
  allMatches_.clear();

  updateHighlights();
}

void AeraVisualizerScene::scaleViewBy(double factor)
{
  double currentScale = views().at(0)->transform().m11();

  QGraphicsView* view = views().at(0);
  QMatrix oldMatrix = view->matrix();
  view->resetMatrix();
  view->translate(oldMatrix.dx(), oldMatrix.dy());
  view->scale(currentScale *= factor, currentScale *= factor);
  
  updateHighlights(); // Any zoom level change must update these
}

void AeraVisualizerScene::zoomViewHome()
{
  // Get the bounding rect of all AeraGraphicsItem. This excludes lines such as frame boundaries.
  QRectF boundingRect;
  foreach(QGraphicsItem* item, items()) {
    if (dynamic_cast<AeraGraphicsItem*>(item) && item->isVisible()) {
      if (boundingRect.width() == 0)
        boundingRect = item->sceneBoundingRect();
      else
        boundingRect = boundingRect.united(item->sceneBoundingRect());
    }
  }

  if (boundingRect.width() != 0)
    views().at(0)->fitInView(boundingRect, Qt::KeepAspectRatio);

  updateHighlights(); // Any zoom level change must update these
}

void AeraVisualizerScene::centerOnItem(QGraphicsItem *item) {
  auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
  if (aeraGraphicsItem) {
    if (!aeraGraphicsItem->isVisible())
      aeraGraphicsItem->setItemAndArrowsAndHorizontalLineVisible(true);

    aeraGraphicsItem->centerOn();
  }
}

void AeraVisualizerScene::focusOnItem(QGraphicsItem* item)
{
  auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
  if (aeraGraphicsItem) {
    if (!aeraGraphicsItem->isVisible())
      aeraGraphicsItem->setItemAndArrowsAndHorizontalLineVisible(true);

    aeraGraphicsItem->focus();
  }
}

void AeraVisualizerScene::zoomToItem(QGraphicsItem* item)
{
  double minimumZoomLevel = 1;

  QGraphicsView* qGraphicsView = views().at(0);

  double currentScale = qGraphicsView->transform().m11();

  if (currentScale < minimumZoomLevel) {
    qGraphicsView->resetMatrix();
    qGraphicsView->scale(minimumZoomLevel, minimumZoomLevel);
  }

  updateHighlights(); // Any zoom level change must update these

  focusOnItem(item);
}

void AeraVisualizerScene::scrollToTimestamp(core::Timestamp timestamp) {
  auto relativeTime = duration_cast<microseconds>(timestamp - replicodeObjects_.getTimeReference());
  auto frameStartTime = timestamp - (relativeTime % replicodeObjects_.getSamplingPeriod());
  qreal xPos = getTimelineX(frameStartTime);
  // This point marks the top left of the scrolled scene
  // it is used to keep the same y position while scrolling
  QGraphicsView* view = views().at(0);
  QPointF scenePoint = view->mapToScene(QPoint(0,0));
  view->ensureVisible(xPos, scenePoint.y(), frameWidth_, 0 , 0, 0);
}

void AeraVisualizerScene::setItemsVisible(int eventType, bool visible)
{
  foreach(QGraphicsItem * item, items()) {
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (aeraGraphicsItem && aeraGraphicsItem->getAeraEvent()->eventType_ == eventType)
      aeraGraphicsItem->setItemAndArrowsAndHorizontalLineVisible(visible);
  }
}

void AeraVisualizerScene::setNonItemsVisible(const set<int>& notEventTypes, bool visible)
{
  foreach(QGraphicsItem * item, items()) {
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (aeraGraphicsItem && 
        notEventTypes.find(aeraGraphicsItem->getAeraEvent()->eventType_) == notEventTypes.end())
      aeraGraphicsItem->setItemAndArrowsAndHorizontalLineVisible(visible);
  }
}

void AeraVisualizerScene::setAutoFocusItemsVisible(const string& property, bool visible)
{
  auto propertyObject = (property == "essence" ? essencePropertyObject_ : replicodeObjects_.getObject(property));
  if (!propertyObject)
    return;

  foreach(QGraphicsItem * item, items()) {
    auto autoFocusItem = dynamic_cast<AutoFocusFactItem*>(item);
    if (autoFocusItem) {
      auto mkVal = autoFocusItem->getAeraEvent()->object_->get_reference(0);
      // TODO: Is this reference always the property?
      if (mkVal->references_size() < 2)
        continue;

      auto mkValProperty = mkVal->get_reference(1);
      if (mkValProperty == propertyObject)
        autoFocusItem->setItemAndArrowsAndHorizontalLineVisible(visible);
    }
  }
}

void AeraVisualizerScene::removeAllItemsByEventType(const set<int>& eventTypes)
{
  // First find the items to delete without deleting, which modifies items().
  vector<AeraGraphicsItem*> toDelete;
  foreach(QGraphicsItem * item, items()) {
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (aeraGraphicsItem &&
      eventTypes.find(aeraGraphicsItem->getAeraEvent()->eventType_) != eventTypes.end())
      toDelete.push_back(aeraGraphicsItem);
  }

  for (auto item = toDelete.begin(); item != toDelete.end(); ++item) {
    (*item)->removeArrowsAndHorizontalLine();
    removeAeraGraphicsItem(*item);
    delete *item;
  }
}

void AeraVisualizerScene::abaSetBinding(int varNumber, const QString& text)
{
  set<AeraGraphicsItemGroup*> toRefit;

  foreach(QGraphicsItem * item, items()) {
    auto abaItem = dynamic_cast<AbaSentenceItem*>(item);
    if (abaItem) {
      if (abaItem->setBinding(varNumber, text)) {
        // The item was changed, so need to re-fit its group box.
        auto itemGroup = getItemGroup(((AbaAddSentence*)abaItem->getAeraEvent())->graphId_);
        if (itemGroup)
          // fitToChildren is expensive, so only call it later, once for each group.
          toRefit.insert(itemGroup);
      }
    }
  }

  for (auto itemGroup = toRefit.begin(); itemGroup != toRefit.end(); itemGroup++)
    (*itemGroup)->fitToChildren();
}

void AeraVisualizerScene::abaRemoveBinding(int varNumber)
{
  foreach(QGraphicsItem * item, items()) {
    auto abaItem = dynamic_cast<AbaSentenceItem*>(item);
    if (abaItem)
      abaItem->removeBinding(varNumber);
  }
}

#if QT_CONFIG(wheelevent)
void AeraVisualizerScene::wheelEvent(QGraphicsSceneWheelEvent* event)
{
  if (event->modifiers() == Qt::ControlModifier) {
    // Accept the event to override other behavior.
    event->accept();
    QGraphicsView* view = views().at(0);
    
    // Zoom around the mouse
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Scale
    scaleViewBy(pow((double)2, event->delta() / 1000.0));
  }
}
#endif

void AeraVisualizerScene::timerEvent(QTimerEvent* event)
{
  // TODO: Make sure we don't re-enter.

  if (event->timerId() != flashTimerId_)
    // This timer event is not for us.
    return;

  bool isFlashing = false;
  foreach(QGraphicsItem* item, items()) {
    auto aeraGraphicsItem = dynamic_cast<AeraGraphicsItem*>(item);
    if (!aeraGraphicsItem)
      continue;

    if (aeraGraphicsItem->borderFlashCountdown_ > 0) {
      isFlashing = true;

      --aeraGraphicsItem->borderFlashCountdown_;
      if (aeraGraphicsItem->borderFlashCountdown_ % 2 == 1)
        aeraGraphicsItem->setPen(borderFlashPen_);
      else
        aeraGraphicsItem->setPen(aeraGraphicsItem->getBorderNoHighlightPen());
    }

    auto modelItem = dynamic_cast<ModelItem*>(item);
    if (modelItem) {
      if (modelItem->strengthFlashCountdown_ > 0) {
        isFlashing = true;

        --modelItem->strengthFlashCountdown_;
        if (modelItem->strengthFlashCountdown_ % 2 == 1)
          modelItem->setStrengthColor
          (modelItem->strengthIncreased_ ? valueUpFlashColor_ : valueDownFlashColor_);
        else
          modelItem->setStrengthColor(noFlashColor_);
      }

      if (modelItem->evidenceCountFlashCountdown_ > 0) {
        isFlashing = true;

        --modelItem->evidenceCountFlashCountdown_;
        if (modelItem->evidenceCountFlashCountdown_ % 2 == 1)
          modelItem->setEvidenceCountColor
          (modelItem->evidenceCountIncreased_ ? valueUpFlashColor_ : valueDownFlashColor_);
        else
          modelItem->setEvidenceCountColor(noFlashColor_);
      }

      if (modelItem->successRateFlashCountdown_ > 0) {
        isFlashing = true;

        --modelItem->successRateFlashCountdown_;
        if (modelItem->successRateFlashCountdown_ % 2 == 1)
          modelItem->setSuccessRateColor
          (modelItem->successRateIncreased_ ? valueUpFlashColor_ : valueDownFlashColor_);
        else
          modelItem->setSuccessRateColor(noFlashColor_);
      }
    }
  }

  if (!isFlashing) {
    killTimer(flashTimerId_);
    flashTimerId_ = 0;
  }
}

}
