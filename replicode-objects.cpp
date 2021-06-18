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

#include <fstream>
#include <sstream>
#include <algorithm>
#include "submodules/replicode/r_comp/preprocessor.h"
#include "submodules/replicode/r_comp/compiler.h"
#include "submodules/replicode/r_comp/decompiler.h"
#include "submodules/replicode/r_exec/model_base.h"
#include "replicode-objects.hpp"

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;
using namespace r_comp;
using namespace r_exec;

namespace aera_visualizer {

string ReplicodeObjects::init(const string& userClassesFilePath, const string& decompiledFilePath,
    microseconds basePeriod)
{
  basePeriod_ = basePeriod;

  // Run the proprocessor on the user operators (which includes std.replicode) just to
  // get the Metadata. The objects are repeated in the decompiled output.
  ifstream userClassesFile(userClassesFilePath);
  if (!userClassesFile)
    return "Can't open user classes file: " + userClassesFilePath;
  r_comp::Metadata metadata;
  Preprocessor preprocessor;
  string error;
  // We won't compile the preprocessed user operators code.
  ostringstream dummyPreprocessedUserClasses;
  if (!preprocessor.process(
      &userClassesFile, userClassesFilePath, &dummyPreprocessedUserClasses, error, &metadata))
    return error;
  dummyPreprocessedUserClasses.clear();

  r_exec::InitOpcodes(metadata);
  // Now() is called when construcing model controllers.
  r_exec::Now = Time::Get;

  map<string, uint32> objectOids;
  map<string, uint64> objectDetailOids;

  {
    ifstream testOpen(decompiledFilePath);
    if (!testOpen)
      return "Can't open decompiled objects file: " + decompiledFilePath;
  }
  auto decompiledOut = processDecompiledObjects(decompiledFilePath, objectOids, objectDetailOids);

  // Preprocess and compile the processed decompiler output, using the metadata we got above.
  istringstream decompiledIn(decompiledOut);
  ostringstream preprocessedOut;
  if (!preprocessor.process(
      &decompiledIn, decompiledFilePath, &preprocessedOut, error, NULL))
    return error;

  istringstream preprocessedIn(preprocessedOut.str());
  Compiler compiler(true);
  r_comp::Image image;
  if (!compiler.compile(&preprocessedIn, &image, &metadata, error, false)) {
    auto iError = (size_t)preprocessedIn.tellg();
    auto nBeforeError = min(iError, 50);
    auto nAfterError = min(preprocessedIn.str().size() - iError, 50);
    string codeBefore = preprocessedIn.str().substr(iError - nBeforeError, nBeforeError);
    string codeAfter = preprocessedIn.str().substr(iError, nBeforeError);
    return codeBefore + "\n<< " + error + "\n" + codeAfter;
  }

  // Transfer objects from the compiler image to imageObjects.
  r_code::vector<Code*> imageObjects;
  // tempMem is only used internally for calling build_object.
  r_exec::Mem<r_exec::LObject, r_exec::MemStatic> tempMem;
  image.get_objects(&tempMem, imageObjects);

  // Set the OIDs and detail OIDs of objects in imageObjects based on the decompiled output.
  // Set up objectLabel_ and labelObject_ based on the object in imageObjects.
  for (auto i = 0; i < imageObjects.size(); ++i) {
    string label = compiler.getObjectName(i);
    if (label != "") {
      objectLabel_[imageObjects[i]] = label;
      labelObject_[label] = imageObjects[i];

      auto oidEntry = objectOids.find(label);
      if (oidEntry != objectOids.end())
        imageObjects[i]->set_oid(oidEntry->second);

      auto detailOidEntry = objectDetailOids.find(label);
      if (detailOidEntry != objectDetailOids.end())
        imageObjects[i]->set_detail_oid(detailOidEntry->second);
    }
  }

  // Transfer imageObjects to objects_, unpacking and processing as needed.
  // Imitate _Mem::load.
  for (uint32 i = 0; i < imageObjects.size(); ++i) {
    Code* object = imageObjects[i];
    int32 dummyLocation;
    objects_.push_back(object, dummyLocation);
    // We don't need to delete, so don't set the storage index.

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
      r_exec::View* view = (r_exec::View*)*v;
      view->set_object(object);
      r_exec::Group* host = view->get_host();

#if 0 // debug: host is NULL.
      if (!host->load(view, object))
        return false;
#endif
    }
  }

  r_exec::_Mem::init_timings(timeReference_, objects_);

  // We have to get the source code by decompiling the packet objects in objects_ (not from
  // the original decompiled code in decompiledFilePath) because variable names can be different.
  r_comp::Image packedImage;
  packedImage.object_names_.symbols_ = image.object_names_.symbols_;
  packedImage.add_objects(objects_, true);

  Decompiler decompiler;
  decompiler.init(&metadata);

  // Fill the objectNames map from the image and use it in decompile_references.
  UNORDERED_MAP<uint16, std::string> objectNames;
  for (auto i = 0; i < packedImage.code_segment_.objects_.size(); ++i)
    objectNames[i] = compiler.getObjectName(i);
  decompiler.decompile_references(&packedImage, &objectNames);

  Timestamp::duration timeOffset = duration_cast<microseconds>(timeReference_.time_since_epoch());
  for (uint16 i = 0; i < packedImage.code_segment_.objects_.size(); ++i) {
    auto object = getObjectByDetailOid(packedImage.code_segment_.objects_[i]->detail_oid_);
    if (object) {
      std::ostringstream decompiledCode;
      decompiler.decompile_object(i, &decompiledCode, timeOffset, false, false, false);
      auto source = decompiledCode.str();

      // Strip ending newlines.
      while (source[source.size() - 1] == '\n')
        source = source.substr(0, source.size() - 1);
      objectSourceCode_[object] = source;
    }
  }

  return "";
}

string ReplicodeObjects::processDecompiledObjects(
  string decompiledFilePath, map<string, uint32>& objectOids, map<string, uint64>& objectDetailOids)
{
  objectOids.clear();
  objectDetailOids.clear();

  ifstream rawDecompiledFile(decompiledFilePath);
  regex blankLineRegex("^\\s*$");
  regex timeReferenceRegex("^> DECOMPILATION. TimeReference (\\d+)s:(\\d+)ms:(\\d+)us");
  regex detailOidRegex("^\\((\\d+)\\) ([\\w\\.]+)(:)(.+)$");
  regex oidAndDetailOidRegex("^(\\d+)\\((\\d+)\\) (\\w+)(:)(.+)$");

  // Scan the input and fill decompiledOut.
  uint64 currentDetailOid = 0;
  ostringstream decompiledOut;
  string line;
  while (getline(rawDecompiledFile, line)) {
    smatch matches;

    if (regex_search(line, matches, blankLineRegex))
      // Skip blank lines.
      decompiledOut << endl;
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
    else if (regex_search(line, matches, detailOidRegex)) {
      auto detailOid = stoull(matches[1].str());
      auto name = matches[2].str();
      auto sourceCodeStart = matches[4].str();
      objectOids[name] = UNDEFINED_OID;
      objectDetailOids[name] = detailOid;

      // Use the line without the OID.
      decompiledOut << name << ':' << sourceCodeStart << endl;

      // We are starting a new object.
      currentDetailOid = detailOid;
    }
    else if (regex_search(line, matches, oidAndDetailOidRegex)) {
      auto oid = stoul(matches[1].str());
      auto detailOid = stoul(matches[2].str());
      auto name = matches[3].str();
      auto sourceCodeStart = matches[5].str();
      objectOids[name] = oid;
      objectDetailOids[name] = detailOid;

      // Use the line without the OID.
      decompiledOut << name << ':' << sourceCodeStart << endl;

      // We are starting a new object.
      currentDetailOid = detailOid;
    }
    else
      // Use the line as-is.
      decompiledOut << line << endl;
  }

  return decompiledOut.str();
}

Code* ReplicodeObjects::getObject(uint32 oid) const
{
  if (oid == UNDEFINED_OID)
    return NULL;

  for (auto o = objects_.begin(); o != objects_.end(); ++o) {
    if ((*o)->get_oid() == oid)
      return *o;
  }

  return NULL;
}

Code* ReplicodeObjects::getObjectByDetailOid(uint64 detailOid) const
{
  for (auto o = objects_.begin(); o != objects_.end(); ++o) {
    if ((*o)->get_detail_oid() == detailOid)
      return *o;
  }

  return NULL;
}

}
