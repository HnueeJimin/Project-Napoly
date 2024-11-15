#include "header.h"

int gamerule(){
    string line;
    ifstream file("mafiarule.txt");
    if(file.is_open()){
        while(getline(file, line)){
            cout << line << endl;
        }
        file.close();
    }else {
        cout << "Error to open" << endl;
        return 1;
    }
    return 0;
}


