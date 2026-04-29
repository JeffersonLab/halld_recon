//
// Created by romanov on 3/28/16.
//
// Extended from original RCDB/Connection.h (v0.07.01)
// to add ability to read run start/end times from rcdb.
// November 14, 2024 [rtj]
//

#ifndef RCDBX_CPP_CONNECTION_H
#define RCDBX_CPP_CONNECTION_H

#include <stdexcept>
#include <string>
#include <memory>
#include <mutex>

#include "rxDataProvider.h"

#ifdef RCDB_SQLITE
    #include "rxSqLiteProvider.h"
#endif

#ifdef RCDB_MYSQL
   #include "rxMySqlProvider.h"
#endif


namespace rcdb{

    class rxConnection{
    public:
        /** Default constructor */
        rxConnection(std::string connectionStr, bool immediateConnect= true):
                _connectionString(connectionStr)
        {
            if (immediateConnect){
                Connect();
            }
        }
        virtual ~rxConnection() {}

        void Connect()
        {
            std::lock_guard<std::mutex> guard(_mutex);

            if(_connectionString.find("sqlite:///") == 0) {
                #ifdef RCDB_SQLITE
                    _provider.reset(new rxSqLiteProvider(_connectionString));
                #else
                    throw std::logic_error("RCDB built without SQLite3 support. Rebuild it using 'with-sqlite=true' flag");
                #endif
            }
            else if(_connectionString.find("mysql://") == 0){

                #ifdef RCDB_MYSQL
                    _provider.reset(new rxMySqlProvider(_connectionString));
                #else                
                    throw std::logic_error("RCDB built without MySQL support. Rebuild it using 'with-mysql=true' flag");
                #endif
            }
            else{
                throw ConnectionStringError("ERROR. Connection string must begin with 'mysql://' or sqlite:///");
            }
        }

        bool IsConnected()
        {
            std::lock_guard<std::mutex> guard(_mutex);
            return _provider.get() != nullptr;
        }

        void Close()
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _provider.reset();
        }

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        std::unique_ptr<Condition> GetCondition(uint64_t runNumber, const ConditionType& cndType)
        {
            return _provider->GetCondition(runNumber, cndType);
        }

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        std::unique_ptr<Condition> GetCondition(uint64_t runNumber, const std::string& name)
        {
            return _provider->GetCondition(runNumber, name);
        }

        /** Gets file (with content) by name and run (@see GetRun and SetRun) */
        std::unique_ptr<RcdbFile> GetFile(uint64_t runNumber, const std::string& name)
        {
            return _provider->GetFile(runNumber, name);
        }

        virtual std::vector<std::string> GetFileNames(uint64_t runNumber)
        {
            return _provider->GetFileNames(runNumber);
        }

        virtual uint32_t GetRunStartTime(uint64_t runNumber)
        {
            return _provider->GetRunStartTime(runNumber);
        }

        virtual uint32_t GetRunEndTime(uint64_t runNumber)
        {
            return _provider->GetRunEndTime(runNumber);
        }

    protected:
        std::string _connectionString;
        std::unique_ptr<rxDataProvider> _provider;
        std::mutex _mutex;    /// This class  uses this mutex

    private:
        rxConnection(const rxConnection &) = delete;               // disable Copy constructor
        rxConnection &operator=(const rxConnection &) = delete;    // disable Copy assignment
    };
}

#endif //RCDBX_CPP_CONNECTION_H
