#ifndef GRAPH_VISUALIZER_WINDOW_H
#define GRAPH_VISUALIZER_WINDOW_H

#include "aera-visualizer-window-base.h"

namespace aera_visualizer {

class GraphVisulizerWindow : public AeraVisulizerWindowBase
{
  Q_OBJECT

public:
  /**
   * Create a new GraphVisulizerWindow.
   * @param parent The main parent window for this window.
   */
  GraphVisulizerWindow(AeraVisulizerWindowBase* mainWindow);
};

}

#endif
