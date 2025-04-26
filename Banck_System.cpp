#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio> // إضافة مكتبة cstdio لـ remove و rename
using namespace std;

//===================================== Person ==============================================

class Person {
protected:
    int id;
    string name;
    string password;
public:
    Person() {}
    Person(int _id, string _name, string _pass) {
        id = _id;
        name = _name;
        password = _pass;
    }

    void setId(int _id) { id = _id; }
    void setName(string _name) { name = _name; }
    void setPassword(string _pass) { password = _pass; }
    int getId() { return id; }
    string getName() { return name; }
    string getPassword() { return password; }
};

//======================================= Client ============================================

class Client : public Person {
    double balance;
public:
    Client() {}
    Client(int _id, string _name, string _pass, double _balance) : Person(_id, _name, _pass) {
        if (_balance >= 1500)
            balance = _balance;
        else
            balance = 1500;
    }

    void setBalance(double _balance) {
        if (_balance >= 1500)
            balance = _balance;
    }
    double getBalance() { return balance; }

    void deposit(double amount) { balance += amount; }
    void withdraw(double amount) {
        if (balance >= amount)
            balance -= amount;
    }
    void display() {
        cout << "ID: " << id << " Name: " << name << " Balance: " << balance << endl;
    }

    void transferTo(Client* other, double amount) {
        if (balance >= amount) {
            balance -= amount;
            other->balance += amount;
            cout << "Transfer successful.\n";
            updateFile();         // تحديث رصيد العميل المحوّل
            other->updateFile();  // تحديث رصيد العميل المستقبل
        }
        else {
            cout << "Insufficient balance for transfer.\n";
        }
    }

    void saveToFile() {
        ofstream file("Clients.txt", ios::app);
        if (file.is_open()) {
            file << id << "," << name << "," << password << "," << balance << endl;
            file.close();
        }
    }

    // دالة جديدة لتحديث بيانات العميل في الملف
    void updateFile() {
        ifstream inFile("Clients.txt");
        ofstream tempFile("temp.txt");
        string line;
        bool found = false;

        if (inFile.is_open() && tempFile.is_open()) {
            while (getline(inFile, line)) {
                stringstream ss(line);
                string tempId, tempName, tempPass, tempBalance;
                getline(ss, tempId, ',');
                getline(ss, tempName, ',');
                getline(ss, tempPass, ',');
                getline(ss, tempBalance, ',');

                try {
                    if (stoi(tempId) == id) {
                        // نكتب البيانات المحدّثة للعميل
                        tempFile << id << "," << name << "," << password << "," << balance << endl;
                        found = true;
                    } else {
                        // ننقل السطر زي ما هو
                        tempFile << line << endl;
                    }
                } catch (const exception& e) {
                    cout << "Error parsing client data: " << e.what() << endl;
                }
            }

            // لو العميل مش موجود، نضيفه كسطر جديد
            if (!found) {
                tempFile << id << "," << name << "," << password << "," << balance << endl;
            }

            inFile.close();
            tempFile.close();

            // نمسح الملف القديم ونعيد تسمية الملف المؤقت
            remove("Clients.txt");
            rename("temp.txt", "Clients.txt");
        } else {
            cout << "Error opening files for update.\n";
            if (inFile.is_open()) inFile.close();
            if (tempFile.is_open()) tempFile.close();
        }
    }
};

//======================================= Employee ============================================

class Employee : public Person {
    double salary;
public:
    Employee() {}
    Employee(int _id, string _name, string _pass, double _salary) : Person(_id, _name, _pass) {
        if (_salary >= 5000)
            salary = _salary;
        else
            salary = 5000;
    }

    void setSalary(double _salary) {
        if (_salary >= 5000)
            salary = _salary;
    }
    double getSalary() { return salary; }

    void display() {
        cout << "ID: " << id << " Name: " << name << " Salary: " << salary << endl;
    }

    void saveToFile() {
        ofstream file("Employees.txt", ios::app);
        if (file.is_open()) {
            file << id << "," << name << "," << password << "," << salary << endl;
            file.close();
        }
    }

    void addClient(Client& c) {
        c.saveToFile();
    }
};

//======================================= Admin ============================================

class Admin : public Employee {
public:
    Admin() {}
    Admin(int _id, string _name, string _pass, double _salary) : Employee(_id, _name, _pass, _salary) {}

    void addEmployee(Employee& e) {
        e.saveToFile();
    }
    void addClient(Client& c) {
        c.saveToFile();
    }

    void addAdmin(Admin& a) {
        ofstream file("Admins.txt", ios::app);
        if (file.is_open()) {
            file << a.getId() << "," << a.getName() << "," << a.getPassword() << "," << a.getSalary() << endl;
            file.close();
        }
    }
};

//======================================= FileHelper ============================================
class FileHelper {
public:
    static void clearFile(const string& fileName) {
        ofstream file(fileName, ios::trunc);
        file.close();
    }

    static void getAllClients() {
        ifstream file("Clients.txt");
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    static void getAllEmployees() {
        ifstream file("Employees.txt");
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    static void getAllAdmins() {
        ifstream file("Admins.txt");
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
        cout << "1. Client\n2. Employee\n3. Admin\nEnter your choice: ";
    }
    static void invalid() {
        cout << "Invalid login credentials!\n";
    }
    static void logout() {
        cout << "\nLogged out successfully.\n";
    }
};

//************************************************************************************************
class ClientManager {
public:
    static void clientMenu(Client* client) {
        int choice;
        do {
            cout << "\n1. Check Balance\n2. Deposit\n3. Withdraw\n4. AllClients\n5. Logout\nChoice: ";
            cin >> choice;
            if (choice == 1) client->display();
            else if (choice == 2) {
                double amount; cout << "Amount to deposit: "; cin >> amount;
                client->deposit(amount);
            }
            else if (choice == 3) {
                double amount; cout << "Amount to withdraw: "; cin >> amount;
                client->withdraw(amount);
            }
            else if (choice == 4) {
                cout << "All The Clients:\n";
                FileHelper::getAllClients();

            }
        } while (choice != 5);
        Screens::logout();
    }
};

// فئة جديدة لإدارة قائمة الموظف
class EmployeeManager {
public:
    static void employeeMenu(Employee* employee) {
        int choice;
        do {
            cout << "\n1. Display Info\n2. Add Client\n3. AllEmployees\n4. Logout\nChoice: ";
            cin >> choice;
            if (choice == 1) {
                employee->display();
            }
            else if (choice == 2) {
                int id;
                string name, password;
                double balance;
                cout << "Enter Client ID: "; cin >> id;
                cout << "Enter Client Name: "; cin >> name;
                cout << "Enter Client Password: "; cin >> password;
                cout << "Enter Client Balance: "; cin >> balance;
                Client newClient(id, name, password, balance);
                employee->addClient(newClient);
                cout << "Client added successfully!\n";
            }
            else if (choice == 3) {
                cout << "All The Employees:\n";
                FileHelper::getAllEmployees();

            }
        } while (choice != 4);
        Screens::logout();
    }
};

// فئة جديدة لإدارة قائمة المدير
class AdminManager {
public:
    static void adminMenu(Admin* admin) {
        int choice;
        do {
            cout << "\n1. Display Info\n2. Add Employee\n3. Add Client\n4. Add Admin\n5. AllAdmins\n6. Logout\nChoice: ";
            cin >> choice;
            if (choice == 1) {
                admin->display();
            }
            else if (choice == 2) {
                int id;
                string name, password;
                double salary;
                cout << "Enter Employee ID: "; cin >> id;
                cout << "Enter Employee Name: "; cin >> name;
                cout << "Enter Employee Password: "; cin >> password;
                cout << "Enter Employee Salary: "; cin >> salary;
                Employee newEmployee(id, name, password, salary);
                admin->addEmployee(newEmployee);
                cout << "Employee added successfully!\n";
            }
            else if (choice == 3) {
                int id;
                string name, password;
                double balance;
                cout << "Enter Client ID: "; cin >> id;
                cout << "Enter Client Name: "; cin >> name;
                cout << "Enter Client Password: "; cin >> password;
                cout << "Enter Client Balance: "; cin >> balance;
                Client newClient(id, name, password, balance);
                admin->addClient(newClient);
                cout << "Client added successfully!\n";
            }
            else if (choice == 4) {
                int id;
                string name, password;
                double salary;
                cout << "Enter Admin ID: "; cin >> id;
                cout << "Enter Admin Name: "; cin >> name;
                cout << "Enter Admin Password: "; cin >> password;
                cout << "Enter Admin Salary: "; cin >> salary;
                Admin newAdmin(id, name, password, salary);
                admin->addAdmin(newAdmin);
                cout << "Admin added successfully!\n";
            }
            else if (choice == 5) {
                cout << "All The Admins:\n";
                FileHelper::getAllAdmins();

            }
        } while (choice != 6);
        Screens::logout();
    }
};

//************************************************************************************************

//------------------------------------------------------------------------------------------------------
//الدالة بتدور في ملف 

Client* clientLogin(int id, string password) {
    ifstream file("Clients.txt");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);//,علشان تقدر تفصل بين القيم باستخدام  
        string tempId, tempName, tempPass, tempBalance;
        getline(ss, tempId, ',');
        getline(ss, tempName, ',');
        getline(ss, tempPass, ',');
        getline(ss, tempBalance, ',');
        try {
            if (stoi(tempId) == id && tempPass == password) {
                file.close();
                return new Client(stoi(tempId), tempName, tempPass, stod(tempBalance));
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

//------------------------------------------------------------------------------------------------------

int main() {
    Screens::bankName();
    Screens::welcome();
    Screens::loginOptions();

    int userType;
    cin >> userType;

    int id;
    string pass;

    cout << "Enter ID: "; cin >> id;
    cout << "Enter Password: "; cin >> pass;

    if (userType == 1) {
        Client* c = clientLogin(id, pass);
        if (c != nullptr) {
            cout << "Client login successful!\n";
            ClientManager::clientMenu(c);
            c->saveToFile();
            delete c;  // تحرير الذاكرة
        }
        else {
            Screens::invalid();
        }
    }
    else if (userType == 2) {
        Employee* e = employeeLogin(id, pass);
        if (e != nullptr) {
            cout << "Employee login successful!\n";
            EmployeeManager::employeeMenu(e);  // عرض قائمة الموظف
            delete e;  // تحرير الذاكرة
        }
        else {
            Screens::invalid();
        }
    }
    else if (userType == 3) {
        Admin* a = adminLogin(id, pass);
        if (a != nullptr) {
            cout << "Admin login successful!\n";
            AdminManager::adminMenu(a);  // عرض قائمة المدير
            delete a;  // تحرير الذاكرة
        }
        else {
            Screens::invalid();
        }
    }
    else {
        cout << "Invalid choice!\n";
    }

    // Example of creating Employee and Admin, and adding them to files
    Employee e1(1, "John Doe", "123", 6000);
    Admin a1(2, "Alice Admin", "1234", 8000);
    Client c1(4, "ahmed", "5", 9000);
    Client c2(9, "ahmed", "9", 4000);
    Client c3(10, "adam", "10", 500);
    Client c4(11, "you", "11", 300);
    Client c5(1, "e", "1", 300);

   /* c1.transferTo(&c2,1000);
    c2.transferTo(&c3,1000);
    c3.transferTo(&c4,100);*/

   /* c1.display();  
    c2.display();*/

    // إلغاء التعليق لتتمكن من تسجيل الدخول باستخدام هذه البيانات
    /*e1.addClient(c1);
    a1.addEmployee(e1);
    a1.addAdmin(a1);*/
    /*FileHelper::clearFile("Admins.txt");*/

    /*FileHelper::getAllClients();*/
    /*FileHelper::getAllEmployees();*/
    /*FileHelper::getAllAdmins();*/
}