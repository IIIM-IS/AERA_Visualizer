#ifndef REPLICODE_OBJECTS_HPP
#define REPLICODE_OBJECTS_HPP

#include <string>
#include <map>
#include "submodules/replicode/r_exec/mem.h"

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
   * \param userOperatorsFilePath The user operators file path, usually ending
   * in "user.classes.replicode".
   * \param decompiledFilePath The decompiled output, usually ending in
   * "decompiled_objects.txt".
   * \return An empty string for success, otherwise an error string.
   */
  std::string init(const std::string& userOperatorsFilePath, const std::string& decompiledFilePath);

  /**
   * Get the time reference that was loaded from the decompiled output. 
   * This can be used to show times relative to the beginning of the Replicode session.
   * \return The time reference.
   */
  core::Timestamp getTimeReference() const { return timeReference_; }

  /**
   * Get the object by OID.
   * \param oid The OID.
   * \return The object, or NULL if not found.
   */
  r_code::Code* getObject(uint32 oid);

  /**
   * Get the object source code (from the decompiled objects file).
   * \param object The object.
   * \return The source code, or "" if not found. This does not have the label or view set.
   */
  std::string getSourceCode(r_code::Code* object)
  {
    auto result = objectSourceCode_.find(object);
    if (result == objectSourceCode_.end())
      return "";
    return result->second;
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
  r_code::list<P<r_code::Code> > objects_;
};

#endif
