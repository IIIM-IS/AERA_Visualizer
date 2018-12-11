#ifndef GRAPH_VISUALIZER_WINDOW_H
#define GRAPH_VISUALIZER_WINDOW_H

#include "aera-visualizer-window-base.h"

namespace aera_visualizer {

/**
 * GraphVisulizerWindow extends AeraVisulizerWindowBase to present the player
 * control panel and a window for visualizing a graph.
 */
class GraphVisulizerWindow : public AeraVisulizerWindowBase
{
  Q_OBJECT

public:
  /**
   * Create a GraphVisulizerWindow.
   * @param parent The main parent window for this window.
   */
  GraphVisulizerWindow(AeraVisulizerWindowBase* mainWindow);
};

}

#endif
