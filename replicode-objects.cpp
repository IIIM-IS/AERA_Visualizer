//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2018-2026 Jeff Thompson
//_/_/ Copyright (c) 2018-2026 Kristinn R. Thorisson
//_/_/ Copyright (c) 2023-2026 Chloe Schaff
//_/_/ Copyright (c) 2018-2026 Icelandic Institute for Intelligent Machines
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

#include <fstream>
#include <sstream>
#include <algorithm>
#include "submodules/AERA/r_comp/preprocessor.h"
#include "submodules/AERA/r_comp/compiler.h"
#include "submodules/AERA/r_comp/decompiler.h"
#include "submodules/AERA/r_exec/model_base.h"
#include "submodules/AERA/r_exec/opcodes.h"
#include "submodules/AERA/AERA/main.h"
#include "replicode-objects.hpp"
#include <QApplication>
#include <QProgressDialog>

#include <QMessageBox>

using namespace std;
using namespace std::chrono;
using namespace core;
using namespace r_code;
using namespace r_comp;
using namespace r_exec;

namespace aera_visualizer {

ReplicodeObjects::ReplicodeObjects()
: intMemberRegex_(" ?(\\d+)")
{
  // Set up progressLines_. Used by getProgressLabelText to make the progress messages clearer.
  progressMessages_.push_back("Snapshotting AERA state");
  progressMessages_.push_back("Retrieving objects");
  progressMessages_.push_back("Postprocessing code");
  progressMessages_.push_back("Reading runtime output");
  progressMessages_.push_back("Setting up workspace");

  initialized_ = false;
}

string ReplicodeObjects::init(const string& userClassesFilePath, const string& decompiledFilePath,
    microseconds basePeriod, QProgressDialog& progress)
{
  // TO DO: These will need to be reconciled with the slightly different ones of the other init
  // Set up progressLines_. Used by getProgressLabelText to make the progress messages clearer.
  progressMessages_.push_back("Preprocessing code (1 of 2)");
  progressMessages_.push_back("Preprocessing code (2 of 2)");
  progressMessages_.push_back("Compiling code");
  progressMessages_.push_back("Postprocessing code");
  progressMessages_.push_back("Reading runtime output");

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

  progress.setLabelText(getProgressLabelText("Preprocessing code (1 of 2)"));
  QApplication::processEvents();
  if (progress.wasCanceled())
    return "cancel";

  if (!preprocessor.process(
      &userClassesFile, userClassesFilePath, &dummyPreprocessedUserClasses, error, &metadata))
    return error;
  dummyPreprocessedUserClasses.clear();

  InitOpcodes(metadata);
  // Now() is called when constructing model controllers.
  r_exec::Now = Time::Get;

  map<string, uint32> objectOids;
  map<string, uint64> objectDetailOids;

  {
    ifstream testOpen(decompiledFilePath);
    if (!testOpen)
      return "Can't open decompiled objects file: " + decompiledFilePath;
  }
  // This sets timeReference_ .
  auto decompiledOut = processDecompiledObjects(decompiledFilePath, objectOids, objectDetailOids);

  // Preprocess and compile the processed decompiler output, using the metadata we got above.
  istringstream decompiledIn(decompiledOut);
  ostringstream preprocessedOut;

  progress.setLabelText(getProgressLabelText("Preprocessing code (2 of 2)"));
  QApplication::processEvents();
  if (progress.wasCanceled())
    return "cancel";

  if (!preprocessor.process(
      &decompiledIn, decompiledFilePath, &preprocessedOut, error, NULL))
    return error;

  istringstream preprocessedIn(preprocessedOut.str());
  r_comp::Compiler compiler(true);
  r_comp::Image image;

  progress.setLabelText(getProgressLabelText("Compiling code"));
  QApplication::processEvents();
  if (progress.wasCanceled())
    return "cancel";

  if (!compiler.compile(&preprocessedIn, &image, &metadata, error, false)) {
    auto iError = (size_t)preprocessedIn.tellg();
    auto nBeforeError = min(iError, 50);
    auto nAfterError = min(preprocessedIn.str().size() - iError, 50);
    string codeBefore = preprocessedIn.str().substr(iError - nBeforeError, nBeforeError);
    string codeAfter = preprocessedIn.str().substr(iError, nBeforeError);
    return codeBefore + "\n<< " + error + "\n" + codeAfter;
  }

  // Transfer objects from the compiler image to imageObjects.
  resized_vector<Code*> imageObjects;
  // tempMem is only used internally for calling build_object.
  MemExec<LObject, MemStatic> tempMem;
  image.get_objects(&tempMem, imageObjects);

  progress.setLabelText(getProgressLabelText("Postprocessing code"));
  // We update progress for 3 loops of imageObjects.size().
  progress.setMaximum(imageObjects.size() * 3);
  // Set the OIDs and detail OIDs of objects in imageObjects based on the decompiled output.
  // Set up objectLabel_ and labelObject_ based on the object in imageObjects.
  for (auto i = 0; i < imageObjects.size(); ++i) {
    if (progress.wasCanceled())
      return "cancel";
    progress.setValue(i);
    if (i % 100 == 0)
      QApplication::processEvents();

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

  r_code::list<P<r_code::Code>> objects;
  // Transfer imageObjects to objects, unpacking and processing as needed.
  // Imitate _Mem::load.
  for (uint32 i = 0; i < imageObjects.size(); ++i) {
    Code* object = imageObjects[i];
    int32 dummyLocation;
    objects.push_back(object, dummyLocation);
    // We don't need to delete, so don't set the storage index.

    switch (object->code(0).getDescriptor()) {
    case Atom::MODEL:
      _Mem::unpack_hlp(object);
      r_exec::ModelBase::Get()->load(object);
      break;
    case Atom::COMPOSITE_STATE:
      _Mem::unpack_hlp(object);
      break;
    case Atom::INSTANTIATED_PROGRAM: // refine the opcode depending on the inputs and the program type.
      if (object->get_reference(0)->code(0).asOpcode() == Opcodes::Pgm) {

        if (object->get_reference(0)->code(object->get_reference(0)->code(PGM_INPUTS).asIndex()).getAtomCount() == 0)
          object->code(0) = Atom::InstantiatedInputLessProgram(object->code(0).asOpcode(), object->code(0).getAtomCount());
      }
      else
        object->code(0) = Atom::InstantiatedAntiProgram(object->code(0).asOpcode(), object->code(0).getAtomCount());
      break;
    }

    for (auto v = object->views_.begin(); v != object->views_.end(); ++v) {

      // init hosts' member_set.
      View* view = (View*)*v;
      view->set_object(object);
      Group* host = view->get_host();

#if 0 // debug: host is NULL.
      if (!host->load(view, object))
        return false;
#endif
    }
  }

  _Mem::init_timestamps(timeReference_, objects);

  return initHelper(metadata, &objects, image.object_names_.symbols_, &progress);
}


string ReplicodeObjects::init(AERA_interface* aera, microseconds basePeriod, QProgressDialog& progress)
{
  // Store this
  basePeriod_ = basePeriod;

  progress.setLabelText(getProgressLabelText("Snapshotting AERA state"));
  QApplication::processEvents();
  if (progress.wasCanceled())
    return "cancel";

  // Get current state of AERA
  r_comp::Metadata metadata = aera->getMetadata();  // Retreve metadata to interpret objects
  auto objects = const_cast<r_code::list<P<r_code::Code>>*>(aera->getMem()->get_objects_());    // Get objects directly from AERA's memory
  
  progress.setLabelText(getProgressLabelText("Retrieving objects"));
  QApplication::processEvents();
  if (progress.wasCanceled())
    return "cancel";
  
  // We update progress for 3 loops of imageObjects.size().
  progress.setLabelText(getProgressLabelText("Postprocessing code"));
  progress.setMaximum(objects->size() * 3);

  // Make sure to set this
  timeReference_ = aera->getStartTime();

  // Use these names where available
  std::unordered_map<uint32, std::string> seedNames = aera->getSeedNames().symbols_;

  return initHelper(metadata, objects, seedNames, &progress);
}

string ReplicodeObjects::compileLine(AERA_interface* aera, const QString& line)
{
  map<string, uint32> objectOids;
  map<string, uint64> objectDetailOids;
  string decompiledOut;

  regex oidAndDetailOidRegex("^(\\d+)\\((\\d+)\\) (\\w+)(:)(.+)$");
  smatch matches;
  string trimmed = line.trimmed().toStdString();
  if (regex_search(trimmed, matches, oidAndDetailOidRegex)) {
    auto oid = stoul(matches[1].str());
    auto detailOid = stoul(matches[2].str());
    auto name = matches[3].str();
    auto sourceCodeStart = matches[5].str();
    objectOids[name] = oid;
    objectDetailOids[name] = detailOid;

    // Use the line without the OID.
    decompiledOut = name + ':' + sourceCodeStart + '\n';
  }
  else
    return "unrecognized format: " + trimmed;

  istringstream decompiledIn(decompiledOut);
  ostringstream preprocessedOut;

  Preprocessor preprocessor;
  string error;
  if (!preprocessor.process(&decompiledIn, "line", &preprocessedOut, error, NULL))
    return error;

  istringstream preprocessedIn(preprocessedOut.str());
  r_comp::Image* image = aera->debugGetSeed();
  r_comp::Metadata metadata = aera->getMetadata();
  r_comp::Compiler* compiler = aera->getCompiler();
  size_t startSize = image->code_segment_.objects_.size();

  if (!compiler->compile(&preprocessedIn, image, &metadata, error, false)) {
    auto iError = (size_t)preprocessedIn.tellg();
    auto nBeforeError = min(iError, 50);
    auto nAfterError = min(preprocessedIn.str().size() - iError, 50);
    string codeBefore = preprocessedIn.str().substr(iError - nBeforeError, nBeforeError);
    string codeAfter = preprocessedIn.str().substr(iError, nBeforeError);
    return codeBefore + "\n<< " + error + "\n" + codeAfter;
  }

  // Transfer new objects from the compiler image to imageObjects.
  r_code::resized_vector<r_code::Code*>& imageObjects = aera->ram_objects_;
  // tempMem is only used internally for calling build_object.
  MemExec<LObject, MemStatic> tempMem;
  image->get_objects(&tempMem, aera->ram_objects_, startSize);

  // Set the OIDs and detail OIDs of objects in imageObjects based on the decompiled output.
  // Set up objectLabel_ and labelObject_ based on the object in imageObjects.
  for (auto i = startSize; i < imageObjects.size(); ++i) {
    string label = compiler->getObjectName(i);
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

  r_code::list<P<r_code::Code>> objects;
  // Transfer imageObjects to objects.
  // Imitate _Mem::load.
  for (uint32 i = startSize; i < imageObjects.size(); ++i) {
    Code* object = imageObjects[i];
    int32 dummyLocation;
    objects.push_back(object, dummyLocation);
    // We don't need to delete, so don't set the storage index.
  }

  _Mem::init_timestamps(timeReference_, objects);

  return initHelper(metadata, &objects, image->object_names_.symbols_);
}

string ReplicodeObjects::initHelper(
  r_comp::Metadata& metadata, r_code::list<P<r_code::Code>>* objects, unordered_map<uint32, string>& seedNames, QProgressDialog* progress)
{
  int i = 0;
  unordered_map<const Class*, uint16> objectIdPerClass;
  // Initialize objectIdPerClass.
  for (size_t j = 0; j < metadata.classes_by_opcodes_.size(); ++j) {
    if (metadata.classes_by_opcodes_[j].str_opcode != "undefined")
      objectIdPerClass[&metadata.classes_by_opcodes_[j]] = 0;
  }
  r_code::list<P<r_code::Code> >::const_iterator o;
  for (o = objects->begin(); o != objects->end(); ++o) {
    i++;

    if (progress) {
      if (progress->wasCanceled())
        return "cancel";
      progress->setValue(i);
    }
    //if (i % 100 == 0)
    //  QApplication::processEvents();

    assignLabel(*o, objectIdPerClass, metadata, seedNames);
  }

  // Get the source code by decompiling the packed objects in objects
  r_comp::Image packedImage;
  packedImage.object_names_.symbols_ = seedNames;
  packedImage.add_objects(*objects, true);

  Decompiler decompiler;
  decompiler.init(&metadata);

  // Fill the objectNames map from objectLabel_ and use it in decompile_references.
  unordered_map<uint16, std::string> objectNames;
  for (auto i = 0; i < packedImage.code_segment_.objects_.size(); ++i) {
    if (progress) {
      if (progress->wasCanceled())
        return "cancel";
      progress->setValue(objectLabel_.size() + i);
    }
    if (i % 100 == 0)
      QApplication::processEvents();

    auto object = getObjectByDetailOid(packedImage.code_segment_.objects_[i]->detail_oid_);
    if (object)
      objectNames[i] = objectLabel_[object];
  }
  decompiler.decompile_references(&packedImage, &objectNames);

  for (uint16 i = 0; i < packedImage.code_segment_.objects_.size(); ++i) {
    if (progress) {
      if (progress->wasCanceled())
        return "cancel";
      progress->setValue(2 * objects->size() + i);
    }
    if (i % 100 == 0)
      QApplication::processEvents();

    auto object = getObjectByDetailOid(packedImage.code_segment_.objects_[i]->detail_oid_);
    if (object) {
      std::ostringstream decompiledCode;
      decompiler.decompile_object(i, &decompiledCode, timeReference_, false, false, false);
      auto source = decompiledCode.str();

      // Strip ending newlines.
      while (source[source.size() - 1] == '\n')
        source = source.substr(0, source.size() - 1);
      objectSourceCode_[object] = source;
    }
  }

  // Mark that initialization is complete
  initialized_ = true;
  
  return "";
}

// Get a string for the value such as "a", "b" ... "z", "aa", "ab" ....
static std::string makeSuffix(uint32 value) {
  string result;
  do {
    // Get the lowest digit as base 26 and prepend a char from 'a' to 'z'.
    uint32 lowest = value % 26;
    result = (char)('a' + lowest) + result;

    // Shift.
    value /= 26;
  } while (value != 0);

  return result;
}

// Assign object labels. All internal references use OIDs, detail OIDs, or reference
// so the labels can be assigned more or less arbitrarily. If available, we'll use
// the name used in the seed program. If not,  this code follows the convention for
// runtime_out.txt (search for the macro `OUTPUT_LINE`).
void ReplicodeObjects::assignLabel(
  Code* object, unordered_map<const Class*, uint16>& objectIdPerClass,
  const r_comp::Metadata& metadata, const std::unordered_map<uint32, std::string>& seedNames)
{
  if (objectLabel_.find(object) != objectLabel_.end())
    // Already processed.
    return;

  uint32 oid = object->get_oid();
  string label;

  // If a name already exists, use it
  if (seedNames.find(oid) != seedNames.end())
    label = seedNames.at(oid);

  // If not, assign it as CLASS_OID or similar
  else {
    // Imitate Decompiler::decompile_references.
    // https://github.com/IIIM-IS/AERA/blob/df783dc59f4e5344dcfeb219f7239e4079bbd65d/r_comp/decompiler.cpp#L268-L306
    // Retrieve prefix from opcode
    const Class* c = &metadata.classes_by_opcodes_[object->code(0).asOpcode()];
    string className = c->str_opcode;

    // A class name like mk.val has a dot, but this isn't allowed as an identifier.
    replace(className.begin(), className.end(), '.', '_');
    // A class name like |fact has a bar, but this isn't allowed as an identifier.
    // (Use regex_replace because it can handle multi-character strings.)
    regex verticalBarRegex("\\|");
    className = regex_replace(className, verticalBarRegex, "anti_");

    if (object->get_oid() != UNDEFINED_OID)
      // Use the object's OID.
      label = className + "_" + std::to_string(oid);
    else {
      // Create a name with a unique ID.
      uint16 last_object_ID = objectIdPerClass[c];
      objectIdPerClass[c] = last_object_ID + 1;
      label = className + std::to_string(last_object_ID);
    }

    if (labelObject_.find(label) != labelObject_.end()) {
      // The created name matches an existing name. Keep trying an added
      // suffix until it is unique.
      for (uint32 value = 1; true; ++value) {
        std::string new_label = label + makeSuffix(value);
        if (labelObject_.find(new_label) == labelObject_.end()) {
          label = new_label;
          break;
        }
      }
    }
  }

  // Save to objectLabel and labelObject
  objectLabel_[object] = label;
  labelObject_[label] = object;

  // Recursively label the referenced objects.
  for (int i = 0; i < object->references_size(); ++i)
    assignLabel(object->get_reference(i), objectIdPerClass, metadata, seedNames);
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

  // Use objectLabel_ because its keys are the objects.
  for (auto o = objectLabel_.begin(); o != objectLabel_.end(); ++o) {
    if (o->first->get_oid() == oid)
      return o->first;
  }

  return NULL;
}

Code* ReplicodeObjects::getObjectByDetailOid(uint64 detailOid) const
{
  // Use objectLabel_ because its keys are the objects.
  for (auto o = objectLabel_.begin(); o != objectLabel_.end(); ++o) {
    if (o->first->get_detail_oid() == detailOid)
      return o->first;
  }

  return NULL;
}

QString ReplicodeObjects::getProgressLabelText(const QString& message)
{
  auto iMessageMatch = find(progressMessages_.begin(), progressMessages_.end(), message);
  if (iMessageMatch == progressMessages_.end())
    // We don't expect this. The message is not one of the messages added by the constructor.
    return message + " ...";

  QString result = "";
  for (auto it = progressMessages_.begin(); it != progressMessages_.end(); ++it) {
    if (result != "")
      result += "\n";

    if (it == iMessageMatch)
      // Highlight this message.
      result += ("\n=> " + *it + " ...");
    else
      result += *it;
  }

  return result;
}

bool ReplicodeObjects::getObjects(string oids, vector<Code*>& objects)
{
  smatch matches;
  bool gotAllInputs = true;
  while (regex_search(oids, matches, intMemberRegex_)) {
    auto input = getObject(stoul(matches[1].str()));
    if (!input)
      gotAllInputs = false;
    else
      objects.push_back(input);

    oids = matches.suffix();
  }

  return gotAllInputs;
}

}
