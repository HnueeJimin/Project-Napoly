// main.cpp
#include "header.h"
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
        cin >> select;

        switch(select) {
            case 1:
                start();
                break;
            case 2:
                int num;
                cout << "몇 명의 플레이어를 등록 하시겠습니까?";
                cin >> num;
                player_add(num);
                break;
            case 3:
                gamerule();
            case 4:
                return 0;
            default:
                cout << "잘못된 입력입니다. 다시 입력해주세요\n";
        }
    }
}