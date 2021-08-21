/*
    This creates two empty databases, populates values, and retrieves them back
    from the SPIFFS file 
*/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include <String.h>
#include <string>
#include <regex>
#include <vector>
#include <bits/stdc++.h>

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
#define TABLE_NAME "table_gpio"

sqlite3 *db1; // handle for the data base

const char *data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    Serial.printf("%s: \n", (const char *)data);
    for (i = 0; i < argc; i++)
    {
        Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    Serial.printf("\n");
    return 0;
}

/**
 * @brief check and print the error while oeprating on the database
 * 
 * @param rc : return of the sqlite3_exec function
 * @param error_message : error message given by the sqlite3_exec command
 */
void db_error_check(int &rc, char *error_message)
{
    if (rc != SQLITE_OK)
    {
        Serial.printf("SQL error: %s\n", error_message);
        sqlite3_free(error_message);
    }
    else
    {
        Serial.printf("Operation done successfully\n");
    }
}

/**
 * @brief perform insert operation on the gpio database.
 * 
 * @param gpio_number gpio number
 * @param gpio_name friendly gpio name
 * @param gpio_type hardwire or wireless
 * @param cms1 cms information for low 
 * @param cms2 cms information for high
 */
void insert_gpio_operation_on_database(int gpio_number, const char gpio_name[40], const char gpio_type[40], const char cms1[40], const char cms2[40])
{

    char *error_message;
    char gpio_number_char[5];
    itoa(gpio_number, gpio_number_char, 10);

    std::string mycommand = "INSERT INTO  #TABLE_NAME  VALUES (#gpio_number_char,'#gpio_name','#gpio_type','#cms1','#cms2');";
    mycommand = std::regex_replace(mycommand, std::regex("#TABLE_NAME"), TABLE_NAME);
    mycommand = std::regex_replace(mycommand, std::regex("#gpio_number_char"), gpio_number_char);
    mycommand = std::regex_replace(mycommand, std::regex("#gpio_name"), gpio_name);
    mycommand = std::regex_replace(mycommand, std::regex("#gpio_type"), gpio_type);
    mycommand = std::regex_replace(mycommand, std::regex("#gpio_type"), gpio_type);
    mycommand = std::regex_replace(mycommand, std::regex("#cms1"), cms1);
    mycommand = std::regex_replace(mycommand, std::regex("#cms2"), cms2);

    int rc = sqlite3_exec(db1, mycommand.c_str(), NULL, (void *)data, &error_message);
    db_error_check(rc, error_message);
}


void output_from_query_gpio()
{
    char *error_message;
    std::vector<std::string> vect;
    std::string operation_command = "SELECT gpio_number FROM " TABLE_NAME;
    
    int rc = sqlite3_exec(db1,operation_command.c_str(),[](void *data, int argc, char **argv, char **azColName)
    {   
        std::vector<int> *vect= static_cast<std::vector<int>*>(data);
        vect->push_back(atoi(argv[0]));
        return 0;
    },
    (void*)data, //static_cast<void*>(&vect),
    &error_message); //  end of sqlite3_exec

    for (auto gpio_num: vect){
        Serial.printf("%d\n", gpio_num);
    }
}

/**
 * @brief Open the database
 * 
 * @param filename full name of the database to be open. If dota base not already created, then it will create the database.
 * @param db  pointer tot the database of the sqlite3 type
 * @return int 0 for the sucess and other for unsucessfull 
 */
int db_open(const char *filename, sqlite3 **db)
{
    int rc = sqlite3_open(filename, db);
    if (rc)
    {
        Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));

        return rc;
    }
    else
    {
        Serial.printf("Opened database successfully\n");
    }
    return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql)
{
    Serial.println(sql);
    long start = micros();
    int rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        Serial.printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        Serial.printf("Operation done successfully\n");
    }
    Serial.print(F("Time taken:"));
    Serial.println(micros() - start);
    return rc;
}

void setup()
{

    Serial.begin(115200);

    int rc;

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        Serial.println("Failed to mount file system");
        return;
    }

    // list SPIFFS contents
    File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }

    sqlite3_initialize();

    if (db_open("/spiffs/project.db", &db1))
        return;

    /**
 * @brief displaying the all the table in the data bese
 * 
 */
    rc = db_exec(db1, "SELECT name FROM sqlite_master WHERE type='table';");
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db1);
        return;
    }

    /**
 * @brief create the table, if not createed thow the error 
 * 
 */
    Serial.println("Creating the gpio table");
    std::string operation_command;
    operation_command = "CREATE TABLE IF NOT EXISTS " TABLE_NAME " (gpio_number INTEGER PRIMARY KEY ,gpio_name NOT NULL, gpio_type TEXT NOT NULL, email_cms_low TEXT,email_cms_high TEXT);";
    rc = sqlite3_exec(
        db1,
        operation_command.c_str(),
        NULL,
        (void *)data,
        &zErrMsg);
    db_error_check(rc, zErrMsg);

    // inserting into the database
    Serial.println("Inserting into the database");
    insert_gpio_operation_on_database(4, "smoke detector", "hardwire", "abcd@gmai.com", "def@gmail.com");

    insert_gpio_operation_on_database(5, "co2 detector", "hardwire", "abcd@gmai.com", "def@gmail.com");

    insert_gpio_operation_on_database(6, "battery", "hardwire", "abcd@gmai.com", "def@gmail.com");


/*
    operation_command = "SELECT gpio_number FROM " TABLE_NAME;
    rc = db_exec(db1, operation_command.c_str());
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db1);
        return;
    }*/
    output_from_query_gpio();

    sqlite3_close(db1);

    
}



void loop()
{
}
