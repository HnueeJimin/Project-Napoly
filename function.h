// function.h
#include <iostream>
#include <string>
#include "header.h"
using namespace std;

void start() { // 시작 함수 구현
    char input;
    cout << "게임이 시작되었습니다\n";
    yinput:
    cout << "플레이어님이 맞으시다면 y를 입력해주세요: ";
    cin >> input;
    if (!(input == 'y' || input == 'Y')) {
        "잘못된 입력입니다.\n";
        goto yinput;
    }
    yourwork(string a);
}

void player_add(int num) { // 플레이어 추가
    vector<string> playlist; // 플레이러 리스트 지정
    for (int i = 0; i < num; i++) {    
        string name; // 이름 입력
        cout << i+1 <<" 번 째 플레이어의 이름을 입력해주세요";
        cin >> name;

        playlist.push_back(name); // 플레이어 리스트에 추가
    }

}

void gamerule() { // 게임 규칙

}

void yourwork(string player) { // 각 플레이어의 직업 호출

}