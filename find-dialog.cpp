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
    QPushButton* findNext = new QPushButton("Find Next", this);
    QPushButton* findPrev = new QPushButton("Find Prev", this);
    status_ = new QLabel(this);

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
    buttonsLayout->addWidget(findNext);
    buttonsLayout->addWidget(findPrev);
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
    connect(findNext, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(findNextAction, SIGNAL(triggered()), this, SLOT(findNext()));
    this->addAction(findNextAction);

    // Connect the Find Prev button and action
    QAction* findPrevAction = new QAction(tr("Find Prev"), this);
    findPrevAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
    connect(findPrev, SIGNAL(clicked()), this, SLOT(findPrev()));
    connect(findPrevAction, SIGNAL(triggered()), this, SLOT(findPrev()));
    this->addAction(findPrevAction);
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
    // Check if the search term has changed
    if (input_->text().toStdString() != lastSearch_) {
      status_->setText("");                         // Clear the status message
      searchTerm_ = input_->text().toStdString();   // Take in new input

      // Ignore empty inputs
      if (searchTerm_ == "")
        return;

      // Strip whitespace and update the input
      int firstCharacter = searchTerm_.find_first_not_of(" ");
      int lastCharacter = searchTerm_.find_last_not_of(" ");
      searchTerm_ = searchTerm_.substr(firstCharacter, lastCharacter - firstCharacter + 1);
      input_->setText(QString::fromStdString(searchTerm_));

      lastSearch_ = searchTerm_;  // Record this as the most recent search term
      n_ = 0;                     // Restart Find scan
    }

    // If the search term and time are the same, don't update
    else if (parentWindow_->getFrameMaxTime() == lastMaxTime_) {
      return;
    }

    // Record this so we can check if the time changed
    lastMaxTime_ = parentWindow_->getFrameMaxTime();

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
      setStatus("Cannot find \"" + searchTerm_ + "\"", true);
      return;
    }

    // Sort items and return
    sort(matches_.begin(), matches_.end(), sortHelper);
    return;
  }


  void FindDialog::highlightMatch(AeraGraphicsItem* item) {
    // Update the progress label (counting from 1 not 0)
    setStatus(std::to_string(n_ + 1) + " out of " + std::to_string(matches_.size()) + " matches");

    // Flash found items, set the flash timer to twice as long to so it's easier to see
    item->borderFlashCountdown_ = AeraVisualizerScene::FLASH_COUNT * 2;
    item->getParentScene()->establishFlashTimer();

    // Zoom to the item if desired
    if (zoomTo_->isChecked()) {
      item->focus();
      item->getParentScene()->zoomToItem(item);
    }
  }


  // Look for the next object in the list that matches the name entered
  void FindDialog::findNext() {
    n_++;             // Go to the next match 
    updateMatches();  // Update the matches if necessary

    // Check if we've run out of (visible) matches and wrap around if applicable
    if (n_ >= matches_.size()){
      if (wraparound_->isChecked()) {
        n_ = 0;
        setStatus(status_->text().toStdString()); // Clear any alert colors
      }
      else {
        setStatus("Reached end of output", true);
        n_ = matches_.size();
        return;
      }
    }
    else {
      setStatus(status_->text().toStdString()); // Clear any alert colors
    }

    // Get the current match
    AeraGraphicsItem* item = matches_.at(n_);
    std::string label = replicodeObjects_.getLabel(item->getAeraEvent()->object_);

    // Make sure an item was actually found
    if (!item) {
      setStatus("Could not get item for \"" + label + "\"", true);
      return;
    }

    // Deal with invisible items
    if (!item->isVisible()) {
      if (!skipHidden_->isChecked())
        setStatus("\"" + label + "\" is invisible", true);
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
    n_--;             // Go to the last match
    updateMatches();  // Update the matches if necessary

    // Check if we've run out of (visible) matches and wrap around if applicable
    if (n_ < 0) {
      if (wraparound_->isChecked()) {
        n_ = matches_.size() - 1;
        setStatus(status_->text().toStdString()); // Clear any alert colors
      }
      else {
        setStatus("Reached beginning of output", true);
        n_ = -1;
        return;
      }
    }
    else {
      setStatus(status_->text().toStdString()); // Clear any alert colors
    }

    // Get the current match
    AeraGraphicsItem* item = matches_.at(n_);
    std::string label = replicodeObjects_.getLabel(item->getAeraEvent()->object_);

    // Make sure an item was actually found
    if (!item) {
      setStatus("Could not get item for \"" + label + "\"", true);
      return;
    }

    // Deal with invisible items
    if (!item->isVisible()) {
      if (!skipHidden_->isChecked())
        setStatus("\"" + label + "\" is invisible", true);
      else
        findPrev();
    }
    else {
      highlightMatch(item);
    }

    return;
  }


  // Update the status message
  void FindDialog::setStatus(std::string message, bool alert) {
    // Trim the label if it's too long (it's not a perfect solution but it works)
    if (message.length() > 31) {
      message = message.substr(0, 31);
      message.append("...");
    }

    // Display the message
    status_->setText(QString::fromStdString(message));

    // Change colors and focus the window if there's an error/alert
    if (alert) {
      status_->setStyleSheet("font-style: italic; color: red;");
      this->activateWindow();
    }
    else {
      status_->setStyleSheet("font-style: italic; color: dark-grey;");
    }
    return;
  }
}
