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

#ifndef DEREF_LAM_H
#define DEREF_LAM_H

#include <vector>
#include "LambdaHelperClasses.h"
#include "ComputeExecutor.h"
#include "SimpleComputeExecutor.h"
#include "TupleSetMachine.h"
#include "TupleSet.h"
#include "Ptr.h"

namespace pdb {

template <class OutType>
class DereferenceLambda : public TypedLambdaObject <OutType> {

public:

	LambdaTree <Ptr <OutType>> input;

public:

	DereferenceLambda (LambdaTree <Ptr <OutType>> &input) : input (input) {}

	std :: string getTypeOfLambda () override {
		return std :: string ("deref");
	}

	int getNumChildren () override {
		return 1;
	}

	GenericLambdaObjectPtr getChild (int which) override {
		if (which == 0)
			return input.getPtr ();
		return nullptr;
	}

        bool addColumnToTupleSet (std :: string &pleaseCreateThisType, TupleSetPtr input, int outAtt) override {
                if (pleaseCreateThisType == getTypeName <OutType> ()) {
                        std :: vector <OutType> *outColumn = new std :: vector <OutType>;
                        input->addColumn (outAtt, outColumn, true);
                        return true;
                }
                return false;
        }

	ComputeExecutorPtr getExecutor (TupleSpec &inputSchema, TupleSpec &attsToOperateOn, TupleSpec &attsToIncludeInOutput) override {
	
		// create the output tuple set
		TupleSetPtr output = std :: make_shared <TupleSet> ();

		// create the machine that is going to setup the output tuple set, using the input tuple set
		TupleSetSetupMachinePtr myMachine = std :: make_shared <TupleSetSetupMachine> (inputSchema, attsToIncludeInOutput);

		// these are the input attributes that we will process
		std :: vector <int> inputAtts = myMachine->match (attsToOperateOn);
		int firstAtt = inputAtts[0];

		// this is the output attribute
		int outAtt = attsToIncludeInOutput.getAtts ().size ();

		return std :: make_shared <SimpleComputeExecutor> (
			output, 
			[=] (TupleSetPtr input) {

				// set up the output tuple set
				myMachine->setup (input, output);	

				// get the columns to operate on
				std :: vector <Ptr <OutType>> &inColumn = input->getColumn <Ptr<OutType>> (firstAtt);

				// create the output attribute, if needed
				if (!output->hasColumn (outAtt)) { 
					std :: vector <OutType> *outColumn = new std :: vector <OutType>;
					output->addColumn (outAtt, outColumn, true); 
				}

				// get the output column
				std :: vector <OutType> &outColumn = output->getColumn <OutType> (outAtt);

				// loop down the columns, setting the output
				int numTuples = inColumn.size ();
				outColumn.resize (numTuples); 
				for (int i = 0; i < numTuples; i++) {
					outColumn[i] = *inColumn[i];
				}
				return output;
			}
		);
		
	}
};

}

#endif