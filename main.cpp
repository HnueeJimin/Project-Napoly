// main.cpp
#include "jobs.h"
#include "function.h"
using namespace std;

int main() {

    int select; // 번호 선택

    while (1) {
        cout << "<<Project : Napoly>>\n";
        cout << "\n1. 게임시작\n";
        cout << "\n2. 플레이어 추가 및 설정\n";
        cout << "\n3. 게임 규칙\n";
        cout << "\n4. 게임 종료\n";
        cout << "\n\n원하는 번호를 선택해주세요: ";

        if (!(cin >> select)) {
            clearInputBuffer();
            cout << "잘못된 입력입니다. 올바른 숫자를 입력해주세요.\n\n";
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
            cout << "게임을 종료합니다.\n";
            return 0;
        default:
            cout << "잘못된 입력입니다. 1-4 사이의 숫자를 입력해주세요.\n\n";
            break;
        }
    }
}