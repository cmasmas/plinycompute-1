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
#ifndef TPCH_DATA_GENERATOR_NEW_CC
#define TPCH_DATA_GENERATOR_NEW_CC

// JiaNote: this version of data generator cleanup allocation blocks manually to resolve the memory
// leak issue brought by cross-block allocation.

#include "CatalogClient.h"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <chrono>
#include <sstream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"

#include "Part.h"
#include "Supplier.h"
#include "LineItem.h"
#include "Order.h"
#include "Customer.h"
#include "SumResultWriteSet.h"
#include "CustomerWriteSet.h"
#include "CustomerSupplierPartGroupBy.h"
#include "CustomerMultiSelection.h"
#include "ScanCustomerSet.h"
#include "SupplierData.h"
#include "CountAggregation.h"
#include "SumResult.h"

#include "Handle.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "WriteBuiltinEmployeeSet.h"
#include "SupervisorMultiSelection.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"

#include "QueryOutput.h"
#include "DataTypes.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include "WriteStringSet.h"

using namespace std;

// Run a Cluster on Localhost
// ./bin/pdb-manager localhost 8108 Y
// ./bin/pdb-worker 4 4000 localhost:8108 localhost:8109
// ./bin/pdb-worker 4 4000 localhost:8108 localhost:8110

// TPCH data set is available here https://drive.google.com/file/d/0BxMSXpJqaNfNMzV1b1dUTzVqc28/view
// Just unzip the file and put the folder in main directory of PDB

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

//#define BLOCKSIZE (256*MB)
#define BLOCKSIZE DEFAULT_NET_PAGE_SIZE


// A function to parse a Line
std::vector<std::string> parseLine(std::string line) {
    stringstream lineStream(line);
    std::vector<std::string> tokens;
    string token;
    while (getline(lineStream, token, '|')) {
        tokens.push_back(token);
    }
    return tokens;
}

void dataGenerator(std::string scaleFactor,
                   pdb::PDBClient &pdbClient,
                   int noOfCopies) {

    // All files to parse:
    string PartFile = "tables_scale_" + scaleFactor + "/part.tbl";
    string supplierFile = "tables_scale_" + scaleFactor + "/supplier.tbl";
    string orderFile = "tables_scale_" + scaleFactor + "/orders.tbl";
    string lineitemFile = "tables_scale_" + scaleFactor + "/lineitem.tbl";
    string customerFile = "tables_scale_" + scaleFactor + "/customer.tbl";

    // Common constructs:
    string line;
    string delimiter = "|";
    ifstream infile;

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########      Part       #########
    // ##########                 #########
    // ####################################
    // ####################################

    infile.open(PartFile.c_str());
    if (!infile.is_open()) {
        cout << "No data directory for part found, add the data directory " << PartFile << endl;
        exit(-1);
    }


    vector<pdb::Handle<Part>> partList;
    map<int, pdb::Handle<Part>> partMap;

    while (getline(infile, line)) {

        std::vector<string> tokens = parseLine(line);

        int partID = atoi(tokens.at(0).c_str());

        if (partID % 10000 == 0)
            cout << "Part ID: " << partID << endl;

        pdb::Handle<Part> part = pdb::makeObject<Part>(atoi(tokens.at(0).c_str()),
                                                       tokens.at(1),
                                                       tokens.at(2),
                                                       tokens.at(3),
                                                       tokens.at(4),
                                                       atoi(tokens.at(5).c_str()),
                                                       tokens.at(6),
                                                       atof(tokens.at(7).c_str()),
                                                       tokens.at(8));

        partList.push_back(part);

        // Populate the hash:
        partMap[atoi(tokens.at(0).c_str())] = part;
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########     Supplier    #########
    // ##########                 #########
    // ####################################
    // ####################################

    infile.open(supplierFile.c_str());
    if (!infile.is_open()) {
        cout << "No data directory for supplier found, add the data directory " << supplierFile
             << endl;
    }

    vector<pdb::Handle<Supplier>> supplierList;
    map<int, pdb::Handle<Supplier>> supplierMap;

    while (getline(infile, line)) {
        std::vector<string> tokens = parseLine(line);

        int supplierKey = atoi(tokens.at(0).c_str());

        pdb::Handle<Supplier> tSupplier = pdb::makeObject<Supplier>(supplierKey,
                                                                    tokens.at(1),
                                                                    tokens.at(2),
                                                                    atoi(tokens.at(3).c_str()),
                                                                    tokens.at(4),
                                                                    atof(tokens.at(5).c_str()),
                                                                    tokens.at(6));
        supplierList.push_back(tSupplier);

        if (supplierKey % 1000 == 0)
            cout << "Supplier ID: " << supplierKey << endl;

        // Populate the hash:
        supplierMap[atoi(tokens.at(0).c_str())] = tSupplier;
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########     LineItem    #########
    // ##########                 #########
    // ####################################
    // ####################################

    // Open "LineitemFile": Iteratively (Read line, Parse line, Create Objects):
    infile.open(lineitemFile.c_str());

    if (!infile.is_open()) {
        cout << "No data directory for lineitem  found, add the data directory " << lineitemFile
             << endl;
    }

    pdb::Handle<pdb::Vector<LineItem>> lineItemList = pdb::makeObject<pdb::Vector<LineItem>>();

    map<int, pdb::Handle<pdb::Vector<LineItem>>> lineItemMap;

    while (getline(infile, line)) {

        std::vector<string> tokens = parseLine(line);

        int orderKey = atoi(tokens.at(0).c_str());
        int partKey = atoi(tokens.at(1).c_str());
        int supplierKey = atoi(tokens.at(2).c_str());

        pdb::Handle<Part> tPart;
        pdb::Handle<Supplier> tSupplier;

        // Find the appropriate "Part"
        if (partMap.find(partKey) != partMap.end()) {
            tPart = partMap[partKey];
        } else {
            throw invalid_argument("There is no such Part.");
        }

        // Find the appropriate "Part"
        if (supplierMap.find(supplierKey) != supplierMap.end()) {
            tSupplier = supplierMap[supplierKey];
        } else {
            throw invalid_argument("There is no such Supplier.");
        }

        pdb::Handle<LineItem> tLineItem = pdb::makeObject<LineItem>(orderKey,
                                                                    tSupplier,
                                                                    tPart,
                                                                    atoi(tokens.at(3).c_str()),
                                                                    atof(tokens.at(4).c_str()),
                                                                    atof(tokens.at(5).c_str()),
                                                                    atof(tokens.at(6).c_str()),
                                                                    atof(tokens.at(7).c_str()),
                                                                    tokens.at(8),
                                                                    tokens.at(9),
                                                                    tokens.at(10),
                                                                    tokens.at(11),
                                                                    tokens.at(12),
                                                                    tokens.at(13),
                                                                    tokens.at(14),
                                                                    tokens.at(15));

        if (orderKey % 100000 == 0)
            cout << "LineItem ID: " << orderKey << endl;

        // Populate the hash:
        if (lineItemMap.find(orderKey) != lineItemMap.end()) {
            // the key already exists in the map
            lineItemMap[orderKey]->push_back(*tLineItem);
        } else {
            // make a new vector
            pdb::Handle<pdb::Vector<LineItem>> lineItemList =
                pdb::makeObject<pdb::Vector<LineItem>>();

            // push in the vector
            lineItemList->push_back(*tLineItem);
            // put in the map
            lineItemMap[orderKey] = lineItemList;
        }
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ########## 				  #########
    // ##########       Order     #########
    // ##########                 #########
    // ####################################
    // ####################################

    // Open "OrderFile": Iteratively (Read line, Parse line, Create Objects):
    infile.open(orderFile.c_str());

    if (!infile.is_open()) {
        cout << "No data directory for orderFile  found, add the data directory " << orderFile
             << endl;
    }

    map<int, pdb::Vector<Order>> orderMap;

    while (getline(infile, line)) {
        std::vector<string> tokens = parseLine(line);

        int orderKey = atoi(tokens.at(0).c_str());
        int customerKey = atoi(tokens.at(1).c_str());

        pdb::Handle<Order> tOrder = pdb::makeObject<Order>(*lineItemMap[orderKey],
                                                           orderKey,
                                                           customerKey,
                                                           tokens.at(2),
                                                           atof(tokens.at(3).c_str()),
                                                           tokens.at(4),
                                                           tokens.at(5),
                                                           tokens.at(6),
                                                           atoi(tokens.at(7).c_str()),
                                                           tokens.at(8));

        if (orderKey % 100000 == 0)
            cout << "Order ID: " << orderKey << endl;

        // Populate the hash:
        if (orderMap.find(customerKey) != orderMap.end()) {
            orderMap[customerKey].push_back(*tOrder);
        } else {
            pdb::Handle<pdb::Vector<Order>> orderList = pdb::makeObject<pdb::Vector<Order>>();
            orderList->push_back(*tOrder);
            orderMap[customerKey] = *orderList;
        }
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########    Customers    #########
    // ##########                 #########
    // ####################################
    // ####################################

    cout << "Started Creating Customers ..." << endl;

    size_t sendingObjectSize = 0;
    string errMsg;

    // make a new Allocation Block
    pdb::makeObjectAllocatorBlock((size_t)BLOCKSIZE, true);
    pdb::Handle<pdb::Vector<pdb::Handle<Customer>>> storeMeCustomerList =
        pdb::makeObject<pdb::Vector<pdb::Handle<Customer>>>();
    map<int, pdb::Handle<Part>>* myPartMap = new map<int, pdb::Handle<Part>>();
    map<int, pdb::Handle<Supplier>>* mySupplierMap = new map<int, pdb::Handle<Supplier>>();
    // Copy the same data multiple times to make it bigger.
    for (int i = 0; i < noOfCopies; ++i) {

        // open the data file
        infile.open(customerFile.c_str());

        // if file does not exists
        if (!infile.is_open()) {
            cout << "No data directory for customer  found, add the data directory " << customerFile
                 << endl;
        }

        pdb::Handle<Customer> objectToAdd = nullptr;

        while (getline(infile, line)) {
            std::vector<string> tokens = parseLine(line);
            int customerKey = atoi(tokens.at(0).c_str());

            try {
                pdb::Vector<Order> myOrders = orderMap[customerKey];
                pdb::Vector<Order> newOrders;
                size_t myOrderSize = myOrders.size();
                for (size_t orderId = 0; orderId < myOrderSize; orderId++) {
                    Order myOrder = myOrders[orderId];
                    pdb::Vector<LineItem> myLineItems = myOrder.getLineItems();
                    size_t myLineItemSize = myLineItems.size();
                    pdb::Vector<LineItem> newLineItems(myLineItemSize);
                    for (size_t lineItemId = 0; lineItemId < myLineItemSize; lineItemId++) {
                        LineItem myLineItem = myLineItems[lineItemId];
                        pdb::Handle<Supplier> mySupplier = myLineItem.getSupplier();
                        // now check whether we have this Supplier already
                        int mySupplierId = mySupplier->getSupplierKey();
                        pdb::Handle<Supplier> newSupplier = nullptr;
                        if (mySupplierMap->count(mySupplierId) == 0) {
                            newSupplier = deepCopyToCurrentAllocationBlock(mySupplier);
                            (*mySupplierMap)[mySupplierId] = newSupplier;
                        } else {
                            newSupplier = (*mySupplierMap)[mySupplierId];
                        }
                        pdb::Handle<Part> myPart = myLineItem.getPart();
                        // now check whether we have this Part already
                        int myPartId = myPart->getPartKey();
                        pdb::Handle<Part> newPart = nullptr;
                        if (myPartMap->count(myPartId) == 0) {
                            newPart = deepCopyToCurrentAllocationBlock(myPart);
                            (*myPartMap)[myPartId] = newPart;
                        } else {
                            newPart = (*myPartMap)[myPartId];
                        }
                        LineItem newLineItem = myLineItem;
                        newLineItem.setSupplier(newSupplier);
                        newLineItem.setPart(newPart);
                        newLineItems.push_back(newLineItem);
                    }
                    Order newOrder = myOrder;
                    newOrder.setLineItems(newLineItems);
                    newOrders.push_back(newOrder);
                }
                objectToAdd = pdb::makeObject<Customer>(newOrders,
                                                        customerKey,
                                                        tokens.at(1),
                                                        tokens.at(2),
                                                        atoi(tokens.at(3).c_str()),
                                                        tokens.at(4),
                                                        atof(tokens.at(5).c_str()),
                                                        tokens.at(6),
                                                        tokens.at(7));
                storeMeCustomerList->push_back(objectToAdd);

            } catch (NotEnoughSpace& e) {

                // First send the existing data over
                if (storeMeCustomerList->size() > 0) {
                    Record<Vector<Handle<Object>>>* myRecord =
                        (Record<Vector<Handle<Object>>>*)getRecord(storeMeCustomerList);
                    pdbClient.sendBytes<Customer>(
                            std::pair<std::string, std::string>("tpch_bench_set1", "TPCH_db"),
                            (char*)myRecord,
                            myRecord->numBytes());
                    sendingObjectSize += storeMeCustomerList->size();

                    std::cout << "Sending data! Count: " << sendingObjectSize << std::endl;
                } else {
                    std::cout << "Vector is zero." << sendingObjectSize << std::endl;
                }

                storeMeCustomerList->clear();
                // make a allocation Block and a new vector.
                pdb::makeObjectAllocatorBlock((size_t)BLOCKSIZE, true);
                if (myPartMap != nullptr) {
                    delete myPartMap;
                }
                myPartMap = new map<int, pdb::Handle<Part>>();
                if (mySupplierMap != nullptr) {
                    delete mySupplierMap;
                }
                mySupplierMap = new map<int, pdb::Handle<Supplier>>();
                storeMeCustomerList = pdb::makeObject<pdb::Vector<pdb::Handle<Customer>>>();
                // retry to make the object and add it to the vector
                try {
                    objectToAdd = pdb::makeObject<Customer>(orderMap[customerKey],
                                                            customerKey,
                                                            tokens.at(1),
                                                            tokens.at(2),
                                                            atoi(tokens.at(3).c_str()),
                                                            tokens.at(4),
                                                            atof(tokens.at(5).c_str()),
                                                            tokens.at(6),
                                                            tokens.at(7));
                    storeMeCustomerList->push_back(objectToAdd);
                } catch (NotEnoughSpace& e) {
                    std::cerr << "This should not happen that we have not enough space after new "
                                 "allocation. Object size is too large. "
                              << std::endl;
                    return;
                }
            }
        }
        infile.close();
        infile.clear();
    }
    // send the rest of data at the end, it can happen that the exception never happens.
    Record<Vector<Handle<Object>>>* myRecord =
        (Record<Vector<Handle<Object>>>*)getRecord(storeMeCustomerList);
    pdbClient.sendBytes<Customer>(
            std::pair<std::string, std::string>("tpch_bench_set1", "TPCH_db"),
            (char*)myRecord,
            myRecord->numBytes());
    sendingObjectSize += storeMeCustomerList->size();

    std::cout << "Send the rest of the data at the end: " << sendingObjectSize << std::endl;
    storeMeCustomerList->clear();

    // make a allocation Block and a new vector.
    pdb::makeObjectAllocatorBlock((size_t)BLOCKSIZE, true);
    storeMeCustomerList = pdb::makeObject<pdb::Vector<pdb::Handle<Customer>>>();
    if (myPartMap != nullptr) {
        delete myPartMap;
    }
    myPartMap = new map<int, pdb::Handle<Part>>();
    if (mySupplierMap != nullptr) {
        delete mySupplierMap;
    }
    mySupplierMap = new map<int, pdb::Handle<Supplier>>();

    storeMeCustomerList = nullptr;
    if (myPartMap != nullptr) {
        delete myPartMap;
    }
    if (mySupplierMap != nullptr) {
        delete mySupplierMap;
    }
}

int main(int argc, char* argv[]) {

    // TPCH Data file scale - Data should be in folder named "tables_scale_"+"scaleFactor"
    string scaleFactor = "0.1";
    int noOfCopies = 1;

    if (argc > 1) {
        scaleFactor = std::string(argv[1]);
    }

    if (argc > 2) {
        noOfCopies = atoi(argv[2]);
    }

    // Connection info
    string managerHostname = "localhost";
    int managerPort = 8108;

    // Create connection to PlinyCompute manager node
    pdb::PDBClient pdbClient(managerPort, managerHostname);

    string errMsg;

    int noOfCopiesEachRound = 16;
    std::cout << "#######################################" << std::endl;
    if (noOfCopiesEachRound >= 8)
        std::cout << "MEMORY REQUIREMENT: " << (noOfCopiesEachRound / 8) * 10 << " GB" << std::endl;
    std::cout << "#######################################" << std::endl;
    int numFullRounds = noOfCopies / noOfCopiesEachRound;
    int noOfCopiesPartialRound = noOfCopies - (numFullRounds * noOfCopiesEachRound);
    for (int i = 0; i < numFullRounds; i++) {
        {
            pdb::makeObjectAllocatorBlock((size_t)2 * GB, true);
            // Generate the data
            dataGenerator(scaleFactor, pdbClient, noOfCopiesEachRound);
            // flush to disk
            pdbClient.flushData();
            cout << errMsg << endl;
        }

        std::cout << getAllocator().printInactiveBlocks() << std::endl;
        std::cout << getAllocator().printCurrentBlock() << std::endl;
        getAllocator().cleanInactiveBlocks();
        std::cout << getAllocator().printInactiveBlocks() << std::endl;
        std::cout << getAllocator().printCurrentBlock() << std::endl;
    }
    if (noOfCopiesPartialRound > 0) {
        {
            pdb::makeObjectAllocatorBlock((size_t)2 * GB, true);
            // Generate the data
            dataGenerator(scaleFactor, pdbClient, noOfCopiesPartialRound);
            // flush to disk
            pdbClient.flushData();
            cout << errMsg << endl;
        }

        std::cout << getAllocator().printInactiveBlocks() << std::endl;
        std::cout << getAllocator().printCurrentBlock() << std::endl;
        getAllocator().cleanInactiveBlocks();
        std::cout << getAllocator().printInactiveBlocks() << std::endl;
        std::cout << getAllocator().printCurrentBlock() << std::endl;
    }
}

#endif
