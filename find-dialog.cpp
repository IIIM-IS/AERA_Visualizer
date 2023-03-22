//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2023 Jeff Thompson
//_/_/ Copyright (c) 2018-2023 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2023 Icelandic Institute for Intelligent Machines
//_/_/ Copyright (c) 2023 Chloe Schaff
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


#include "aera-visualizer-window.hpp"
#include "graphics-items/aera-visualizer-scene.hpp"
#include "find-dialog.hpp"

#include <regex>
#include <string>
#include <string.h>


using namespace std;

namespace aera_visualizer {

  FindDialog::FindDialog(AeraVisualizerWindow* parent, ReplicodeObjects& replicodeObjects) : QDialog(parent) {

    // Store these for reference
    parentWindow_ = parent;
    replicodeObjects_ = replicodeObjects;

    // Set up the window
    setWindowTitle("Find");
    setWindowIcon(QIcon(":/images/app.ico"));
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    // Initialize the widgets
    QLabel* prompt = new QLabel("Find what:", this);
    input_ = new QLineEdit(this);
    wraparound_ = new QCheckBox("&Wrap around", this);
    skipHidden_ = new QCheckBox("Skip &hidden items", this);
    zoomTo_ = new QCheckBox("&Zoom to items", this);
    highlightAll_ = new QCheckBox("Highlight &all", this);
    QPushButton* findNextButton = new QPushButton("Find Next", this);
    QPushButton* findPrevButton = new QPushButton("Find Prev", this);
    QPushButton* fitAllButton = new QPushButton("Fit all matches", this);
    status_ = new QLabel(this);

    status_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    connect(status_, SIGNAL(linkHovered(const QString)), this, SLOT(hiddenLinkHovered(const QString)));
    connect(status_, SIGNAL(linkActivated(const QString)), this, SLOT(hiddenLinkActivated(const QString)));

    // Initialize and build the layouts
    QVBoxLayout* stack = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout(this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(this);
    QVBoxLayout* buttonsLayout = new QVBoxLayout(this);
    QHBoxLayout* middleLayout = new QHBoxLayout(this);
    stack->addLayout(inputLayout);                // Prompt and text box
    middleLayout->addLayout(optionsLayout, 4);    // Checkboxes
    middleLayout->addLayout(buttonsLayout, 1);    // Buttons
    stack->addLayout(middleLayout);
    stack->addWidget(status_);                    // Status bar at bottom
    inputLayout->addWidget(prompt, 1);
    inputLayout->addWidget(input_, 4);
    optionsLayout->addWidget(wraparound_);
    optionsLayout->addWidget(skipHidden_);
    optionsLayout->addWidget(zoomTo_);
    optionsLayout->addWidget(highlightAll_);
    buttonsLayout->addWidget(findNextButton);
    buttonsLayout->addWidget(findPrevButton);
    buttonsLayout->addWidget(fitAllButton);
    buttonsLayout->addStretch();

    // Set up some initial values and placeholders
    input_->setPlaceholderText("Enter an object name");
    skipHidden_->setCheckState(Qt::Checked);
    zoomTo_->setCheckState(Qt::Checked);

    // Set up the autocompleter for input_
    completer_ = new QCompleter(wordList_, this);
    completer_->setCaseSensitivity(Qt::CaseInsensitive);
    completer_->setCompletionMode(QCompleter::InlineCompletion);
    completer_->setFilterMode(Qt::MatchContains);
    input_->setCompleter(completer_);

    // Set up some actions for navigating completions
    QAction* select = new QAction(tr("Select Completion"), this);
    QAction* next = new QAction(tr("Next Completion"), this);
    QAction* prev = new QAction(tr("Prev Completion"), this);
    select->setShortcut(QKeySequence(Qt::Key_Tab));
    next->setShortcut(QKeySequence(Qt::Key_Down));
    prev->setShortcut(QKeySequence(Qt::Key_Up));

    // Connect those actions
    connect(select, SIGNAL(triggered()), this, SLOT(selectCompletion()));
    connect(next, SIGNAL(triggered()), this, SLOT(nextCompletion()));
    connect(prev, SIGNAL(triggered()), this, SLOT(prevCompletion()));
    this->addAction(select);
    this->addAction(next);
    this->addAction(prev);

    // Connect the Find Next button and action
    QAction* findNextAction = new QAction(tr("Find Next"), this);
    findNextAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    connect(findNextButton, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(findNextAction, SIGNAL(triggered()), this, SLOT(findNext()));
    this->addAction(findNextAction);

    // Connect the Find Prev button and action
    QAction* findPrevAction = new QAction(tr("Find Prev"), this);
    findPrevAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
    connect(findPrevButton, SIGNAL(clicked()), this, SLOT(findPrev()));
    connect(findPrevAction, SIGNAL(triggered()), this, SLOT(findPrev()));
    this->addAction(findPrevAction);

    // Connect the Fit All and Highlight All functions
    connect(fitAllButton, SIGNAL(clicked()), this, SLOT(fitAll()));
    connect(highlightAll_, SIGNAL(clicked(bool)), this, SLOT(highlightAllStateChange(bool)));
  }


  // Select the current completion
  void FindDialog::selectCompletion() {
    input_->setText(completer_->currentCompletion());
    return;
  }


  // Retrieve the next completion
  void FindDialog::nextCompletion() {
    // Using completionCount() may incur speed penalties for more completions
    if (completerCurrentRow_ < (completer_->completionCount()) - 1)
      completerCurrentRow_++;

    // Apply the completion
    completer_->setCurrentRow(completerCurrentRow_);
    input_->setText(completer_->currentCompletion());
    return;
  }


  // Retrieve the previous completion
  void FindDialog::prevCompletion() {
    if (completerCurrentRow_ > 0)
      completerCurrentRow_--;

    // Apply the completion
    completer_->setCurrentRow(completerCurrentRow_);
    input_->setText(completer_->currentCompletion());
    return;
  }


  // A helper function so we can sort items chronologically and by OID
  bool sortHelper(AeraGraphicsItem* item1, AeraGraphicsItem* item2) {
    if (item1->getAeraEvent()->time_ != item2->getAeraEvent()->time_)
      return item1->getAeraEvent()->time_ < item2->getAeraEvent()->time_;
    else
      return item1->getAeraEvent()->object_->get_oid() < item2->getAeraEvent()->object_->get_oid();
  }


  // Find objects with labels that fit the input and have valid AeraGraphicsItems
  void FindDialog::updateMatches() {
    // Ignore empty inputs
    if (input_->text().isEmpty()) {
      resetState();
      setStatus("Input must not be empty");
      statusAlert(true);
      return;
    }

    // Skip the refresh if any of these conditions are true
    if (input_->text().toStdString() == lastSearch_ && !timeSteppedFlag_ && !highlightAllFlag_)
      return;

    // Get the new search term if necessary
    if (input_->text().toStdString() != lastSearch_) {
      status_->setText("");                         // Clear the status message
      searchTerm_ = input_->text().toStdString();   // Take in new input

      // Strip whitespace and update the input
      int firstCharacter = searchTerm_.find_first_not_of(" ");
      int lastCharacter = searchTerm_.find_last_not_of(" ");
      searchTerm_ = searchTerm_.substr(firstCharacter, lastCharacter - firstCharacter + 1);
      input_->setText(QString::fromStdString(searchTerm_));

      lastSearch_ = searchTerm_;  // Record this as the most recent search term
      n_ = 0;                     // Restart Find scan
      highlightAllFlag_ = true;   // This'll need to be redone
    }

    // Reset highlights
    parentWindow_->getMainScene()->unhighlightAll();
    parentWindow_->getModelsScene()->unhighlightAll();

    // Search for matching object labels
    std::vector<std::string> labels = replicodeObjects_.getObjectsByLabelSubstring(searchTerm_);

    // Filter the labels and get only the ones with valid graphics items
    matches_.clear();
    for (int i = 0; i < labels.size(); i++) {
      auto object = replicodeObjects_.getObject(labels.at(i));
      if (!object)
        continue;

      AeraVisualizerScene* scene;
      AeraGraphicsItem* item = parentWindow_->getAeraGraphicsItem(object, &scene);
      if (!item)
        continue;
      matches_.push_back(item);
    }

    // Handle no matches
    if (matches_.empty()) {
      setStatus("Cannot find \"" + QString::fromStdString(searchTerm_) + "\"");
      statusAlert(true);
      n_ = 0;
      return;
    }

    // Sort the matches
    else {
      sort(matches_.begin(), matches_.end(), sortHelper);
    }

    // If time was rolled back, go back to the most recent match that's still visible
    n_ = (std::min)(n_, (int) matches_.size() - 1);

    // Reapply "Highlight all" effect if requested
    if (highlightAllFlag_) {
      applyHighlightAll(highlightAll_->isChecked());
      highlightAllFlag_ = false;
    }

    // Clear timeSteppedFlag_
    timeSteppedFlag_ = false;

    return;
  }


  // Fit all matches in the frame (same thing as AeraVisualizerScene::zoomViewHome())
  void FindDialog::fitAll() {
    // In case something changes
    updateMatches();
    if (!matches_.empty()) {
      setStatus(QString::number(n_ + 1) + " out of " + QString::number(matches_.size()) + " matches");
      statusAlert(false);
    }

    // Get the bounding rect of all matches
    QRectF boundingRect;
    foreach(QGraphicsItem * item, matches_) {
      if (dynamic_cast<AeraGraphicsItem*>(item) && item->isVisible()) {
        if (boundingRect.width() == 0)
          boundingRect = item->sceneBoundingRect();
        else
          boundingRect = boundingRect.united(item->sceneBoundingRect());
      }
    }

    if (boundingRect.width() != 0) {
      matches_.at(0)->getParentScene()->views().at(0)->fitInView(boundingRect, Qt::KeepAspectRatio);
      matches_.at(0)->getParentScene()->updateHighlights();
    }
  }


  // Look for the next object in the list that matches the name entered
  void FindDialog::findNext() {
    n_++;             // Increment first
    updateMatches();  // Update the matches if necessary

    // Stop if there are no matches to go through
    if (matches_.empty()) {
      n_ = 0;
      return;
    }

    // Check if we've run out of (visible) matches and wrap around if applicable
    if (n_ >= matches_.size()){
      if (wraparound_->isChecked()) {
        n_ = 0;
        statusAlert(false); // Clear any alert colors
      }
      else {
        if (!matches_.empty()) {
          setStatus("Reached end of output");
          statusAlert(true);
        }
        n_ = matches_.size();
        return;
      }
    }
    else {
      statusAlert(false); // Clear any alert colors
    }

    // Get the current match
    AeraGraphicsItem* item = matches_.at(n_);
    std::string label = replicodeObjects_.getLabel(item->getAeraEvent()->object_);

    // Make sure an item was actually found
    if (!item) {
      setStatus("Could not get item for \"" + QString::fromStdString(label) + "\"");
      return;
    }

    // Deal with invisible items
    if (!item->isVisible()) {
      if (!skipHidden_->isChecked()) {
        setStatus(item->makeHtmlLink(item->getAeraEvent()->object_, replicodeObjects_) + " is invisible");

        // Unhighlight the last item
        if (item->getParentScene()->currentMatch_) {
          item->getParentScene()->currentMatch_->restorePen();
          item->getParentScene()->currentMatch_ = NULL;
          item->getParentScene()->updateHighlights();
        }
      }
      else
        findNext();
    }
    else {
      highlightMatch(item);
    }

    return;
  }


  // Look for the last object in the list that matches the name entered
  void FindDialog::findPrev() {
    n_--;             // Decrement first
    updateMatches();  // Update the matches if necessary

    // Stop if there are no matches to go through
    if (matches_.empty()) {
      n_ = 0;
      return;
    }

    // Check if we've run out of (visible) matches and wrap around if applicable
    if (n_ < 0) {
      if (wraparound_->isChecked()) {
        n_ = matches_.size() - 1;
        statusAlert(false); // Clear any alert colors
      }
      else {
        if (!matches_.empty()) {
          setStatus("Reached beginning of output");
          statusAlert(true);
        }
        n_ = 0;
        return;
      }
    }
    else {
      statusAlert(false); // Clear any alert colors
    }

    // Get the current match
    AeraGraphicsItem* item = matches_.at(n_);
    std::string label = replicodeObjects_.getLabel(item->getAeraEvent()->object_);

    // Make sure an item was actually found
    if (!item) {
      setStatus("Could not get item for \"" + QString::fromStdString(label) + "\"");
      return;
    }

    // Deal with invisible items
    if (!item->isVisible()) {
      if (!skipHidden_->isChecked()) {
        setStatus(item->makeHtmlLink(item->getAeraEvent()->object_, replicodeObjects_) + " is invisible");

        // Unhighlight the last item
        if (item->getParentScene()->currentMatch_) {
          item->getParentScene()->currentMatch_->restorePen();
          item->getParentScene()->currentMatch_ = NULL;
          item->getParentScene()->updateHighlights();
        }
      }
      else
        findPrev();
    }
    else {
      highlightMatch(item);
    }

    return;
  }


  // Update the status message
  void FindDialog::setStatus(QString message) {
    QFontMetrics metrics(status_->font());
    QString elidedMessage;

    // If there's a URL, we'll need to elide it specially
    if (message.contains("</a>")) {
      smatch regexmatches;
      std::string msg = message.toStdString();

      // Extract the tag from the message
      regex_search(msg, regexmatches, regex("<a href.*\\\">"));
      QString tag = QString::fromStdString(regexmatches[0].str());
      message.replace(tag, "");

      // Extract the name and the tail end of the message
      int nameEnd = message.indexOf("</a>", 0);
      QString name = message.left(nameEnd);
      QString tail = message.mid(nameEnd + 4, message.length());

      // Elide the name
      QString elidedName = metrics.elidedText(name, Qt::ElideRight, status_->width() - metrics.width(tail));     

      // Put it back together
      elidedMessage = tag + elidedName + "</a>" + tail;
    }

    // Normal messages can be elided simply
    else
      elidedMessage = metrics.elidedText(message, Qt::ElideRight, status_->width());
    
    // Display the message
    status_->setText(elidedMessage);
  }


  // Change colors and focus the window if there's an error/alert
  void FindDialog::statusAlert(bool alert) {
    if (alert) {
      status_->setStyleSheet(alertStyleSheet_);
      this->activateWindow();
    }
    else {
      status_->setStyleSheet(normalStyleSheet_);
    }
  }


  // Highlight the current match
  void FindDialog::highlightMatch(AeraGraphicsItem* item) {
    // Update the progress label (counting from 1 not 0)
    setStatus(QString::number(n_ + 1) + " out of " + QString::number(matches_.size()) + " matches");

    // Highlight one specific item
    if (!highlightAll_->isChecked()) {
      // Unhighlight the old item
      if (item->getParentScene()->currentMatch_)
        item->getParentScene()->currentMatch_->restorePen();

      // Highlight the new item
      item->getParentScene()->currentMatch_ = item;
      item->savePen();

    }
    else {
      item->getParentScene()->currentMatch_ = item;
    }

    // Zoom to the item if desired
    if (zoomTo_->isChecked()) {
      item->focus();
      item->getParentScene()->zoomToItem(item);
    }
    else {
      item->getParentScene()->updateHighlights();
    }
  }


  // Handle state changes on the highlightAll_ checkbox. This checks for matches first
  // just in case the highlightAll_ checkbox is activated after a time/search term change
  void FindDialog::highlightAllStateChange(bool newState) {

    // Set this flag so the highlight effect is applied in updateMatches
    highlightAllFlag_ = true;
    // Just in case time has changed or something
    updateMatches();

    // Update the status message if needed
    if (!matches_.empty())
      setStatus(QString::number(n_ + 1) + " out of " + QString::number(matches_.size()) + " matches");
  }


  // Apply the highlight all effect. This doesn't refresh matches first so it can be used anywhere
  void FindDialog::applyHighlightAll(bool highlight) {   
    // Wipe all highlights
    if (highlight == false) {
      parentWindow_->getMainScene()->unhighlightAll();
      parentWindow_->getModelsScene()->unhighlightAll();
    }

    // Save each item's pen and add it to the relevant list
    else {
      foreach(AeraGraphicsItem * item, matches_) {
        if (n_ < matches_.size() && item != matches_.at(n_))
          item->savePen();
        item->getParentScene()->allMatches_.push_back(item);
        item->borderFlashCountdown_ = 0; // Don't flash if highlighting
      }
    }

    // Reapply current highlight if needed
    if (n_ < matches_.size() && n_ >= 0)
      matches_.at(n_)->getParentScene()->currentMatch_ = matches_.at(n_);

    // Draw the highlights
    parentWindow_->getMainScene()->updateHighlights();
    parentWindow_->getModelsScene()->updateHighlights();
  }


  // Wipe highlights and search status
  void FindDialog::resetState() {
    parentWindow_->getModelsScene()->unhighlightAll();
    parentWindow_->getMainScene()->unhighlightAll();

    lastSearch_ = "";
    input_->setText("");

    matches_.clear();
    n_ = 0;
    setStatus("");
    statusAlert(false);
  }


  // Reset everything on close
  void FindDialog::reject() {
    resetState();
    QDialog::reject();
  }


  // A simple function for debugging
  void FindDialog::printMatches(QString title = "Contents of matches_") {
    QString printVector = "";

    for (int i = 0; i < matches_.size(); i++) {
      AeraGraphicsItem* item = matches_.at(i);
      std::string label = replicodeObjects_.getLabel(item->getAeraEvent()->object_);
      printVector += QString::fromStdString(std::to_string(i)) + " -> " + QString::fromStdString(label) + "\n";
    }

    QMessageBox::information(this, title, printVector);
  }
  

  // Borrow some code from AeraGraphicsItem to generate a tooltip when the user hovers over a link
  void FindDialog::hiddenLinkHovered(const QString& link) {
    if (link.startsWith("#detail_oid-")) {
      uint64 detail_oid = link.mid(12).toULongLong();
      auto object = replicodeObjects_.getObjectByDetailOid(detail_oid);
      if (object) {
        AeraGraphicsItem* aeraGraphicsItem = parentWindow_->getAeraGraphicsItem(object);
        if (aeraGraphicsItem) {
          status_->setToolTip(aeraGraphicsItem->getHtml());
          //aeraGraphicsItem->hovere
        }
      }
    }
    else {
      status_->setToolTip("");
    }
  }

  // Borrow some code to generate a present a "Zoom To, Focus, Center" menu for hidden objects
  void FindDialog::hiddenLinkActivated(const QString& link) {
    if (link.startsWith("#detail_oid-")) {
      uint64 detail_oid = link.mid(12).toULongLong();
      auto object = replicodeObjects_.getObjectByDetailOid(detail_oid);
      if (object) {
        AeraGraphicsItem* aeraGraphicsItem = parentWindow_->getAeraGraphicsItem(object);
        if (aeraGraphicsItem) {
          auto menu = new QMenu();

          menu->addAction("Zoom to This", [=]() {
            aeraGraphicsItem->getParentScene()->zoomToItem(aeraGraphicsItem);
            aeraGraphicsItem->getParentScene()->currentMatch_ = aeraGraphicsItem;
            aeraGraphicsItem->getParentScene()->updateHighlights();
          });
          menu->addAction("Focus on This", [=]() {
            aeraGraphicsItem->getParentScene()->focusOnItem(aeraGraphicsItem);
            aeraGraphicsItem->getParentScene()->currentMatch_ = aeraGraphicsItem;
            aeraGraphicsItem->getParentScene()->updateHighlights(); 
          });
          menu->addAction("Center on This", [=]() {
            aeraGraphicsItem->getParentScene()->centerOnItem(aeraGraphicsItem);
            aeraGraphicsItem->getParentScene()->currentMatch_ = aeraGraphicsItem;
            aeraGraphicsItem->getParentScene()->updateHighlights();
          });

          menu->exec(QCursor::pos() - QPoint(10, 10));
          delete menu;
        }
      }
    }
    else {
      status_->setToolTip("");
    }
  }

}
