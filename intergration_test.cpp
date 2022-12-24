#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include "vector.hpp"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

using namespace std;

enum Test {PASS, DIF_SIZE, DIF_RES, MISSING_FILE};

void result(Test test, string info=""){
    // Print the result
    cout << BOLDWHITE << "Test result: " << RESET;
    switch (test){
        case PASS:
            cout << BOLDGREEN << "PASS" << RESET << endl;
            break;
        case DIF_SIZE:
            cout << BOLDRED << "Different size result" << RESET << endl;
            break;
        case DIF_RES:
            cout << BOLDRED << "Different results" << RESET << endl;
            break;
        case MISSING_FILE:
            cout << BOLDYELLOW << "Missing file " << RESET << info << endl;
            break;
    }
    exit(0);
}


int main(){

    // Files
    string filename1 = "output.txt";
    string filename2 = "./workloads/small/custom.result";


    // Parse first file
    ifstream file1(filename1);
    if (!file1.is_open()) result(MISSING_FILE, filename1);
    string str1;
    Vector<string> v1 = Vector<string>();
    while (getline(file1, str1)){
        if (str1 != "") v1.push(str1);
    }

    // Parse second file
    ifstream file2(filename2);
    if (!file2.is_open()) result(MISSING_FILE, filename2);
    string str2;
    Vector<string> v2 = Vector<string>();
    while (getline(file2, str2)){
        if (str2 != "") v2.push(str2);
    }

    // Sort both vectors
    // v1.sort();
    // v2.sort();


    Test test = PASS;

    // Compare the vectors
    if (v1.get_size() != v2.get_size()){
        result(DIF_SIZE);
    }

    if (test == PASS){
        for (uint i = 0; i < v1.get_size(); i++){
            if (v1.get(i) != v2.get(i)){
                result(DIF_RES);
            }
        }
    }

    result(PASS);


}