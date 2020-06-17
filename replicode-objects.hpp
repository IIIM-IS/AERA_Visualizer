//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA VISUALIZER
//_/_/
//_/_/ Copyright(c)2020 Icelandic Institute for Intelligent Machines ses
//_/_/ Vitvelastofnun Islands ses, kt. 571209-0390
//_/_/ Author: Jeffrey Thompson <jeff@iiim.is>
//_/_/
//_/_/ -----------------------------------------------------------------------
//_/_/ Released under Open-Source BSD License with CADIA Clause v 1.0
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/   and collaboration notice, this list of conditions and the 
//_/_/   following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/   notice, this list of conditions and the following
//_/_/   disclaimer in the documentation and/or other materials provided 
//_/_/   with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/   contributors may be used to endorse or promote products 
//_/_/   derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause v 1.0: The license granted in and to the software under 
//_/_/   this agreement is a limited-use license. The software may not be used
//_/_/   in furtherance of: 
//_/_/   (i) intentionally causing bodily injury or severe emotional distress 
//_/_/   to any person; 
//_/_/   (ii) invading the personal privacy or violating the human rights of 
//_/_/   any person; or 
//_/_/   (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef REPLICODE_OBJECTS_HPP
#define REPLICODE_OBJECTS_HPP

#include <string>
#include <map>
#include "submodules/replicode/r_exec/mem.h"

namespace aera_visualizer {

/**
 * ReplicodeObjects holds a list of Replicode objects that are compiled from the
 * decompiler output.
 */
class ReplicodeObjects {
public:
  /**
   * Compile and load the metadata from the user operators file, then compile
   * the decompiled file and set up the list of Replicode objects. This
   * processes the decompile file to get the time reference and original OIDs.
   * \param userClassesFilePath The user operators file path, usually ending
   * in "user.classes.replicode".
   * \param decompiledFilePath The decompiled output, usually ending in
   * "decompiled_objects.txt".
   * \return An empty string for success, otherwise an error string.
   */
  std::string init(const std::string& userClassesFilePath, const std::string& decompiledFilePath);

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
   * Get the object by the debug OID.
   * \param debugOid The debug OID.
   * \return The object, or NULL if not found.
   */
  r_code::Code* getObjectByDebugOid(uint64 debugOid) const;

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

private:
  /**
   * Process the decompiled objects file to remove OIDs, debug OIDs and info lines starting with ">".
   * This sets timeReference_ from the header info line. This gets the object's source code, which is
   * stripped of the label and view set.
   * The source code does not have an ending newline, even if it is multi-line code.
   * \param decompiledFilePath The path of the decompiled objects file.
   * \param objectOids Fill this map of label to OID. This first clears the map.
   * \param objectDebugOids Fill this map of label to debug OID. This first clears the map.
   * \param objectSourceCode Fill this map of debugOID to source code. This will be stored in
   * objectSourceCode_ once we have the Code* for the object. This first clears the map.
   * \return A string of the decompiled objects file with removed OIDs and debug OIDs.
   */
  std::string processDecompiledObjects(
    std::string decompiledFilePath, std::map<std::string, core::uint32>& objectOids,
    std::map<std::string, core::uint64>& objectDebugOids, std::map<uint64,
    std::string>& objectSourceCode);

  core::Timestamp timeReference_;
  // Key is the Code* object, value is the source code from the decompiled objects.
  std::map<r_code::Code*, std::string> objectSourceCode_;
  // Key is the Code* object, value is the label from the decompiled objects.
  std::map<r_code::Code*, std::string> objectLabel_;
  // Key is the label from the decompiled objects, value is the Code* object.
  std::map<std::string, r_code::Code*> labelObject_;
  r_code::list<P<r_code::Code> > objects_;
};

}

#endif
