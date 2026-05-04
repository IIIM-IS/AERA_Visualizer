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

#include "abagraph.hpp"
#include "replicode-objects.hpp"

using namespace std;
using namespace r_code;

namespace aera_visualizer {

AbaGraph::AbaGraph(const QString& path, ReplicodeObjects& replicodeObjects)
  : replicodeObjects_(replicodeObjects),
    intMemberRegex_(" ?(\\d+)")
{
  if (path == "")
    return;

  abagraph_.start(path, QStringList());
  abagraph_.waitForStarted(500);
  // If not started, then abagraph_.state() != QProcess::Running.
}

vector<QString> AbaGraph::readResponse(const QString& prompt)
{
  if (abagraph_.state() != QProcess::Running)
    return vector<QString>();

  abagraph_.write((prompt + "\n").toStdString().c_str());

  vector<QString> response;
  while (true) {
    if (!abagraph_.canReadLine()) {
      if (!abagraph_.waitForReadyRead(5000))
        return vector<QString>();
    }

    auto line = QString::fromUtf8(abagraph_.readLine()).trimmed();
    if (line == "")
      break;
    response.push_back(line);
  }

  return response;
}

Code* AbaGraph::getObject(uint32 id)
{
  return replicodeObjects_.getObject(id);
}

bool AbaGraph::getObjects(string ids, vector<Code*>& objects)
{
  smatch matches;
  bool gotAllInputs = true;
  while (regex_search(ids, matches, intMemberRegex_)) {
    auto input = getObject(stoul(matches[1].str()));
    if (!input)
      gotAllInputs = false;
    else
      objects.push_back(input);

    ids = matches.suffix();
  }

  return gotAllInputs;
}

}
