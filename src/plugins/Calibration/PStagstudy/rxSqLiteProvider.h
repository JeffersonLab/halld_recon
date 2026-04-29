//
// Created by romanov on 1/23/16.
//
// Extended from original RCDB/Connection.h (v0.07.01)
// to add ability to read run start/end times from rcdb.
// November 14, 2024 [rtj]
//

#ifndef RCDBX_CPP_SQLITEPROVIDER_H
#define RCDBX_CPP_SQLITEPROVIDER_H

#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <memory>
#include "rxDataProvider.h"
#include <RCDB/RcdbFile.h>

namespace rcdb {
    class rxSqLiteProvider : public rxDataProvider {
    public:
        rxSqLiteProvider(std::string dbPath) :
                rxDataProvider(),
                _db(ParseConnectionString(dbPath)),
                _getConditionQuery(_db, "SELECT id, bool_value, float_value, int_value, text_value, time_value "
                                        "FROM conditions WHERE run_number = ? AND condition_type_id = ?"),
                _getFileQuery(_db, "SELECT files.id AS files_id, "
                                   "       files.path AS files_path, "
                                   "       files.sha256 AS files_sha256, "
                                   "       files.content AS files_content "
                                   "FROM files, files_have_runs AS files_have_runs_1 "
                                   "WHERE files.path = ? AND files.id = files_have_runs_1.files_id "
                                   "      AND ? = files_have_runs_1.run_number "
                                   "ORDER BY files.id DESC"),
                _getFileNamesQuery(_db, "SELECT files.path AS files_path "
                                    "FROM files, files_have_runs AS files_have_runs_1 "
                                    "WHERE files.id = files_have_runs_1.files_id "
                                    "AND ? = files_have_runs_1.run_number "
                                    "ORDER BY files.id DESC")
        {



            //Fill types
            SQLite::Statement query(_db, "SELECT id, name, value_type FROM condition_types");

            while (query.executeStep()) {
                const int id = query.getColumn(0);
                const std::string name(query.getColumn(1).getText()); // = query.getColumn(1).getText();
                const std::string typeStr(query.getColumn(2).getText()); // .getColumn(1).getBytes();

                ConditionType conditionType;
                conditionType.SetId(id);
                conditionType.SetName(name);
                conditionType.SetValueType(ConditionType::StringToValueType(typeStr));
                _types.push_back(conditionType);
                _typesByName[name]=conditionType;
            }
        }

        static std::string ParseConnectionString(std::string connectionStr)
        {
            auto typePos = connectionStr.find("sqlite:///");
            if(typePos==std::string::npos) {
                throw ConnectionStringError("ERROR. SQLite connection string must begin with 'sqlite:///'");
            }

            connectionStr.erase(0,10);

            return connectionStr;
        }

        rxSqLiteProvider(rxSqLiteProvider &&) = default;                  // Move constructor
        rxSqLiteProvider &operator=(rxSqLiteProvider &&) & = default;     // Move assignment operator
        virtual ~rxSqLiteProvider() { }                                   // Destructor

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        virtual std::unique_ptr<Condition> GetCondition(uint64_t runNumber, const ConditionType& cndType) override
        {
            // id:0, bool_value:1, float_value:2, int_value:3, text_value:4, time_value:5
            static const int bool_column = 1;
            static const int float_column = 2;
            static const int int_column = 3;
            static const int text_column = 4;
            static const int time_column = 5;
            uint64_t typeId = cndType.GetId();
            uint64_t run = runNumber;

            _getConditionQuery.reset();
            _getConditionQuery.clearBindings();
            _getConditionQuery.bind(1, (sqlite3_int64)run);
            _getConditionQuery.bind(2, (sqlite3_int64)typeId);


            while (_getConditionQuery.executeStep()) {
                const int id = _getConditionQuery.getColumn(0);
                std::unique_ptr<Condition> condition(new Condition((ConditionType &) cndType));
                condition->SetId(id);

                switch (cndType.GetValueType()) {
                    case ValueTypes::Bool:
                        if(_getConditionQuery.isColumnNull(bool_column)) return std::unique_ptr<Condition>();
                        condition->SetBoolValue((bool)_getConditionQuery.getColumn(bool_column).getInt());
                        return condition;
                    case ValueTypes::Json:
                    case ValueTypes::String:
                    case ValueTypes::Blob:
                        if(_getConditionQuery.isColumnNull(text_column)) return std::unique_ptr<Condition>();
                        condition->SetTextValue(_getConditionQuery.getColumn(text_column).getText());
                        return condition;
                    case ValueTypes::Float:
                        if(_getConditionQuery.isColumnNull(float_column)) return std::unique_ptr<Condition>();
                        condition->SetFloatValue(_getConditionQuery.getColumn(float_column).getDouble());
                        return condition;
                    case ValueTypes::Int:
                        if(_getConditionQuery.isColumnNull(int_column)) return std::unique_ptr<Condition>();
                        condition->SetIntValue(_getConditionQuery.getColumn(int_column).getInt());
                        return condition;
                    case ValueTypes::Time:
                        if(_getConditionQuery.isColumnNull(time_column)) return std::unique_ptr<Condition>();
                        condition->SetTime(
                                std::chrono::system_clock::from_time_t(
                                        _getConditionQuery.getColumn(int_column).getInt64()));
                        return condition;
                    default:
                        throw std::logic_error("ValueTypes type is something different than one of possible values");
                }
            }

            return std::unique_ptr<Condition>(); //Empty ptr
        }

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        virtual std::unique_ptr<RcdbFile> GetFile(uint64_t runNumber, const std::string& name) override
        {
            _getFileQuery.reset();
            _getFileQuery.clearBindings();
            _getFileQuery.bind(1, name.c_str());
            _getFileQuery.bind(2, (sqlite3_int64)runNumber);


            while (_getFileQuery.executeStep()) {
                const uint64_t id = _getFileQuery.getColumn(0).getInt64();
                const std::string path(_getFileQuery.getColumn(1).getText());     // files.path AS files_path
                const std::string sha256(_getFileQuery.getColumn(2).getText());   // files.sha256 AS files_sha256
                const std::string content(_getFileQuery.getColumn(3).getText());  // files.content AS files_content

                std::unique_ptr<RcdbFile> file(new RcdbFile(id, path, sha256, content));
                return file;
            }

            return std::unique_ptr<RcdbFile>(); //Empty ptr
        }


        /** Gets conditions by name and run (@see GetRun and SetRun) */
        virtual std::vector<std::string> GetFileNames(uint64_t runNumber)  override
        {
            _getFileNamesQuery.reset();
            _getFileNamesQuery.clearBindings();
            _getFileNamesQuery.bind(1, (sqlite3_int64)runNumber);

            std::vector<std::string> filePaths;
            while (_getFileQuery.executeStep()) {
                filePaths.push_back(_getFileQuery.getColumn(0).getText());
            }

            return filePaths; //Empty ptr
        }

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        virtual std::unique_ptr<Condition> GetFile(uint64_t runNumber, const ConditionType& cndType)
        {
            // id:0, bool_value:1, float_value:2, int_value:3, text_value:4, time_value:5
            static const int bool_column = 1;
            static const int float_column = 2;
            static const int int_column = 3;
            static const int text_column = 4;
            static const int time_column = 5;
            uint64_t typeId = cndType.GetId();
            uint64_t run = runNumber;

            _getConditionQuery.reset();
            _getConditionQuery.clearBindings();
            _getConditionQuery.bind(1, (sqlite3_int64)run);
            _getConditionQuery.bind(2, (sqlite3_int64)typeId);


            while (_getConditionQuery.executeStep()) {
                const int id = _getConditionQuery.getColumn(0);
                std::unique_ptr<Condition> condition(new Condition((ConditionType &) cndType));
                condition->SetId(id);

                switch (cndType.GetValueType()) {
                    case ValueTypes::Bool:
                        if(_getConditionQuery.isColumnNull(bool_column)) return std::unique_ptr<Condition>();
                        condition->SetBoolValue((bool)_getConditionQuery.getColumn(bool_column).getInt());
                        return condition;
                    case ValueTypes::Json:
                    case ValueTypes::String:
                    case ValueTypes::Blob:
                        if(_getConditionQuery.isColumnNull(text_column)) return std::unique_ptr<Condition>();
                        condition->SetTextValue(_getConditionQuery.getColumn(text_column).getText());
                        return condition;
                    case ValueTypes::Float:
                        if(_getConditionQuery.isColumnNull(float_column)) return std::unique_ptr<Condition>();
                        condition->SetFloatValue(_getConditionQuery.getColumn(float_column).getDouble());
                        return condition;
                    case ValueTypes::Int:
                        if(_getConditionQuery.isColumnNull(int_column)) return std::unique_ptr<Condition>();
                        condition->SetIntValue(_getConditionQuery.getColumn(int_column).getInt());
                        return condition;
                    case ValueTypes::Time:
                        if(_getConditionQuery.isColumnNull(time_column)) return std::unique_ptr<Condition>();
                        condition->SetTime(
                                std::chrono::system_clock::from_time_t(
                                        _getConditionQuery.getColumn(int_column).getInt64()));
                        return condition;
                    default:
                        throw std::logic_error("ValueTypes type is something different than one of possible values");
                }
            }

            return std::unique_ptr<Condition>(); //Empty ptr
        }

        /** Gets conditions by name and run (@see GetRun and SetRun) */
        std::unique_ptr<Condition> GetCondition(uint64_t runNumber, const std::string& name)
        {
            return GetCondition(runNumber, _typesByName[name]);
        }

        void Test() {
            // Open a database file in readonly mode
            SQLite::Database db("/home/romanov/gluex/rcdb/rcdb_2016-01-28.sqlite.db");  // SQLITE_OPEN_READONLY

            std::cout << "SQLite database file '" << db.getFilename().c_str() << "' opened successfully\n";

            ///// a) Loop to get values of column by index, using auto cast to variable type

            // Compile a SQL query, containing one parameter (index 1)
            SQLite::Statement query(db, "SELECT id, name, value_type FROM condition_types WHERE id > ?");
            std::cout << "SQLite statement '" << query.getQuery().c_str() << "' compiled (" << query.getColumnCount() <<
            " columns in the result)\n";

            // Bind the integer value 2 to the first parameter of the SQL query
            query.bind(1, 5);
            std::cout << "binded with integer value '5' :\n";

            // Loop to execute the query step by step, to get one a row of results at a time
            while (query.executeStep()) {
                // Demonstrates how to get some typed column value (and the equivalent explicit call)
                const int id = query.getColumn(0); // = query.getColumn(0).getInt();
                //const char*       pvalue = query.getColumn(1); // = query.getColumn(1).getText();
                const std::string name(query.getColumn(1).getText()); // = query.getColumn(1).getText();
                const std::string type(query.getColumn(2).getText()); // .getColumn(1).getBytes();

                std::cout << "row (" << id << ", \"" << name.c_str() << "\"(" << type << ") " << ")\n";
            }

            ///// b) Get aliased column names (and original column names if possible)

            // Reset the query to use it again
            query.reset();
        }

        virtual uint32_t GetRunStartTime(uint64_t runNumber)
        {
            SQLite::Statement query(_db, "SELECT runs.started from runs "
                                         "WHERE runs.number = " + to_string(runNumber));
            uint32_t started;
            while (query.executeStep()) {
                started = query.getColumn(0);
            }
            return started;
        }

        virtual uint32_t GetRunEndTime(uint64_t runNumber)
        {
            SQLite::Statement query(_db, "SELECT runs.finished from runs "
                                         "WHERE runs.number = " + to_string(runNumber));
            uint32_t finished;
            while (query.executeStep()) {
                finished = query.getColumn(0);
            }
            return finished;
        }

    protected:

    private:
        rxSqLiteProvider(const rxSqLiteProvider &) = delete;             // disable Copy constructor
        rxSqLiteProvider &operator=(const rxSqLiteProvider &) = delete;  // disable Copy assignment operator

        SQLite::Database _db;
        SQLite::Statement _getConditionQuery;
        SQLite::Statement _getFileQuery;
        SQLite::Statement _getFileNamesQuery;
    };
}

#endif //RCDBX_CPP_SQLITEPROVIDER_H
