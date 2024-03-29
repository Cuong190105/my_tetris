#include <iostream>
#include <cstdlib>
#include <vector>
#include "logic.hpp"
using namespace std;

int main(int argv, char **args) {
    //Initialize SDL & Create window
    if ( !init() ) {
        cout << "Failed to initialize" << endl;
    }
    else
    {
        srand(time(NULL));
        loadMedia();
        taskManager();
    }
    close();
    return 0;
}