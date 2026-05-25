//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA Visualizer
//_/_/ 
//_/_/ Copyright (c) 2026 Jeff Thompson
//_/_/ Copyright (c) 2026 Kristinn R. Thorisson
//_/_/ Copyright (c) 2026 Icelandic Institute for Intelligent Machines
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

#ifndef ABAGRAPH_HPP
#define ABAGRAPH_HPP

#include <regex>
#include <QProcess>
#include "submodules/AERA/r_code/object.h"
#include "submodules/AERA/AERA/main.h"

namespace aera_visualizer {

class ReplicodeObjects;

/**
 * AbaGraph is an interface to the abagraph process.
 */
class AbaGraph {
public:
  /**
   * Create a new AbaGraph and start the process.
   * \param path The path to the abagraph executable. If "" then don't start the process.
   * \param replicodeObjects
   */
  AbaGraph(const QString& path, ReplicodeObjects& replicodeObjects);

  /**
   * Send the prompt and return the response lines until a blank line, or "" if timeout.
   * This will append the "\n" to the prompt.
   */
  QString readResponse(const QString& prompt);

  /**
   * Get the object by the ABA ID.
   * \param id The ABA ID.
   * \return The object, or NULL if not found.
   */
  r_code::Code* getObject(uint32 id);

  /**
   * Parse the list of integers ABA IDs and use getObject() to add each to objects.
   * \param ids The string with the list of ABA ID integers, e.g. "12 14". This may be "".
   * \param objects Add found objects. This does not first clear the vector.
   * \return True for success, false if getObject() failed to find an ID
   */
  bool getObjects(std::string ids, std::vector<r_code::Code*>& objects);

  void setAeraInterface(AERA_interface* aera) { aera_ = aera; }

private:
  QProcess abagraph_;
  ReplicodeObjects& replicodeObjects_;
  AERA_interface *aera_;
  std::regex idNotFoundRegex_;
  std::regex intMemberRegex_;
};

}

#endif
