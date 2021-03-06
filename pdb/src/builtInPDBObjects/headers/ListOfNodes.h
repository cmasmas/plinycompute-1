/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Rice University                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#ifndef LIST_OF_NODES_H
#define LIST_OF_NODES_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "PDBVector.h"

using pdb::Vector;

//  PRELOAD %ListOfNodes%

// List of current cluster nodes from distribution manager.
namespace pdb {

class ListOfNodes : public Object {

public:
    ENABLE_DEEP_COPY

    ListOfNodes() {}

    ~ListOfNodes() {}

    Handle<Vector<String>> getHostNames() {
        return hostNames;
    }

    void setHostNames(Handle<Vector<String>> hostNames) {
        this->hostNames = hostNames;
    }

private:
    // hostnames of the nodes
    Handle<Vector<String>> hostNames;
};
}
#endif
