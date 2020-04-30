#ifndef EXPLANATION_LOG_WINDOW_HPP
#define EXPLANATION_LOG_WINDOW_HPP

#include "aera-visualizer-window-base.hpp"

namespace aera_visualizer {

/**
 * ExplanationLogWindow extends AeraVisulizerWindowBase to present the player
 * control panel and the explanation log.
 */
class ExplanationLogWindow : public AeraVisulizerWindowBase
{
  Q_OBJECT

public:
  /**
   * Create an ExplanationLogWindow.
   * \param parent The main parent window for this window.
   */
  ExplanationLogWindow(AeraVisulizerWindowBase* mainWindow);

protected:
  // TODO: Implement.
  bool haveMoreEvents() override { return false; }
  // TODO: Implement.
  core::Timestamp stepEvent(core::Timestamp maximumTime) override { return r_code::Utils_MaxTime; }
  // TODO: Implement.
  core::Timestamp unstepEvent() override { return r_code::Utils_MaxTime; }
};

}

#endif
