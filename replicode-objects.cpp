#include <fstream>
#include <sstream>
#include <regex>
#include "submodules/replicode/r_comp/preprocessor.h"
#include "submodules/replicode/r_comp/compiler.h"
#include "submodules/replicode/r_exec/model_base.h"
#include "replicode-objects.hpp"

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;
using namespace r_comp;

string ReplicodeObjects::init(const string& userOperatorsFilePath, const string& decompiledFilePath)
{
  // Run the proprocessor on the user operators (which includes std.replicode) just to
  // get the Metadata. The objects are repeated in the decompiled output.
  ifstream userOperatorsFile(userOperatorsFilePath);
  Metadata metadata;
  Preprocessor preprocessor;
  string error;
  // We won't compile the preprocessed user operators code.
  ostringstream dummyPreprocessedUserOperators;
  if (!preprocessor.process(
      &userOperatorsFile, userOperatorsFilePath, &dummyPreprocessedUserOperators, error, &metadata))
    return false;
  dummyPreprocessedUserOperators.clear();

  r_exec::InitOpcodes(metadata);
  // Now() is called when construcing model controllers.
  r_exec::Now = Time::Get;

  // Preprocess the decompiler output to remove extra info and get the OIDs.
  map<string, uint32> objectOids;
  map<string, uint32> objectDebugOids;
  ifstream rawDecompiledFile(decompiledFilePath);
  regex timeReferenceRegex("^> DECOMPILATION. TimeReference (\\d+)s:(\\d+)ms:(\\d+)us");
  regex debugOidRegex("^\\((\\d+)\\) ([\\w\\.]+):(.+)$");
  regex oidAndDebugOidRegex("^(\\d+)\\((\\d+)\\) (\\w+):(.+)$");

  ostringstream decompiledOut;
  string line;
  while (getline(rawDecompiledFile, line)) {
    smatch matches;

    if (regex_search(line, matches, timeReferenceRegex)) {
      microseconds us(1000000 * stoll(matches[1].str()) +
                         1000 * stoll(matches[2].str()) +
                                stoll(matches[3].str()));
      timeReference_ = Timestamp(us);

      // Make the line blank.
      decompiledOut << endl;
    }
    else if (line.size() > 0 && line[0] == '>')
      // Skip other decompiler messages starting with '>'.
      decompiledOut << endl;
    else if (regex_search(line, matches, debugOidRegex)) {
      auto debugOid = stoul(matches[1].str());
      auto name = matches[2].str();
      objectOids[name] = UNDEFINED_OID;
      objectDebugOids[name] = debugOid;

      // Use the line without the OID.
      decompiledOut << name << ':' << matches[3].str() << endl;
    }
    else if (regex_search(line, matches, oidAndDebugOidRegex)) {
      auto oid = stoul(matches[1].str());
      auto debugOid = stoul(matches[2].str());
      auto name = matches[3].str();
      objectOids[name] = oid;
      objectDebugOids[name] = debugOid;

      // Use the line without the OID.
      decompiledOut << name << ':' << matches[4].str() << endl;
    }
    else
      // Use the line as-is.
      decompiledOut << line << endl;
  }

  // Preprocess and complile the processed decompiler output, using the metadata we got above.
  istringstream decompiledIn(decompiledOut.str());
  ostringstream preprocessedOut;
  if (!preprocessor.process(
      &decompiledIn, decompiledFilePath, &preprocessedOut, error, NULL))
    return error;

  auto preprocessedOutString = preprocessedOut.str();
  istringstream preprocessedIn(preprocessedOutString);
  Compiler compiler(true);
  r_comp::Image image;
  if (!compiler.compile(&preprocessedIn, &image, &metadata, error, false))
    return error;

  // Get the the objects from the compiled image.
  r_code::vector<Code*> imageObjects;
  // tempMem is only used for calling build_object.
  r_exec::Mem<r_exec::LObject, r_exec::MemStatic> tempMem;
  image.get_objects(&tempMem, imageObjects);

  // Set the OIDs based on the decompiled output.
  for (auto i = 0; i < imageObjects.size(); ++i) {
    string name = compiler.getObjectName(i);
    if (name != "") {
      auto oidEntry = objectOids.find(name);
      if (oidEntry != objectOids.end())
        imageObjects[i]->set_oid(oidEntry->second);

      auto debugOidEntry = objectDebugOids.find(name);
      if (debugOidEntry != objectDebugOids.end())
        imageObjects[i]->set_debug_oid(debugOidEntry->second);
    }
  }
  
  // Transfer imageObjects to objects_, processing as needed.
  // Imitate _Mem::load.
  for (uint32 i = 0; i < imageObjects.size(); ++i) {
    Code* object = imageObjects[i];
    int32 location;
    objects_.push_back(object, location);
    object->set_strorage_index(location);

    switch (object->code(0).getDescriptor()) {
    case Atom::MODEL:
      r_exec::_Mem::unpack_hlp(object);
      r_exec::ModelBase::Get()->load(object);
      break;
    case Atom::COMPOSITE_STATE:
      r_exec::_Mem::unpack_hlp(object);
      break;
    case Atom::INSTANTIATED_PROGRAM: // refine the opcode depending on the inputs and the program type.
      if (object->get_reference(0)->code(0).asOpcode() == r_exec::Opcodes::Pgm) {

        if (object->get_reference(0)->code(object->get_reference(0)->code(PGM_INPUTS).asIndex()).getAtomCount() == 0)
          object->code(0) = Atom::InstantiatedInputLessProgram(object->code(0).asOpcode(), object->code(0).getAtomCount());
      }
      else
        object->code(0) = Atom::InstantiatedAntiProgram(object->code(0).asOpcode(), object->code(0).getAtomCount());
      break;
    }

    for (auto v = object->views_.begin(); v != object->views_.end(); ++v) {

      // init hosts' member_set.
      r_exec::View* view = (r_exec::View*) * v;
      view->set_object(object);
      r_exec::Group* host = view->get_host();

#if 0 // debug: host is NULL.
      if (!host->load(view, object))
        return false;
#endif
    }
  }

  r_exec::_Mem::init_timings(timeReference_, objects_);

#if 0 // Test re-decompiling.
#include "submodules/replicode/r_comp/decompiler.h"
  Decompiler debugDecompiler;
  debugDecompiler.init(&metadata);
  r_comp::Image debugImage;
  debugImage.timestamp_ = Time::Get();
  debugImage.add_objects(objects_);
  debugImage.object_names_ = image.object_names_;
  std::ostringstream decompiled_code;
  debugDecompiler.decompile(&debugImage, &decompiled_code, timeReference_.time_since_epoch(), false);
  ofstream debugOutFile("c:\\temp\\decompiled.txt");
  debugOutFile << decompiled_code.str() << endl;
#endif

  return "";
}

r_code::Code* ReplicodeObjects::getObject(uint32 oid)
{
  for (auto i = objects_.begin(); i != objects_.end(); ++i) {
    if ((*i)->get_oid() == oid)
      return *i;
  }

  return NULL;
}
