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
