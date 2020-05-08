#ifndef EXPLANATION_LOG_WINDOW_HPP
#define EXPLANATION_LOG_WINDOW_HPP

#include <QTextBrowser>
#include "aera-visualizer-window.hpp"

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
  ExplanationLogWindow(AeraVisulizerWindow* mainWindow);

  void appendHtml(const QString& html)
  {
    // TODO: Does QTextBrowser have an actual append operation?
    html_ += html;
    textBrowser_->setText(html_);
  }

  void appendHtml(const std::string& html) { appendHtml(QString(html.c_str())); }

protected:
  // TODO: Implement.
  bool haveMoreEvents() override { return false; }
  // TODO: Implement.
  core::Timestamp stepEvent(core::Timestamp maximumTime) override { return r_code::Utils_MaxTime; }
  // TODO: Implement.
  core::Timestamp unstepEvent(core::Timestamp minimumTime) override { return r_code::Utils_MaxTime; }

private slots:
  void textBrowserAnchorClicked(const QUrl& url);

private:
  // TODO: We should be able to use textBrowser_.
  QString html_;
  QTextBrowser* textBrowser_;
};

}

#endif
