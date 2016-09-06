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

#ifndef USE_TEMP_ALLOCATION_BLOCK_H
#define USE_TEMP_ALLOCATION_BLOCK_H

#include "Allocator.h"
#include "InterfaceFunctions.h"

namespace pdb {

class UseTemporaryAllocationBlock {

	AllocatorState oldInfo;
	bool myMemory = false;

public:

	explicit UseTemporaryAllocationBlock (void *memory, size_t size) {
		myMemory = true;
		oldInfo = getAllocator ().temporarilyUseBlockForAllocations (memory, size);			
	}

	explicit UseTemporaryAllocationBlock (size_t size) {
		oldInfo = getAllocator ().temporarilyUseBlockForAllocations (size);	
	}

	~UseTemporaryAllocationBlock () {
		
		// if the outside world supplied the RAM
		if (myMemory) {
			// we don't need to free it when done; the caller is in charge
			getAllocator ().restoreAllocationBlock (oldInfo);
		} else {
			// we do need to free it when done; the caller is not in charge
			getAllocator ().restoreAllocationBlockAndManageOldOne (oldInfo);
		}
	}

	// forbidden, to avoid double frees
	UseTemporaryAllocationBlock (const UseTemporaryAllocationBlock &) = delete;
	UseTemporaryAllocationBlock & operator = (const UseTemporaryAllocationBlock &) = delete;

};

}

#endif

