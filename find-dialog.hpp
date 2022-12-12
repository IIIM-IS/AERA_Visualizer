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

#pragma once

#ifndef FIND_WINDOW_HPP
#define FIND_WINDOW_HPP

#include <regex>
#include "aera-visualizer-window.hpp"

#include <QtWidgets>
#include <qdialog.h>


namespace aera_visualizer {

/**
 * FindDialog extends QDialog to provide a separate modeless interface
 * for the ZoomTo/Find Next function.
 */
class FindDialog : public QDialog
{
  Q_OBJECT

  public:
    /*
    * Instantiates a FindDialog with a reference to its parent. It'll
    * call the parent's methods when zooming in on particular items.
    * \param parentWindow The main window of the visualizer
    * \param replicodeObjects All decompiled objects
    */
    FindDialog(AeraVisualizerWindow* parentWindow, ReplicodeObjects& replicodeObjects);

    // The stepEvent and unstepEvent functions in AeraVisualizerWindow use this to notify the
    // find dialog that it should refresh its matches (don't put much here since it's called A LOT)
    void reportStepEvent() {
      timeSteppedFlag_ = true;
      highlightAllFlag_ = true;
    }

  public slots:
    void findNext();
    void findPrev();
    void fitAll();

  private slots:
    void selectCompletion();
    void nextCompletion();
    void prevCompletion();
    void highlightAllStateChange(bool newState);

  private:
    // Autocompleter wordlist (just a static list of built-in objects for now)
    QStringList wordList_ = {
      "fact_", "anti_fact_", "mdl_", "imdl_", "model_", "view_",
      "ptn_", "anti_ptn_", "iptn_", "pgm_", "anti_pgm_", "ipgm_",
      "icpp_pgm_", "grp_", "cst_", "icst_", "pred_", "goal_",
      "success_", "cmd_", "icmd_", "dev_", "nod_", "perf_",
      "S", "M", "G", "mk."
    };

    // Used to update the status message
    void setStatus(std::string message, bool alert = false);

    // Get a new list of matches when there's a change in search term or visible objects
    void updateMatches();

    // Helper functions for buttons
    void highlightMatch(AeraGraphicsItem* item);

    // Apply the highlight all effect. This is separate from the state changed handler
    // so it can be used by other functions without needing to first update the matches
    void applyHighlightAll(bool highlight);

    // Use this to wipe highlights and search terms
    void resetState();

    // Catch this so we can unhighlight everything before closing the dialog
    void reject();

    // A debugging function for inspecting the matches_ vector
    void printMatches(QString title);

    // References to the main window
    AeraVisualizerWindow* parentWindow_;
    ReplicodeObjects replicodeObjects_;

    // UI Elements
    QLineEdit* input_;
    QCompleter* completer_;
    QCheckBox* wraparound_;
    QCheckBox* skipHidden_;
    QCheckBox* zoomTo_;
    QCheckBox* highlightAll_;
    QLabel* status_;

    // Used to track autocompleter selection
    int completerCurrentRow_ = 0;
    
    // Used in findNext
    std::string lastSearch_;
    std::string searchTerm_;
    std::vector<AeraGraphicsItem*> matches_;
    int n_ = 0;

    // Flags used by updateMatches
    bool timeSteppedFlag_;    // Set this when any time change has ocurred
    bool highlightAllFlag_;   // Set this when any refresh of the highlights needed
};
}

#endif
