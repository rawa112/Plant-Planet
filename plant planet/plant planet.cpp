#include "C:\\Users\\EL10_gazy\\Downloads\\Plant Planet\\sqlite3.h" //The path of sqlites files
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
// uses preprocessor directives to clear the console screen based on the operating system
void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Suggest a group of plants
string Check(string& answer)
{

    if (answer != "y" && answer != "n")
    {
        cout << "Error!! Enter y/n\n";
        cin >> answer;
        clearScreen();
        while (answer != "y" && answer != "n")
        {
            cout << "Error!! Enter y/n\n";
            cin >> answer;
            clearScreen();
        }

        return answer;
    }

    return answer;
}

// Define a Plant class to represent plant details and provide methods to interact with a database
class Plant
{
private:
    string name;       // Plant name
    string waterRate;  // Frequency or amount of water needed
    string bloomRate;  // Frequency or season of blooming
    int age;           // Age of the plant in years
    int purchaseDay;   // Day the plant was purchased
    int purchaseMonth; // Month the plant was purchased
    int purchaseYear;  // Year the plant was purchased
    string note;       // Additional notes about the plant

public:
    // Constructor to initialize a Plant object with its properties
    Plant(string n, string wr, string br, int a, int pd, int pm, int py, string nt)
        : name(n), waterRate(wr), bloomRate(br), age(a),
        purchaseDay(pd), purchaseMonth(pm), purchaseYear(py), note(nt) {}
    
    // Display all plant details to the console
    void displayInfo()
    {
        cout << "Plant Name: " << name << endl;
        cout << "Water Rate: " << waterRate << endl;
        cout << "Bloom Rate: " << bloomRate << endl;
        cout << "Age of Plant: " << age << " years" << endl;
        cout << "Date Purchased: " << purchaseDay << "/" << purchaseMonth << "/" << purchaseYear << endl;
        cout << "Note: " << note << endl;
        cout << endl;
    }

    // Save the plant details to the database using SQLite
    int saveToDatabase(sqlite3* db)
    {
        // Construct SQL query to insert plant details into the plants table
        string sql = "INSERT INTO plants (name, waterRate, bloomRate, age, purchaseDay, purchaseMonth, purchaseYear, note) VALUES ('" + name + "', '" + waterRate + "', '" + bloomRate + "', " + to_string(age) + ", " + to_string(purchaseDay) + ", " + to_string(purchaseMonth) + ", " + to_string(purchaseYear) + ", '" + note + "');";
        char* errorMessage;
        // Execute the SQL command and handle any errors
        int exit = sqlite3_exec(db, sql.c_str(), 0, 0, &errorMessage);
        if (exit != SQLITE_OK)
        {
            cerr << "Error inserting data: " << errorMessage << endl;
            sqlite3_free(errorMessage);
        }
        else
        {
            cout << "Plant data saved successfully!" << endl;
        }
        return exit;
    }
};

// Callback function to display the results of SQL queries
static int displayCallback(void* NotUsed, int argc, char** argv, char** azColName)
{
    for (int i = 0; i < argc; i++)
    {
        cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << endl;
    }
    cout << endl;
    return 0;
}

// Initialize the database connection and create the plants table if it doesn't already exist
int initDatabase(sqlite3*& db)
{
    int exit = sqlite3_open("plants.db", &db);
    if (exit)
    {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return exit;
    }
    // SQL statement to create the plants table if it is not present
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS plants ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, waterRate TEXT, bloomRate TEXT, "
        "age INTEGER, purchaseDay INTEGER, "
        "purchaseMonth INTEGER, purchaseYear INTEGER, note TEXT);";
    char* errorMessage;
    // Execute table creation command and handle errors
    exit = sqlite3_exec(db, createTableSQL, 0, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error creating table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
    return exit;
}

// function if user need to modify a specific plant
int updatePlant(sqlite3* db, int plantID, string newName, string newWaterRate, string newBloomRate, int newAge, int newPurchaseDay, int newPurchaseMonth, int newPurchaseYear, string newNote)
{
    // SQL command to update plant details in the database using the plant ID
    string sql = "UPDATE plants SET name='" + newName + "', waterRate='" + newWaterRate + "', bloomRate='" + newBloomRate +
        "', age=" + to_string(newAge) + ", purchaseDay=" + to_string(newPurchaseDay) +
        ", purchaseMonth=" + to_string(newPurchaseMonth) + ", purchaseYear=" + to_string(newPurchaseYear) +
        ", note='" + newNote + "' WHERE id=" + to_string(plantID) + ";";

    char* errorMessage;
    // Execute the update command and handle any errors
    int exit = sqlite3_exec(db, sql.c_str(), 0, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error updating data: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
    else
    {
        cout << "Plant data updated successfully!" << endl;
    }
    return exit;
}

// Function to delete a specific plant record from the database based on its ID
int deletePlant(sqlite3* db, int plantID)
{
    // SQL command to delete a plant record from the plants table where the ID matches
    string sql = "DELETE FROM plants WHERE id=" + to_string(plantID) + ";";
    char* errorMessage;
    int exit = sqlite3_exec(db, sql.c_str(), 0, 0, &errorMessage);
    // Execute the delete command and check for errors
    if (exit != SQLITE_OK)
    {
        cerr << "Error deleting data: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
    else
    {
        cout << "Plant record deleted successfully!" << endl;
    }
    return exit;
}

// Function to reset plant IDs sequentially in the database after deletions
void resetPlantIDs(sqlite3* db)
{
    char* errorMessage;

    // Create a temporary table without AUTOINCREMENT to reset IDs
    const char* createTempTableSQL =
        "CREATE TABLE IF NOT EXISTS temp_plants ("
        "id INTEGER PRIMARY KEY, "
        "name TEXT, waterRate TEXT, bloomRate TEXT, "
        "age INTEGER, purchaseDay INTEGER, "
        "purchaseMonth INTEGER, purchaseYear INTEGER, note TEXT);";
    // Execute command to create temporary table and check for errors
    int exit = sqlite3_exec(db, createTempTableSQL, 0, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error creating temporary table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    // Copy data from the original table to the temporary table with sequential IDs
    const char* copyDataSQL =
        "INSERT INTO temp_plants (name, waterRate, bloomRate, age, purchaseDay, purchaseMonth, purchaseYear, note) "
        "SELECT name, waterRate, bloomRate, age, purchaseDay, purchaseMonth, purchaseYear, note FROM plants ORDER BY id;";

    exit = sqlite3_exec(db, copyDataSQL, 0, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error copying data to temporary table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    // Drop the original table
    const char* dropOriginalTableSQL = "DROP TABLE plants;";
    exit = sqlite3_exec(db, dropOriginalTableSQL, 0, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error dropping original table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    // Rename the temporary table to the original table's name
    const char* renameTableSQL = "ALTER TABLE temp_plants RENAME TO plants;";
    exit = sqlite3_exec(db, renameTableSQL, 0, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error renaming temporary table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
    else
    {
        cout << "Plant IDs have been reset successfully!" << endl;
    }
}

// Function to search for plants by name using a SQL LIKE query
void searchPlantByName(sqlite3* db, const string& plantName)
{
    // SQL query to find any plant records where the name partially matches the input
    string sql = "SELECT * FROM plants WHERE name LIKE '%" + plantName + "%';";
    char* errorMessage;
    // Execute the query and use displayCallback to show results
    int exit = sqlite3_exec(db, sql.c_str(), displayCallback, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error searching data: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

// Function to list plants based on the purchase year
void listPlantsByPurchaseYear(sqlite3* db, int year)
{
    // SQL query to select plants with a specific purchase year
    string sql = "SELECT * FROM plants WHERE purchaseYear=" + to_string(year) + ";";
    char* errorMessage;
    // Execute the query and display results using displayCallback
    int exit = sqlite3_exec(db, sql.c_str(), displayCallback, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error retrieving data: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

// Function to display unique water rates used by plants in the database
void displayUniqueWaterRates(sqlite3* db)
{
    const char* sql = "SELECT DISTINCT waterRate FROM plants;";
    char* errorMessage;
    // Execute query to fetch unique water rates and display results
    int exit = sqlite3_exec(db, sql, displayCallback, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error retrieving water rates: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

// Load all plants from the database and display them
void loadAndDisplayPlants(sqlite3* db)
{
    const char* sql = "SELECT * FROM plants;";
    char* errorMessage;

    // Execute the SQL command with a callback to display each row
    int exit = sqlite3_exec(db, sql, displayCallback, 0, &errorMessage);
    if (exit != SQLITE_OK)
    {
        cerr << "Error loading data: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

// Function to delete all records from the database and reset the table
void deleteAllRecords(sqlite3* db)
{
    char confirmation;
    cout << "Are you sure you want to delete all records in the database? This action cannot be undone. (y/n): ";
    cin >> confirmation;

    if (confirmation == 'y' || confirmation == 'Y')
    {
        char* errorMessage;

        // SQL command to delete all records from the plants table
        const char* deleteAllSQL = "DELETE FROM plants;";

        int exit = sqlite3_exec(db, deleteAllSQL, 0, 0, &errorMessage);
        if (exit != SQLITE_OK)
        {
            cerr << "Error deleting all records: " << errorMessage << endl;
            sqlite3_free(errorMessage);
            return;
        }

        // Reset the autoincrement ID by vacuuming the database
        const char* resetSQL = "VACUUM;";
        exit = sqlite3_exec(db, resetSQL, 0, 0, &errorMessage);
        if (exit != SQLITE_OK)
        {
            cerr << "Error resetting database: " << errorMessage << endl;
            sqlite3_free(errorMessage);
            return;
        }

        cout << "All records have been deleted and the database has been reset!" << endl;
    }
    else
    {
        cout << "Operation canceled. No records were deleted." << endl;
    }
}

//function to recommend plants to user 
void RecommendPlants()
{
    string line;
    ifstream myfile("recommend plant"); // Open the file named "recommend plant"

    if (myfile.is_open())
    {                    // Check if the file was opened successfully
        bool start = 0;  // Flag to control when to start displaying lines
        bool finish = 0; // Flag to control when to stop displaying lines

        while (getline(myfile, line))
        { // Read each line from the file
            if (line == "////")
            { // If line is "////", indicate end of recommendations
                cout << "\n\n\nThat is all we have \nI hope that you found a nice one\n\n";
                myfile.close(); // Close the file
                continue;
            }

            if (line == "----")
            { // If line is "----", prompt user for more recommendations
                cout << "Do you want another one?\nEnter y/n\n\n";
                string answer;
                cin >> answer; // Get user's response
                clearScreen(); // Clear the screen (assumes this function is defined elsewhere)
                Check(answer); // Process user's answer with Check function

                if (answer == "y")
                { // If user wants more recommendations
                    start = 1;
                    finish = 0;
                    continue;
                }
                else
                { // If user does not want more
                    cout << "\n\nI hope that you found a nice choice\n";
                    myfile.close(); // Close the file
                    continue;
                }
            }

            if (line == "--")
            { // If line is "--", set flags to start displaying lines
                start = 1;
                finish = 0;
                continue;
            }

            if (start && !finish)
            { // If in start mode, display the current line
                cout << line << '\n';
            }
        }
    }
}
int main()
{
    sqlite3* db; // Pointer to SQLite database
    if (initDatabase(db) != SQLITE_OK)
        return -1; // Initialize the database, exit if failed

    int choice; // Variable to store the user’s choice
    while (true)
    { // Infinite loop to display menu and handle user choices
        // Display menu options
        cout << "\n1. Add New Plant\n2. Update Plant\n3. Delete Plant\n4. Search Plant by Name\n"
            << "5. List Plants by Purchase Year\n6. Display Unique Water Rates\n7. Display All Plants\n8. Delete All Records\n9. Recommended plants for you to plant\n10. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice; // Get the user's choice
        clearScreen(); // Clear the screen 
        cin.ignore();  // Ignore any newline character left in the input buffer

        if (choice == 1)
        { // Add new plant
            string name, waterRate, bloomRate, note;
            int age, purchaseDay, purchaseMonth, purchaseYear;

            // Collect information about the new plant
            cout << "Enter plant name: ";
            getline(cin, name);
            cout << "Enter water rate: ";
            getline(cin, waterRate);
            cout << "Enter bloom rate: ";
            getline(cin, bloomRate);
            cout << "Enter age of plant (in years): ";
            cin >> age;
            cout << "Enter purchase date (day month year): ";
            cin >> purchaseDay >> purchaseMonth >> purchaseYear;
            cin.ignore();
            cout << "Enter any additional note: ";
            getline(cin, note);

            // Create a Plant object and save it to the database
            Plant plant(name, waterRate, bloomRate, age, purchaseDay, purchaseMonth, purchaseYear, note);
            plant.saveToDatabase(db);
        }
        else if (choice == 2)
        { // Update an existing plant
            int plantID, age, purchaseDay, purchaseMonth, purchaseYear;
            string name, waterRate, bloomRate, note;

            // Collect new data to update the plant
            cout << "Enter plant ID to update: ";
            cin >> plantID;
            cin.ignore();
            cout << "Enter new plant name: ";
            getline(cin, name);
            cout << "Enter new water rate: ";
            getline(cin, waterRate);
            cout << "Enter new bloom rate: ";
            getline(cin, bloomRate);
            cout << "Enter new age of plant (in years): ";
            cin >> age;
            cout << "Enter new purchase date (day month year): ";
            cin >> purchaseDay >> purchaseMonth >> purchaseYear;
            cin.ignore();
            cout << "Enter new note: ";
            getline(cin, note);

            // Update the plant in the database using the collected data
            updatePlant(db, plantID, name, waterRate, bloomRate, age, purchaseDay, purchaseMonth, purchaseYear, note);
        }
        else if (choice == 3)
        { // Delete a plant
            int plantID;
            cout << "Enter plant ID to delete: ";
            cin >> plantID;

            // Delete the specified plant from the database
            deletePlant(db, plantID);

            // Reset plant IDs after deletion to maintain sequential ordering
            resetPlantIDs(db);
        }
        else if (choice == 4)
        { // Search plant by name
            string name;
            cout << "Enter plant name to search: ";
            getline(cin, name);

            // Search for the plant by name in the database
            searchPlantByName(db, name);
        }
        else if (choice == 5)
        { // List plants by purchase year
            int year;
            cout << "Enter purchase year to filter plants: ";
            cin >> year;

            // List plants that match the specified purchase year
            listPlantsByPurchaseYear(db, year);
        }
        else if (choice == 6)
        { // Display unique water rates
            displayUniqueWaterRates(db);
        }
        else if (choice == 7)
        { // Display all plants
            loadAndDisplayPlants(db);
        }
        else if (choice == 8)
        { // Delete all records
            deleteAllRecords(db);
        }
        else if (choice == 9)
        { // Show recommended plants
            cout << "\n\nThere is some recommended plant that I suggest you to plant\n\n";
            RecommendPlants(); // Call the RecommendPlants function
        }
        else if (choice == 10)
        { // Exit the program
            break;
        }
        else
        { // Handle invalid choices
            cout << "Invalid choice, please try again." << endl;
        }
    }

    sqlite3_close(db); // Close the database before exiting
    return 0;
}