//
// Created by romanov on 1/25/16.
//
// Modified from original RCDB/Connection.h (v0.07.01)
// to add ability to read run start/end times from rcdb.
// November 14, 2024 [rtj]
//

#ifndef RCDBX_CPP_DATAPROVIDER_H
#define RCDBX_CPP_DATAPROVIDER_H

#include <vector>
#include <memory>
#include <map>

#include <RCDB/ConditionType.h>
#include <RCDB/Condition.h>
#include <RCDB/RcdbFile.h>


namespace rcdb {

    class rxDataProvider {
    public:
        /** Gets conditions by conditionType (@see GetRun and SetRun) */
        virtual std::unique_ptr<Condition> GetCondition(uint64_t runNumber, const ConditionType& cndType) = 0;

        /** Gets file saved to database by run number and file name */
        virtual std::unique_ptr<RcdbFile> GetFile(uint64_t runNumber, const std::string& name) = 0;

        virtual std::vector<std::string> GetFileNames(uint64_t runNumber) = 0;

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        std::unique_ptr<Condition> GetCondition(uint64_t runNumber, const std::string& name)
        {
            return GetCondition(runNumber, _typesByName[name]);
        }

        /** New functionality that was missing in the released rcdb c++ interface */
        virtual uint32_t GetRunStartTime(uint64_t runNumber) = 0;
        virtual uint32_t GetRunEndTime(uint64_t runNumber) = 0;

        virtual ~rxDataProvider() { }                                 // Destructor

    protected:


        rxDataProvider() { }

        rxDataProvider(const rxDataProvider &) = default;               // Copy constructor
        rxDataProvider(rxDataProvider &&) = default;                    // Move constructor
        rxDataProvider &operator=(const rxDataProvider &) & = default;  // Copy assignment operator
        rxDataProvider &operator=(rxDataProvider &&) & = default;       // Move assignment operator


        std::vector<ConditionType> _types;                          /// Condition types
        std::map<std::string, ConditionType> _typesByName;          /// Condition types mapped by name

    private:
        //bool mAreConditionTypesLoaded;
    };


}

#endif //RCDBX_CPP_DATAPROVIDER_H
