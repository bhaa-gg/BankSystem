#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <conio.h>


using namespace std;



//===================================== Person ==============================================

class Person {
protected:
    string name;
    string password;
    int id;
public:
    Person(int _id, string _name, string _pass) : id(_id), name(_name), password(_pass) {}

    // Setters
    void setId(int _id) { id = _id; }
    void setName(string _name) { name = _name; }
    void setPassword(string _pass) { password = _pass; }

    // Getters
    int getId() const { return id; }
    string getName() const { return name; }
    string getPassword() const { return password; }

    // Virtual functions
    virtual void printDetails() const = 0; ///< Displays detailed information
    virtual double calculateBenefits() const = 0; ///< Calculates benefits (abstract)
    virtual void applyMonthlyUpdate() = 0; ///< Applies monthly updates
    virtual void display() const = 0; ///< Displays basic information
    virtual void updateFile() = 0; ///< Updates user data in file

    virtual ~Person() {} ///< Virtual destructor
};

//===================================== Visa ==============================================

class Visa {
private:
    double balance;
    int transferCount;
    int visaId;
public:
    Visa(int _visaId, double _balance) : visaId(_visaId), balance(_balance), transferCount(0) {}

    void setBalance(double _balance) { balance += _balance; }
    double getBalance() const { return balance; }
    int getTransferCount() const { return transferCount; }
    int getVisaId() const { return visaId; }

    void incrementTransfer() { transferCount++; }

    void display() const {
        cout << "Visa ID: " << visaId << " | Balance: " << balance
            << " | Number of Transfers: " << transferCount << endl;
    }
};


//===================================== Transaction Struct =================================

enum class TransactionType { Deposit, Withdraw, TransferTo, TransferFrom, Interest };


string to_string(TransactionType type) {
    switch (type) {
    case TransactionType::Deposit: return "Deposit";
    case TransactionType::Withdraw: return "Withdraw";
    case TransactionType::TransferTo: return "TransferTo";
    case TransactionType::TransferFrom: return "TransferFrom";
    case TransactionType::Interest: return "Interest";
    default: return "Unknown";
    }
}

struct Transaction {
    double amount;
    TransactionType type;
    int recipientId;
    string name;

    Transaction(double _amount, TransactionType _type, string _name="", int _recipientId = -1)
        : amount(_amount), type(_type), recipientId(_recipientId), name(_name){}
};

//======================================= Client ============================================
class Client : public Person {
    Visa visa;
    double balance;
    vector<Transaction> transactionHistory;
public:
    Client(int _id, string _name, string _pass, double _balance)
        : Person(_id, _name, _pass), visa(_id, _balance) {
        if (_balance >= 1500)
            balance = _balance;
        else
            balance = 1500;
        visa.setBalance(balance);
    }

    void setBalance(double _balance) {
        if (_balance >= 1500)
            balance = _balance;
        visa.setBalance(balance);
    }
    double getBalance() const { return balance; }
    Visa& getVisa() { return visa; }

    void deposit(double amount) {
        if (amount <= 0) throw runtime_error("Deposit amount must be positive");
        balance += amount;
        visa.setBalance(balance);
        transactionHistory.emplace_back(amount, TransactionType::Deposit,name);
    }

    void withdraw(double amount) {
        if (amount <= 0) throw runtime_error("Withdrawal amount must be positive");
        if (balance >= amount) {
            balance -= amount;
            visa.setBalance(balance);
            transactionHistory.emplace_back(amount, TransactionType::Withdraw, name);
        }
        else {
            throw runtime_error("Insufficient balance for withdrawal");
        }
    }

    void transferTo(Client* other, double amount) {
        if (amount <= 0) throw runtime_error("Transfer amount must be positive");
        if (balance >= amount) {
            balance -= amount;
            other->balance += amount;
            visa.setBalance(balance);
            other->visa.setBalance(other->balance);
            visa.incrementTransfer();
            transactionHistory.emplace_back(amount, TransactionType::TransferTo, other->getName(), other->getId());
            other->transactionHistory.emplace_back(amount, TransactionType::TransferFrom, name, id);
            cout << "Transfer successful.\n";
            updateFile();
            other->updateFile();
        }
        else {
            throw runtime_error("Insufficient balance for transfer");
        }
    }

    void printDetails() const override {
        cout << "Client Details:\n";
        display();
        cout << "Transaction History:\n";
        for (const auto& t : transactionHistory) {
            cout << "Type: " << to_string(t.type) << ", Amount: " << t.amount;
			cout << ", Name: " << t.name;
            if (t.recipientId != -1) cout << ", Recipient ID: " << t.recipientId;
            cout << endl;
        }
    }

    double calculateBenefits() const override {
        return balance * 0.02; // 2% annual interest, monthly portion
    }

    void applyMonthlyUpdate() override {
        double interest = calculateBenefits() / 12; // Monthly interest
        balance += interest;
        visa.setBalance(balance);
        transactionHistory.emplace_back(interest, TransactionType::Interest);
        updateFile();
    }

    void display() const override {
        cout << "ID: " << id << " Name: " << name << " Balance: " << balance << endl;
        visa.display();
    }

    void saveToFile() {
        ofstream file("Clients.txt", ios::app);
        if (!file.is_open()) throw runtime_error("Unable to open Clients.txt for writing");
        file << id << "," << name << "," << password << "," << balance
            << "," << visa.getTransferCount() << endl;
        file.close();
    }

    void updateFile() override {
        ifstream inFile("Clients.txt");
        if (!inFile.is_open()) throw runtime_error("Unable to open Clients.txt for reading");
        ofstream tempFile("temp.txt");
        if (!tempFile.is_open()) {
            inFile.close();
            throw runtime_error("Unable to open temp.txt for writing");
        }

        string line;
        bool found = false;
        while (getline(inFile, line)) {
            stringstream ss(line);
            string tempId, tempName, tempPass, tempBalance, tempTransferCount;
            getline(ss, tempId, ',');
            getline(ss, tempName, ',');
            getline(ss, tempPass, ',');
            getline(ss, tempBalance, ',');
            getline(ss, tempTransferCount, ',');

            try {
                if (stoi(tempId) == id) {
                    tempFile << id << "," << name << "," << password << ","
                        << balance << "," << visa.getTransferCount() << endl;
                    found = true;
                }
                else {
                    tempFile << line << endl;
                }
            }
            catch (const exception& e) {
                cout << "Error parsing client data: " << e.what() << endl;
            }
        }

        if (!found) {
            tempFile << id << "," << name << "," << password << ","
                << balance << "," << visa.getTransferCount() << endl;
        }

        inFile.close();
        tempFile.close();

        remove("Clients.txt");
        rename("temp.txt", "Clients.txt");
    }
};

//======================================= Employee ============================================
class Employee : public Person {
    double salary;
public:
    Employee(int _id, string _name, string _pass, double _salary)
        : Person(_id, _name, _pass) {
        if (_salary >= 5000)
            salary = _salary;
        else
            salary = 5000;
    }

    void setSalary(double _salary) {
        if (_salary >= 5000)
            salary = _salary;
    }
    double getSalary() const { return salary; }

    void printDetails() const override {
        cout << "Employee Details:\n";
        display();
        cout << "Monthly Benefits: " << calculateBenefits() << endl;
    }

    double calculateBenefits() const override {
        return salary * 0.1; // 10% bonus
    }

    void applyMonthlyUpdate() override {
        // Employees receive salary, no file update needed unless salary changes
    }

    void display() const override {
        cout << "ID: " << id << " Name: " << name << " Salary: " << salary << endl;
    }

    void saveToFile() {
        ofstream file("Employees.txt", ios::app);
        if (!file.is_open()) throw runtime_error("Unable to open Employees.txt for writing");
        file << id << "," << name << "," << password << "," << salary << endl;
        file.close();
    }

    void updateFile() override {
        ifstream inFile("Employees.txt");
        if (!inFile.is_open()) throw runtime_error("Unable to open Employees.txt for reading");
        ofstream tempFile("temp.txt");
        if (!tempFile.is_open()) {
            inFile.close();
            throw runtime_error("Unable to open temp.txt for writing");
        }

        string line;
        bool found = false;
        while (getline(inFile, line)) {
            stringstream ss(line);
            string tempId, tempName, tempPass, tempSalary;
            getline(ss, tempId, ',');
            getline(ss, tempName, ',');
            getline(ss, tempPass, ',');
            getline(ss, tempSalary, ',');

            try {
                if (stoi(tempId) == id) {
                    tempFile << id << "," << name << "," << password << ","
                        << salary << endl;
                    found = true;
                }
                else {
                    tempFile << line << endl;
                }
            }
            catch (const exception& e) {
                cout << "Error parsing employee data: " << e.what() << endl;
            }
        }

        if (!found) {
            tempFile << id << "," << name << "," << password << ","
                << salary << endl;
        }

        inFile.close();
        tempFile.close();

        remove("Employees.txt");
        rename("temp.txt", "Employees.txt");
    }

    void addClient(Client& c) {
        c.saveToFile();
    }
};

//===================================================================================
 bool isIdUnique(int id, const string& fileName) {
    ifstream file(fileName);
    if (!file.is_open()) return true; // File doesn't exist yet
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string tempId;
        getline(ss, tempId, ',');
        try {
            if (stoi(tempId) == id) {
                file.close();
                return false;
            }
        }
        catch (const exception& e) {
            cout << "Error parsing ID in " << fileName << ": " << e.what() << endl;
        }
    }
    file.close();
    return true;
}

//======================================= Admin ============================================
class Admin : public Employee {
public:
    Admin(int _id, string _name, string _pass, double _salary)
        : Employee(_id, _name, _pass, _salary) {}

    void addEmployee(Employee& e) {
        if (isIdUnique(e.getId(), "Employees.txt")) {
            e.saveToFile();
        }
        else {
            throw runtime_error("Employee ID already exists");
        }
    }

    void addClient(Client& c) {
        if (isIdUnique(c.getId(), "Clients.txt")) {
            c.saveToFile();
        }
        else {
            throw runtime_error("Client ID already exists");
        }
    }

    void addAdmin(Admin& a) {
        if (isIdUnique(a.getId(), "Admins.txt")) {
            ofstream file("Admins.txt", ios::app);
            if (!file.is_open()) throw runtime_error("Unable to open Admins.txt for writing");
            file << a.getId() << "," << a.getName() << "," << a.getPassword() << "," << a.getSalary() << endl;
            file.close();
        }
        else {
            throw runtime_error("Admin ID already exists");
        }
    }

    void printDetails() const override {
        cout << "Admin Details:\n";
        display();
        cout << "Monthly Benefits: " << calculateBenefits() << endl;
    }

    double calculateBenefits() const override {
        return getSalary() * 0.15; // 15% bonus for admins
    }

    void applyMonthlyUpdate() override {
        // Admins receive salary, no file update needed unless salary changes
    }
};

//======================================= FileHelper ============================================
class FileHelper {
public:
    static void clearFile(const string& fileName) {
        ofstream file(fileName, ios::trunc);
        if (!file.is_open()) throw runtime_error("Unable to clear file: " + fileName);
        file.close();
    }

    static void getAllClients() {
        ifstream file("Clients.txt");
        if (!file.is_open()) throw runtime_error("Unable to open Clients.txt for reading");
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    static void getAllEmployees() {
        ifstream file("Employees.txt");
        if (!file.is_open()) throw runtime_error("Unable to open Employees.txt for reading");
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    static void getAllAdmins() {
        ifstream file("Admins.txt");
        if (!file.is_open()) throw runtime_error("Unable to open Admins.txt for reading");
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }
};

//======================================= Screens ============================================
class Screens {
public:
    static void bankName() {
        cout << "\n================ BANK SYSTEM ================\n";
    }
    static void welcome() {
        cout << "\nWelcome to the Bank System!\n";
    }
    static void loginOptions() {
        cout << "1. Client\n2. Employee\n3. Admin\n4. Exit\nEnter your choice: ";
    }
    static void invalid() {
        cout << "Invalid login credentials!\n";
    }
    static void logout() {
        cout << "\nLogged out successfully.\n";
    }

    static string getPassword() {
        string password;
        char ch;
        while (true) {
            ch = _getch();
            if (ch == 13) { 
                cout << endl;
                break;
            }
            else if (ch == 8) { 
                if (!password.empty()) {
                    password.pop_back();
                    cout << "\b \b";
                }
            }
            else if (isprint(ch)) {
                password += ch;
                cout << '*';
            }
        }
        return password;
    }
};

//======================================= ClientManager ============================================
class ClientManager {
public:
    static Client* findClientById(int id) {
        ifstream file("Clients.txt");
        if (!file.is_open()) throw runtime_error("Unable to open Clients.txt for reading");
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string tempId, tempName, tempPass, tempBalance, tempTransferCount;
            getline(ss, tempId, ',');
            getline(ss, tempName, ',');
            getline(ss, tempPass, ',');
            getline(ss, tempBalance, ',');
            getline(ss, tempTransferCount, ',');
            try {
                if (stoi(tempId) == id) {
                    file.close();
                    Client* client = new Client(stoi(tempId), tempName, tempPass, stod(tempBalance));
                    for (int i = 0; i < stoi(tempTransferCount); i++) {
                        client->getVisa().incrementTransfer();
                    }
                    return client;
                }
            }
            catch (const exception& e) {
                cout << "Error parsing client data: " << e.what() << endl;
            }
        }
        file.close();
        return nullptr;
    }

    static void clientMenu(Client* client) {
        int choice;
        do {
            cout << "\n1. Check Balance\n2. Deposit\n3. Withdraw\n4. Transfer Money\n"
                << "5. View Visa Info\n6. View Transaction History\n7. Logout\nChoice: ";
            cin >> choice;
            try {
                if (choice == 1) {
                    client->display();
                }
                else if (choice == 2) {
                    double amount;
                    cout << "Amount to deposit: ";
                    cin >> amount;
                    client->deposit(amount);
                }
                else if (choice == 3) {
                    double amount;
                    cout << "Amount to withdraw: ";
                    cin >> amount;
                    client->withdraw(amount);
                }
                else if (choice == 4) {
                    int recipientId;
                    double amount;
                    cout << "Enter recipient Client ID: ";
                    cin >> recipientId;
                    cout << "Enter amount to transfer: ";
                    cin >> amount;
                    if (recipientId == client->getId()) {
                        cout << "Cannot transfer to yourself!\n";
                    }
                    else {
                        Client* recipient = findClientById(recipientId);
                        if (recipient) {
                            client->transferTo(recipient, amount);
                            delete recipient;
                        }
                        else {
                            cout << "Recipient client not found!\n";
                        }
                    }
                }
                else if (choice == 5) {
                    client->getVisa().display();
                }
                else if (choice == 6) {
                    client->printDetails();
                }
            }
            catch (const runtime_error& e) {
                cout << "Error: " << e.what() << endl;
            }
        } while (choice != 7);
        client->updateFile();
        Screens::logout();
    }
};

//======================================= EmployeeManager ============================================
class EmployeeManager {
public:
    static Client* findClientById(int id) {
        return ClientManager::findClientById(id);
    }

    static void employeeMenu(Employee* employee) {
        int choice;
        do {
            cout << "\n1. Display Info\n2. Add Client\n3. All Employees\n4. All Clients\n"
                << "5. Search Client by ID\n6. Edit Client Info\n7. Apply Monthly Update\n8. Logout\nChoice: ";
            cin >> choice;
            try {
                if (choice == 1) {
                    employee->printDetails();
                }
                else if (choice == 2) {
                    int id;
                    string name, password;
                    double balance;
                    cout << "Enter Client ID: ";
                    cin >> id;
                    cout << "Enter Client Name: ";
                    cin >> name;
                    cout << "Enter Client Password: ";
                    cin >> password;
                    cout << "Enter Client Balance: ";
                    cin >> balance;
                    Client newClient(id, name, password, balance);
                    employee->addClient(newClient);
                    cout << "Client added successfully!\n";
                }
                else if (choice == 3) {
                    cout << "All The Employees:\n";
                    FileHelper::getAllEmployees();
                }
                else if (choice == 4) {
                    cout << "All The Clients:\n";
                    FileHelper::getAllClients();
                }
                else if (choice == 5) {
                    int clientId;
                    cout << "Enter Client ID to search: ";
                    cin >> clientId;
                    Client* client = findClientById(clientId);
                    if (client) {
                        cout << "Client found:\n";
                        client->printDetails();
                        delete client;
                    }
                    else {
                        cout << "Client not found!\n";
                    }
                }
                else if (choice == 6) {
                    int clientId;
                    cout << "Enter Client ID to edit: ";
                    cin >> clientId;
                    Client* client = findClientById(clientId);
                    if (client) {
                        string newName, newPassword;
                        double newBalance;
                        cout << "Enter new Client Name: ";
                        cin >> newName;
                        cout << "Enter new Client Password: ";
                        cin >> newPassword;
                        cout << "Enter new Client Balance: ";
                        cin >> newBalance;
                        client->setName(newName);
                        client->setPassword(newPassword);
                        client->setBalance(newBalance);
                        client->updateFile();
                        cout << "Client information updated successfully!\n";
                        delete client;
                    }
                    else {
                        cout << "Client not found!\n";
                    }
                }
                else if (choice == 7) {
                    employee->applyMonthlyUpdate();
                    cout << "Monthly update applied.\n";
                }
            }
            catch (const runtime_error& e) {
                cout << "Error: " << e.what() << endl;
            }
        } while (choice != 8);
        Screens::logout();
    }
};

//======================================= AdminManager ============================================
class AdminManager {
public:
    static Employee* findEmployeeById(int id) {
        ifstream file("Employees.txt");
        if (!file.is_open()) throw runtime_error("Unable to open Employees.txt for reading");
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string tempId, tempName, tempPass, tempSalary;
            getline(ss, tempId, ',');
            getline(ss, tempName, ',');
            getline(ss, tempPass, ',');
            getline(ss, tempSalary, ',');
            try {
                if (stoi(tempId) == id) {
                    file.close();
                    return new Employee(stoi(tempId), tempName, tempPass, stod(tempSalary));
                }
            }
            catch (const exception& e) {
                cout << "Error parsing employee data: " << e.what() << endl;
            }
        }
        file.close();
        return nullptr;
    }

    static void generateReport() {
        ifstream file("Clients.txt");
        if (!file.is_open()) throw runtime_error("Unable to open Clients.txt for reading");
        double totalBalance = 0;
        int clientCount = 0;
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string tempId, tempName, tempPass, tempBalance, tempTransferCount;
            getline(ss, tempId, ',');
            getline(ss, tempName, ',');
            getline(ss, tempPass, ',');
            getline(ss, tempBalance, ',');
            getline(ss, tempTransferCount, ',');
            try {
                totalBalance += stod(tempBalance);
                clientCount++;
            }
            catch (const exception& e) {
                cout << "Error parsing client data: " << e.what() << endl;
            }
        }
        file.close();
        cout << "Bank Report:\n";
        cout << "Total Clients: " << clientCount << "\n";
        cout << "Total Balance: " << totalBalance << "\n";
    }

    static void adminMenu(Admin* admin) {
        int choice;
        do {
            cout << "\n1. Display Info\n2. Add Employee\n3. Add Client\n4. Add Admin\n"
                << "5. Search Employee by ID\n6. Edit Employee Info\n7. All Admins\n"
                << "8. Generate Report\n9. Apply Monthly Update\n10. Logout\nChoice: ";
            cin >> choice;
            try {
                if (choice == 1) {
                    admin->printDetails();
                }
                else if (choice == 2) {
                    int id;
                    string name, password;
                    double salary;
                    cout << "Enter Employee ID: ";
                    cin >> id;
                    cout << "Enter Employee Name: ";
                    cin >> name;
                    cout << "Enter Employee Password: ";
                    cin >> password;
                    cout << "Enter Employee Salary: ";
                    cin >> salary;
                    Employee newEmployee(id, name, password, salary);
                    admin->addEmployee(newEmployee);
                    cout << "Employee added successfully!\n";
                }
                else if (choice == 3) {
                    int id;
                    string name, password;
                    double balance;
                    cout << "Enter Client ID: ";
                    cin >> id;
                    cout << "Enter Client Name: ";
                    cin >> name;
                    cout << "Enter Client Password: ";
                    cin >> password;
                    cout << "Enter Client Balance: ";
                    cin >> balance;
                    Client newClient(id, name, password, balance);
                    admin->addClient(newClient);
                    cout << "Client added successfully!\n";
                }
                else if (choice == 4) {
                    int id;
                    string name, password;
                    double salary;
                    cout << "Enter Admin ID: ";
                    cin >> id;
                    cout << "Enter Admin Name: ";
                    cin >> name;
                    cout << "Enter Admin Password: ";
                    cin >> password;
                    cout << "Enter Admin Salary: ";
                    cin >> salary;
                    Admin newAdmin(id, name, password, salary);
                    admin->addAdmin(newAdmin);
                    cout << "Admin added successfully!\n";
                }
                else if (choice == 5) {
                    int empId;
                    cout << "Enter Employee ID to search: ";
                    cin >> empId;
                    Employee* emp = findEmployeeById(empId);
                    if (emp) {
                        cout << "Employee found:\n";
                        emp->printDetails();
                        delete emp;
                    }
                    else {
                        cout << "Employee not found!\n";
                    }
                }
                else if (choice == 6) {
                    int empId;
                    cout << "Enter Employee ID to edit: ";
                    cin >> empId;
                    Employee* emp = findEmployeeById(empId);
                    if (emp) {
                        string newName, newPassword;
                        double newSalary;
                        cout << "Enter new Employee Name: ";
                        cin >> newName;
                        cout << "Enter new Employee Password: ";
                        cin >> newPassword;
                        cout << "Enter new Employee Salary: ";
                        cin >> newSalary;
                        emp->setName(newName);
                        emp->setPassword(newPassword);
                        emp->setSalary(newSalary);
                        emp->updateFile();
                        cout << "Employee information updated successfully!\n";
                        delete emp;
                    }
                    else {
                        cout << "Employee not found!\n";
                    }
                }
                else if (choice == 7) {
                    cout << "All The Admins:\n";
                    FileHelper::getAllAdmins();
                }
                else if (choice == 8) {
                    generateReport();
                }
                else if (choice == 9) {
                    admin->applyMonthlyUpdate();
                    cout << "Monthly update applied.\n";
                }
            }
            catch (const runtime_error& e) {
                cout << "Error: " << e.what() << endl;
            }
        } while (choice != 10);
        Screens::logout();
    }
};

//======================================= Login Functions ============================================
Client* clientLogin(int id, string password) {
    ifstream file("Clients.txt");
    if (!file.is_open()) throw runtime_error("Unable to open Clients.txt for reading");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string tempId, tempName, tempPass, tempBalance, tempTransferCount;
        getline(ss, tempId, ',');
        getline(ss, tempName, ',');
        getline(ss, tempPass, ',');
        getline(ss, tempBalance, ',');
        getline(ss, tempTransferCount, ',');
        try {
            if (stoi(tempId) == id && tempPass == password) {
                file.close();
                Client* client = new Client(stoi(tempId), tempName, tempPass, stod(tempBalance));
                for (int i = 0; i < stoi(tempTransferCount); i++) {
                    client->getVisa().incrementTransfer();
                }
                return client;
            }
        }
        catch (const exception& e) {
            cout << "Error parsing client data: " << e.what() << endl;
        }
    }
    file.close();
    return nullptr;
}

Employee* employeeLogin(int id, string password) {
    ifstream file("Employees.txt");
    if (!file.is_open()) throw runtime_error("Unable to open Employees.txt for reading");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string tempId, tempName, tempPass, tempSalary;
        getline(ss, tempId, ',');
        getline(ss, tempName, ',');
        getline(ss, tempPass, ',');
        getline(ss, tempSalary, ',');
        try {
            if (stoi(tempId) == id && tempPass == password) {
                file.close();
                return new Employee(stoi(tempId), tempName, tempPass, stod(tempSalary));
            }
        }
        catch (const exception& e) {
            cout << "Error parsing employee data: " << e.what() << endl;
        }
    }
    file.close();
    return nullptr;
}

Admin* adminLogin(int id, string password) {
    ifstream file("Admins.txt");
    if (!file.is_open()) throw runtime_error("Unable to open Admins.txt for reading");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string tempId, tempName, tempPass, tempSalary;
        getline(ss, tempId, ',');
        getline(ss, tempName, ',');
        getline(ss, tempPass, ',');
        getline(ss, tempSalary, ',');
        try {
            if (stoi(tempId) == id && tempPass == password) {
                file.close();
                return new Admin(stoi(tempId), tempName, tempPass, stod(tempSalary));
            }
        }
        catch (const exception& e) {
            cout << "Error parsing admin data: " << e.what() << endl;
        }
    }
    file.close();
    return nullptr;
}

//======================================= RunApp ============================================
class RunApp {
public:
    static void run() {
        Screens::bankName();
        Screens::welcome();

        while (true) {
            Screens::loginOptions();
            int userType;
            cin >> userType;

            if (userType == 4) {
                cout << "\nBank system terminated.\n";
                break;
            }

            int id;
            string pass;
            cout << "Enter ID: ";
            cin >> id;
            cout << "Enter Password: ";
            pass = Screens::getPassword();

            try {
                switch (userType) {
                case 1: {
                    Client* c = clientLogin(id, pass);
                    if (c != nullptr) {
                        cout << "Client login successful!\n";
                        ClientManager::clientMenu(c);
                        delete c;
                    }
                    else {
                        Screens::invalid();
                    }
                    break;
                }
                case 2: {
                    Employee* e = employeeLogin(id, pass);
                    if (e != nullptr) {
                        cout << "Employee login successful!\n";
                        EmployeeManager::employeeMenu(e);
                        delete e;
                    }
                    else {
                        Screens::invalid();
                    }
                    break;
                }
                case 3: {
                    Admin* a = adminLogin(id, pass);
                    if (a != nullptr) {
                        cout << "Admin login successful!\n";
                        AdminManager::adminMenu(a);
                        delete a;
                    }
                    else {
                        Screens::invalid();
                    }
                    break;
                }
                default:
                    cout << "Invalid choice!\n";
                    break;
                }
            }
            catch (const runtime_error& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
    }
};

//======================================= Main ============================================
int main() {
    try {
        FileHelper::clearFile("Clients.txt");
        FileHelper::clearFile("Employees.txt");
        FileHelper::clearFile("Admins.txt");

        Admin a1(1, "Alice Admin", "1234", 8000);
        Employee e1(1, "John Doe", "123", 6000);
        Client c1(1, "Ahmed", "1", 9000);
        Client c2(2, "Sara", "2", 4000);
        Client c3(3, "Adam", "3", 5000);

        a1.addAdmin(a1);
        a1.addEmployee(e1);
        a1.addClient(c1);
        e1.addClient(c2);
        e1.addClient(c3);

        RunApp::run();
    }
    catch (const runtime_error& e) {
        cout << "Fatal error: " << e.what() << endl;
        return 1;
    }

   
}