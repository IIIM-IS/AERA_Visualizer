#ifndef AERA_EVENT_HPP
#define AERA_EVENT_HPP

#include <QPointF>
// TODO: Include this directly from the Replicode source.
#include "submodules/replicode/submodules/CoreLibrary/CoreLibrary/types.h" 

namespace aera_visualizer {

class AeraEvent {
public:
  AeraEvent(int eventType, core::Timestamp time)
    : eventType_(eventType),
    time_(time)
  {}

  int eventType_;
  core::Timestamp time_;
};

class NewModelEvent : public AeraEvent {
public:
  NewModelEvent(core::Timestamp time, core::uint32 oid, core::float32 confidence)
    : AeraEvent(EVENT_TYPE, time),
    oid_(oid),
    confidence_(confidence),
    itemPosition_(qQNaN(), qQNaN())
  {}

  static const int EVENT_TYPE = 1;

  core::uint32 oid_;
  core::float32 confidence_;
  QPointF itemPosition_;
};

class DeleteModelEvent : public AeraEvent {
public:
  DeleteModelEvent(core::Timestamp time, core::uint32 modelOid)
    : AeraEvent(EVENT_TYPE, time)
  {
    // TODO: Implement.
  }

  static const int EVENT_TYPE = 2;
};

class SetModelConfidenceEvent : public AeraEvent {
public:
  SetModelConfidenceEvent
  (core::Timestamp time, core::uint32 modelOid, core::float32 confidence)
    : AeraEvent(EVENT_TYPE, time),
    modelOid_(modelOid),
    confidence_(confidence),
    oldConfidence_(qQNaN())
  {}

  static const int EVENT_TYPE = 3;

  core::uint32 modelOid_;
  core::float32 confidence_;
  core::float32 oldConfidence_;
};

}

#endif