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

#ifndef CHAR_PTR
#define CHAR_PTR(c) ((char*)c)
#endif
#ifndef REF_COUNT_PREAMBLE_SIZE
#define REF_COUNT_PREAMBLE_SIZE (sizeof(unsigned))
#endif
#ifndef ALLOCATOR_STAMP_BIT
#define ALLOCATOR_STAMP_BIT (4)
#endif
#ifndef PREAMBLE_BIT
#define PREAMBLE_BIT (sizeof(unsigned) * 8)
#endif
#ifndef ALLOCATOR_STAMP
#define ALLOCATOR_STAMP ((*((unsigned*)this)) & ((1 << ALLOCATOR_STAMP_BIT) - 1))
#endif
#ifndef NUM_COPIES
#define NUM_COPIES (((*((unsigned*)this)) & (((1 << (PREAMBLE_BIT - ALLOCATOR_STAMP_BIT)) - 1) << ALLOCATOR_STAMP_BIT)) >> ALLOCATOR_STAMP_BIT)
#endif
