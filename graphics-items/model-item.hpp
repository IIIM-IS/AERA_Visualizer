//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2025 Jeff Thompson
//_/_/ Copyright (c) 2018-2025 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2025 Icelandic Institute for Intelligent Machines
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

#ifndef MODEL_ITEM_HPP
#define MODEL_ITEM_HPP

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPen>
#include "aera-graphics-item.hpp"

namespace aera_visualizer {

class AeraVisualizerScene;

class ModelItem : public AeraGraphicsItem
{
public:
  ModelItem(
    NewModelEvent* newModelEvent, ReplicodeObjects& replicodeObjects, AeraVisualizerScene* parent);

  void updateFromModel();

  void setStrengthColor(QString color)
  {
    strengthColor_ = color;
    refreshText();
  };

  void setEvidenceCountColor(QString color) 
  { 
    evidenceCountColor_ = color;
    refreshText();
  };

  void setSuccessRateColor(QString color)
  {
    successRateColor_ = color;
    refreshText();
  };

  /**
   * Start with the source from replicodeObjects_.getSourceCode for a model, and
   * remove the set of output groups and parameters, and remove trailing wildcards, and
   * replace a template variable with a wildcard if it is also assigned.
   * \param modelSource The source from replicodeObjects_.getSourceCode.
   * \return The simplified source code
   */
  static QString simplifyModelSource(const std::string& modelSource);

  /**
   * Assume the html is a model with \n line endings, and highlight the left-hand-side
   * and right-hand-side expressions with red and green background.s
   * \param html The model HTML string to modify.
   */
  static void highlightLhsAndRhs(QString& html);

  /**
   * Modify the HTML string to change the font color of variables.
   * \param html The HTML string to modify.
   */
  static void highlightVariables(QString& html);

  /**
   * Check that the timings are variables and return the variable indexes.
   * \param fact The Fact with the timings.
   * \param iAfterVariable Set this to the index of the after variable. If this
   * method returns false, the value is undetermined.
   * \param iAfterVariable Set this to the index of the before variable. If this
   * method returns false, the value is undetermined.
   * \return True for success, false if fact is not a Fact, or if the timings are
   * not variables.
   */
  static bool getTimingVariables(r_code::Code* fact, int& iAfterVariable, int& iBeforeVariable);

  int strengthFlashCountdown_;
  bool strengthIncreased_;
  int evidenceCountFlashCountdown_;
  bool evidenceCountIncreased_;
  int successRateFlashCountdown_;
  bool successRateIncreased_;

private:
  QString makeHtml();

  /**
   * Set textItem_ to headerHtml_ + makeHtml().
   */
  void refreshText()
  {
    auto html = makeHtml();
    textItem_->setHtml(html);
    // adjustSize() is needed for right-aligned text.
    textItem_->adjustSize();
    textItem_->setHtml(headerHtml_ + html);
    textItem_->adjustSize();
  }

  NewModelEvent* newModelEvent_;
  QString sourceCodeHtml_;
  core::float32 strength_;
  QString strengthColor_;
  core::float32 evidenceCount_;
  QString evidenceCountColor_;
  core::float32 successRate_;
  QString successRateColor_;
};

}

#endif
