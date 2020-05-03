#ifndef AERA_EVENT_HPP
#define AERA_EVENT_HPP

#include <QPointF>
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
  NewModelEvent(core::Timestamp time, core::uint32 oid, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time),
    oid_(oid),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    itemPosition_(qQNaN(), qQNaN())
  {}

  static const int EVENT_TYPE = 1;

  core::uint32 oid_;
  core::float32 evidenceCount_;
  core::float32 successRate_;
  QPointF itemPosition_;
};

class SetModelEvidenceCountAndSuccessRateEvent : public AeraEvent {
public:
  SetModelEvidenceCountAndSuccessRateEvent
  (core::Timestamp time, core::uint32 modelOid, core::float32 evidenceCount, core::float32 successRate)
    : AeraEvent(EVENT_TYPE, time),
    modelOid_(modelOid),
    evidenceCount_(evidenceCount),
    successRate_(successRate),
    oldEvidenceCount_(qQNaN()),
    oldSuccessRate_(qQNaN())
  {}

  static const int EVENT_TYPE = 2;

  core::uint32 modelOid_;
  core::float32 evidenceCount_;
  core::float32 successRate_;
  core::float32 oldEvidenceCount_;
  core::float32 oldSuccessRate_;
};

}

#endif
