// main.cpp
#include "jobs.h"
#include "function.h"
using namespace std;

int main() {

    int select; // ��ȣ ����

    while (1) {

        cout << "1. ���ӽ���\n";
        cout << "2. �÷��̾� ����\n";
        cout << "3. ���� ��Ģ\n";
        cout << "4. ���� ����\n";
        cout << "\n���ϴ� ��ȣ�� �Է����ּ���: ";

        if (!(cin >> select)) {
            clearInputBuffer();
            cout << "�߸��� �Է��Դϴ�. �ùٸ� ���ڸ� �Է����ּ���.\n\n";
            continue;
        }

        switch (select) {
        case 1:
            startGame();
            break;
        case 2:
            playerModify();
            break;
        case 3:
            gameRule();
            break;
        case 4:
            cout << "������ �����մϴ�.\n";
            return 0;
        default:
            cout << "�߸��� �Է��Դϴ�. 1-4 ������ ���ڸ� �Է����ּ���.\n\n";
            break;
        }
    }
}