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
   * \param replicodeObjects The ReplicodeObjects used to find objects.
   */
  ExplanationLogWindow(AeraVisulizerWindow* mainWindow, ReplicodeObjects& replicodeObjects);

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
  /**
   * ExplanationLogWindow::TextBrowser extends QTextBrowser so that we can override its
   * mouseMoveEvent.
   */
  class TextBrowser : public QTextBrowser {
  public:
    TextBrowser(ExplanationLogWindow* parent)
      : QTextBrowser(parent), parent_(parent)
    {}

    ExplanationLogWindow* parent_;
    QString previousUrl_;

  protected:
    void mouseMoveEvent(QMouseEvent* event) override;
  };
  friend TextBrowser;

  AeraVisulizerWindow* parent_;
  ReplicodeObjects& replicodeObjects_;
  // TODO: We should be able to use textBrowser_ to append HTML.
  QString html_;
  TextBrowser* textBrowser_;
};

}

#endif
