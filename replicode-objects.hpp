//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2022 Jeff Thompson
//_/_/ Copyright (c) 2018-2022 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2022 Icelandic Institute for Intelligent Machines
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

#ifndef REPLICODE_OBJECTS_HPP
#define REPLICODE_OBJECTS_HPP

#include <string>
#include <map>
#include <regex>
#include <QString>
#include "submodules/AERA/r_exec/mem.h"

class QProgressDialog;

namespace aera_visualizer {

/**
 * ReplicodeObjects holds a list of Replicode objects that are compiled from the
 * decompiler output.
 */
class ReplicodeObjects {
public:
  ReplicodeObjects();

  /**
   * Compile and load the metadata from the user operators file, then compile
   * the decompiled file and set up the list of Replicode objects. This
   * processes the decompile file to get the time reference and original OIDs.
   * \param userClassesFilePath The user operators file path, usually ending
   * in "user.classes.replicode".
   * \param decompiledFilePath The decompiled output, usually ending in
   * "decompiled_objects.txt".
   * \param basePeriod The base_period from settings.xml, used for getSamplinePeriod().
   * \param progress The progress dialog where you can call setLabelText, setMaximum and setValue. You should
   * periodically call QApplication::processEvents(). You can call wasCanceled and quit if true.
   * \return An empty string for success, otherwise an error string. If the string is "cancel" then
   * the user clicked Cancel in the progress dialog.
   */
  std::string init(const std::string& userClassesFilePath, const std::string& decompiledFilePath,
    std::chrono::microseconds basePeriod, QProgressDialog& progress);

  /**
   * Get the sampling period, which is 2 * base_period from settings.xml. This should
   * match sampling_period in user.classes.replicode. This method follows Mem::get_sampling_period().
   */
  std::chrono::microseconds getSamplingPeriod() const { return 2 * basePeriod_; }

  /**
   * Get the time reference that was loaded from the decompiled output.
   * This can be used to show times relative to the beginning of the Replicode session.
   * \return The time reference.
   */
  core::Timestamp getTimeReference() const { return timeReference_; }

  /**
   * Get the object by the OID.
   * \param oid The OID.
   * \return The object, or NULL if not found.
   */
  r_code::Code* getObject(uint32 oid) const;

  /**
   * Get the object by the detail OID.
   * \param detailOid The detail OID.
   * \return The object, or NULL if not found.
   */
  r_code::Code* getObjectByDetailOid(uint64 detailOid) const;

  /**
   * Get the object's label (from the decompiled objects file).
   * \param object The object.
   * \return The label, or "" if not found.
   */
  std::string getLabel(r_code::Code* object) const
  {
    auto result = objectLabel_.find(object);
    if (result == objectLabel_.end())
      return "";
    return result->second;
  }

  /**
   * Get the object by its label (from the decompiled objects file).
   * \param label The label.
   * \return The object, or NULL if not found.
   */
  r_code::Code* getObject(const std::string& label) const
  {
    auto result = labelObject_.find(label);
    if (result == labelObject_.end())
      return NULL;
    return result->second;
  }

  /**
   * Get the object source code (from the decompiled objects file).
   * \param object The object.
   * \return The source code, or "" if not found. This does not have the label or view set.
   */
  std::string getSourceCode(r_code::Code* object) const
  {
    auto result = objectSourceCode_.find(object);
    if (result == objectSourceCode_.end())
      return "";
    return result->second;
  }

  /**
   * Similar to r_code::Utils::RelativeTime, use Time::ToString_seconds to show the
   * relative time from the time reference (but use getTimeReference() from this object, not
   * the global value in r_exec).
   * \param timestamp The timestamp.
   * \return A string representing the relative time.
   */
  std::string relativeTime(Timestamp timestamp) const
  {
    return core::Time::ToString_seconds(timestamp - timeReference_);
  }

  /**
   * If message is in progressMessages_, then show all the progressMessages_ with this message highlighted.
   * Otherwise, just show this message.
   * \param message The progress message for matching an entry in progressMessages_.
   * \return The text for progress.setLabelText.
   */
  QString getProgressLabelText(const QString& message);

  /**
   * Parse the list of integers oids and use getObject() to add each to objects.
   * \param oids The string with the list of OID integers, e.g. "12 14". This may be "".
   * \param objects Add found objects. This does not first clear the vector.
   * \return True for success, false if getObject() failed to find an OID
   */
  bool getObjects(std::string oids, std::vector<r_code::Code*>& objects);

private:
  /**
   * Process the decompiled objects file to remove OIDs, detail OIDs and info lines starting with ">".
   * This sets timeReference_ from the header info line. This gets the object's source code, which is
   * stripped of the label and view set.
   * The source code does not have an ending newline, even if it is multi-line code.
   * \param decompiledFilePath The path of the decompiled objects file.
   * \param objectOids Fill this map of label to OID. This first clears the map.
   * \param objectDetailOids Fill this map of label to detail OID. This first clears the map.
   * \return A string of the decompiled objects file with removed OIDs and detail OIDs.
   */
  std::string processDecompiledObjects(
    std::string decompiledFilePath, std::map<std::string, core::uint32>& objectOids,
    std::map<std::string, core::uint64>& objectDetailOids);

  std::chrono::microseconds basePeriod_;
  core::Timestamp timeReference_;
  // Key is the Code* object, value is the source code from the decompiled objects.
  std::map<r_code::Code*, std::string> objectSourceCode_;
  // Key is the Code* object, value is the label from the decompiled objects.
  std::map<r_code::Code*, std::string> objectLabel_;
  // Key is the label from the decompiled objects, value is the Code* object.
  std::map<std::string, r_code::Code*> labelObject_;
  r_code::list<P<r_code::Code> > objects_;
  std::vector<QString> progressMessages_;
  std::regex intMemberRegex_;
};

}

#endif
