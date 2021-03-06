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

#ifndef PDB_ADVANCEDPHYSICALSELECTIONALGORITHM_H
#define PDB_ADVANCEDPHYSICALSELECTIONALGORITHM_H

#include <AdvancedPhysicalOptimizer/AdvancedPhysicalAbstractAlgorithm.h>

namespace pdb {

class AdvancedPhysicalPipelineAlgorithm : public AdvancedPhysicalAbstractAlgorithm  {

public:

  AdvancedPhysicalPipelineAlgorithm(const AdvancedPhysicalPipelineNodePtr &handle,
                                       const std::string &jobID,
                                       bool isProbing,
                                       bool isOutput,
                                       const Handle<SetIdentifier> &source,
                                       const Handle<ComputePlan> &computePlan,
                                       const LogicalPlanPtr &logicalPlan,
                                       const ConfigurationPtr &conf);

  /**
   * Generates the stages for this algorithm
   * @return
   */
  PhysicalOptimizerResultPtr generate(int nextStageID,
                                      const StatisticsPtr &stats) override;

  /**
   * Returns the type of the algorithm
   * @return the type id
   */
  AdvancedPhysicalAbstractAlgorithmTypeID getType() override;

};

}

#endif //PDB_ADVANCEDPHYSICALSELECTIONALGORITHM_H
