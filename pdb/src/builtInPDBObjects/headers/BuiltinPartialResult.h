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
#ifndef BUILTIN_PARTIAL_RESULT_H
#define BUILTIN_PARTIAL_RESULT_H

// PRELOAD %BuiltinPartialResult%
#include "Object.h"
#include "Centroid.h"
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <iostream>


#ifndef NUM_CLUSTERS
#define NUM_CLUSTERS 10
#endif

namespace pdb {
class BuiltinPartialResult : public Object {

public:
    ENABLE_DEEP_COPY

    BuiltinPartialResult() {}
    ~BuiltinPartialResult() {}

    void initialize() {

        int i;
        for (i = 0; i < NUM_CLUSTERS; i++) {
            centroids[i].initialize();
        }
    }


    void updateCentroid(int clusterIndex, double* point) {

        centroids[clusterIndex].update(point);
    }

    void printCentroids() {
        int i;
        std::cout << std::endl;
        for (i = 0; i < NUM_CLUSTERS; i++) {
            std::cout << "cluster-" << i << ":" << std::endl;
            centroids[i].print();
        }
    }

    Centroid* getCentroids() {

        return centroids;
    }


private:
    Centroid centroids[NUM_CLUSTERS];
};
}


#endif
