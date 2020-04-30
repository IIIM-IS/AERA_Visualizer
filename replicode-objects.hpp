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
   * \return True for success, false for error compiling.
   */
  bool init(const std::string& userOperatorsFilePath, const std::string& decompiledFilePath);

  /**
   * Get the time reference that was loaded from the decompiled output. 
   * This can be used to show times relative to the beginning of the Replicode session.
   * \return The time reference.
   */
  core::Timestamp getTimeReference() const { return timeReference_; }

private:
  core::Timestamp timeReference_;
  r_code::list<P<Code> > objects_;
};

#endif
