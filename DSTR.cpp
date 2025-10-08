#include <iostream>
#include "common.h"

// declare run functions
void runLinkedVersion();
void runLinkedVersion2();
void runArrayVersion();
void runArrayVersion2();

int main() {
    while (true) {
        cout << "\n=================================\n";
        cout << "       DSTR - MAIN MENU\n";
        cout << "=================================\n";
        cout << "1. Linked List (Job > Resume)\n";
        cout << "2. Linked List (Resume > Job)\n";
        cout << "3. Array List (Job > Resume)\n";
        cout << "4. Array List (Resume > Job)\n";
        cout << "0. Exit\n";
        cout << "Select option: ";

        int choice; if (!(cin >> choice)) break; cin.ignore();
        switch (choice) {
            case 1: runLinkedVersion(); break;
            case 2: runLinkedVersion2(); break;
            case 3: runArrayVersion(); break;
            case 4: runArrayVersion2(); break;
            case 0: cout << "Exiting.\n"; return 0;
            default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
    return 0;
}
