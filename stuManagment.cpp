#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

const int AES_KEY_SIZE = 256;
const int AES_BLOCK_SIZE = 128;
const int SALT_SIZE = 32;
const int ITERATIONS = 10000;

class User {
private:
    string username;
    vector<unsigned char> salt;
    vector<unsigned char> passwordHash;

public:
    User(const string& username, const string& password)
        : username(username) {
        generateSalt();
        hashPassword(password);
    }

    string getUsername() const {
        return username;
    }

    vector<unsigned char> getSalt() const {
        return salt;
    }

    vector<unsigned char> getPasswordHash() const {
        return passwordHash;
    }

    void generateSalt() {
        salt.resize(SALT_SIZE);
        RAND_bytes(&salt[0], SALT_SIZE);
    }

    void hashPassword(const string& password) {
        vector<unsigned char> key(AES_KEY_SIZE / 8, 0);
        PKCS5_PBKDF2_HMAC(password.c_str(), -1, &salt[0], salt.size(), ITERATIONS, EVP_sha256(), key.size(), &key[0]);
        passwordHash = key;
    }
};

class Student {
private:
    string rollNum;
    string name;
    string fName;
    string address;

public:
    string encryptData(const vector<unsigned char>& key) {
        string dataToEncrypt = rollNum + '*' + name + '*' + fName + '*' + address;
        EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(context);
        EVP_EncryptInit_ex(context, EVP_aes_256_cbc(), nullptr, &key[0], nullptr);

        int maxOutputSize = dataToEncrypt.size() + AES_BLOCK_SIZE;
        vector<unsigned char> encryptedData(maxOutputSize);

        int encryptedSize = 0;
        EVP_EncryptUpdate(context, &encryptedData[0], &encryptedSize, (const unsigned char*)&dataToEncrypt[0], dataToEncrypt.size());

        int finalEncryptedSize = 0;
        EVP_EncryptFinal_ex(context, &encryptedData[encryptedSize], &finalEncryptedSize);
        encryptedSize += finalEncryptedSize;

        EVP_CIPHER_CTX_free(context);

        string base64EncryptedData = base64_encode(&encryptedData[0], encryptedSize);
        return base64EncryptedData;
    }

    static Student decryptData(const string& encryptedData, const vector<unsigned char>& key) {
        vector<unsigned char> decodedData = base64_decode(encryptedData);
        EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(context);
        EVP_DecryptInit_ex(context, EVP_aes_256_cbc(), nullptr, &key[0], nullptr);

        int maxOutputSize = decodedData.size() + AES_BLOCK_SIZE;
        vector<unsigned char> decryptedData(maxOutputSize);

        int decryptedSize = 0;
        EVP_DecryptUpdate(context, &decryptedData[0], &decryptedSize, &decodedData[0], decodedData.size());

        int finalDecryptedSize = 0;
        EVP_DecryptFinal_ex(context, &decryptedData[decryptedSize], &finalDecryptedSize);
        decryptedSize += finalDecryptedSize;

        EVP_CIPHER_CTX_free(context);

        string decryptedDataStr(decryptedData.begin(), decryptedData.begin() + decryptedSize);
        stringstream ss(decryptedDataStr);
        string rollNum, name, fName, address;
        getline(ss, rollNum, '*');
        getline(ss, name, '*');
        getline(ss, fName, '*');
        getline(ss, address, '*');

        return Student(rollNum, name, fName, address);
    }
};

class StudentDatabase {
private:
    vector<Student> students;
    fstream file;
    User currentUser;

public:
    StudentDatabase(const User& user)
        : currentUser(user) {
        file.open("stuData.txt", ios::in | ios::out | ios::app);
    }

    ~StudentDatabase() {
        file.close();
    }

    bool login(const string& username, const string& password) {
        if (username == currentUser.getUsername()) {
            User tempUser(username, password);
            if (tempUser.getPasswordHash() == currentUser.getPasswordHash()) {
                return true;
            }
        }
        return false;
    }

    void addStudent() {
        if (currentUser.getUsername().empty()) {
            cout << "Login required for this operation." << endl;
            return;
        }

        string rollNum, name, fName, address;
        cin.ignore();
        cout << "Enter Student Roll Number: ";
        getline(cin, rollNum);
        cout << "Enter Student Name: ";
        getline(cin, name);
        cout << "Enter Student Father Name: ";
        getline(cin, fName);
        cout << "Enter Student Address: ";
        getline(cin, address);

        Student student(rollNum, name, fName, address);
        students.push_back(student);

        string encryptedData = student.encryptData(currentUser.getPasswordHash());
        file << encryptedData << endl;
    }
};

int main() {
    User user("admin", "adminpassword");
    StudentDatabase studentDB(user);

    char choice;

    cout << "Enter Username: ";
    string username;
    cin >> username;

    cout << "Enter Password: ";
    string password;
    cin >> password;

    if (studentDB.login(username, password)) {
        do {
            cout << "---------------------------" << endl;
            cout << "1- Add Student Record" << endl;
            cout << "2- View All Student Records" << endl;
            cout << "3- Search Student Record" << endl;
            cout << "4- Exit" << endl;
            cout << "---------------------------" << endl;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case '1':
                    studentDB.addStudent();
                    break;
                case '2':
                    studentDB.viewAllStudents();
                    break;
                case '3':
                    studentDB.searchStudent();
                    break;
                case '4':
                    return 0;
                default:
                    cout << "Invalid Choice...!" << endl;
            }
        } while (true);
    } else {
        cout << "Login failed. Incorrect credentials." << endl;
    }

    return 0;
}