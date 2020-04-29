#ifndef REPLICODE_OBJECTS_HPP
#define REPLICODE_OBJECTS_HPP

#include <string>
#include <map>
#include "submodules/replicode/r_exec/mem.h"

/// <summary>
/// ReplicodeObjects holds a list of Replicode objects that are compiled from the
/// decompiler output.
/// </summary>
class ReplicodeObjects {
public:
  /// <summary>
  /// Compile and load the metadata from the user operators file, then compile
  /// the decompiled file and set up the list of Replicode objects. This
  /// processes the decompile file to get the time reference and original OIDs.
  /// </summary>
  /// <param name="userOperatorsFilePath">The user operators file path, usually ending
  /// in "user.classes.replicode".</param>
  /// <param name="decompiledFilePath">The decompiled output, usually ending in
  /// "decompiled_objects.txt".</param>
  /// <returns>True for success, false for error compiling.</returns>
  bool init(const std::string& userOperatorsFilePath, const std::string& decompiledFilePath);

  /// <summary>
  /// Get the time reference that was loaded from the decompiled output. 
  /// This can be used to show times relative to the beginning of the Replicode session.
  /// </summary>
  /// <returns>The time reference.</returns>
  core::Timestamp getTimeReference() const { return timeReference_; }

private:
  core::Timestamp timeReference_;
  r_code::list<P<Code> > objects_;
};

#endif
