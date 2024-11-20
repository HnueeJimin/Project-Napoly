// main.cpp
#include "jobs.h"
#include "function.h"
using namespace std;

int main() {

    int select; // 번호 선택

    while(1) {

        cout << "1. 게임시작\n";
        cout << "2. 플레이어 설정\n";
        cout << "3. 게임 규칙\n";
        cout << "4. 게임 종료\n";
        cout << "원하는 번호를 입력해주세요: ";
        
        if (!(cin >> select)) {
            clearInputBuffer();
            cout << "잘못된 입력입니다. 올바른 숫자를 입력해주세요.\n\n";
            continue;
        }

        switch(select) {
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