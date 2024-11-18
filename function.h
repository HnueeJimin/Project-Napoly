// function.h
#ifndef FUNCTION_H
#define FUNCTION_H

#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include <functional>
#include <memory>
#include "jobs.h"
using namespace std;

// 전방 선언
void yourwork(string playerName);

// 전역 변수로 플레이어 리스트와 플레이어 객체 리스트 선언
vector<string> playlist;
vector<shared_ptr<Player>> players;

// 플레이어 관리 함수들
void showPlayerList() {
    cout << "\n=== 현재 등록된 플레이어 목록 ===\n";
    for (size_t i = 0; i < playlist.size(); i++) {
        cout << i + 1 << ". " << playlist[i] << "\n";
    }
    cout << "총 " << playlist.size() << "명\n";
}

void playerModify() {
    cout << "\n=== 플레이어 관리 메뉴 ===\n";
    while (true) {
        showPlayerList();
        cout << "\n1. 플레이어 추가\n";
        cout << "2. 플레이어 삭제\n";
        cout << "3. 돌아가기\n";
        cout << "선택: ";
        
        int choice;
        cin >> choice;
        
        switch(choice) {
            case 1: {
                string name;
                cout << "추가할 플레이어 이름을 입력하세요: ";
                cin >> name;
                playlist.push_back(name);
                cout << name << " 플레이어가 추가되었습니다.\n";
                break;
            }
            case 2: {
                if (playlist.empty()) {
                    cout << "삭제할 플레이어가 없습니다.\n";
                    break;
                }
                showPlayerList();
                cout << "삭제할 플레이어 번호를 입력하세요: ";
                int index;
                cin >> index;
                if (index > 0 && index <= static_cast<int>(playlist.size())) {
                    string removedName = playlist[index - 1];
                    playlist.erase(playlist.begin() + index - 1);
                    cout << removedName << " 플레이어가 삭제되었습니다.\n";
                } else {
                    cout << "잘못된 번호입니다.\n";
                }
                break;
            }
            case 3:
                return;
            default:
                cout << "잘못된 입력입니다.\n";
        }
    }
}

shared_ptr<Player> createRole(const string& name, int roleType) {
    switch(roleType) {
        case 0: return make_shared<Mafia>(name);
        case 1: return make_shared<Spy>(name);
        case 2: return make_shared<Werewolf>(name);
        case 3: return make_shared<Madame>(name);
        case 4: return make_shared<Scientist>(name);
        case 5: return make_shared<Police>(name);
        case 6: return make_shared<Doctor>(name);
        case 7: return make_shared<Soldier>(name);
        case 8: return make_shared<Thug>(name);
        case 9: return make_shared<Politician>(name);
        case 10: return make_shared<Reporter>(name);
        case 11: return make_shared<Terrorist>(name);
        case 12: return make_shared<Nurse>(name);
        case 13: return make_shared<Mercenary>(name);
        default: return make_shared<Mafia>(name);
    }
}

void assignRoles() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 13);

    players.clear();
    for (const auto& name : playlist) {
        int roleType = dis(gen);
        players.push_back(createRole(name, roleType));
    }
}

void gamerule() {
    cout << "\n=== 마피아 게임 규칙 ===\n";
    cout << "1. 게임은 최소 8명의 플레이어가 필요합니다.\n";
    cout << "2. 각 플레이어는 게임 시작 시 랜덤으로 직업을 부여받습니다.\n";
    cout << "3. 게임은 낮과 밤으로 진행됩니다.\n";
    cout << "4. 낮에는 토론과 투표를 통해 용의자를 처형합니다.\n";
    cout << "5. 밤에는 각자의 직업에 따른 특수 능력을 사용할 수 있습니다.\n";
    cout << "6. 마피아를 모두 제거하거나, 마피아가 선량한 시민 수와 같아지면 게임이 종료됩니다.\n\n";
}

void start() {
    // 플레이어 수 확인
    if (playlist.size() < 8) {
        cout << "게임을 시작하기 위해서는 최소 8명의 플레이어가 필요합니다.\n";
        cout << "현재 플레이어 수: " << playlist.size() << "명\n";
        return;
    }

    cout << "게임이 시작되었습니다\n";
    
    // 직업 할당
    assignRoles();

    // 각 플레이어 순서대로 진행
    for (const auto& player : players) {
        char input;
        yinput:
        cout << "\n" << player->getName() << "님이 맞으시다면 y를 입력해주세요: ";
        cin >> input;
        if (!(input == 'y' || input == 'Y')) {
            cout << "잘못된 입력입니다.\n";
            goto yinput;
        }

        // 직업 공개 및 능력 사용
        cout << player->getName() << "님의 직업은 " << player->getRole() << "입니다.\n";
        if (player->checkAlive()) {
            yourwork(player->getName());
        }

        // 화면 지우기
        system("cls"); // Windows
        // system("clear"); // Linux/Mac
    }
}

void yourwork(string playerName) {
    // 현재 플레이어의 객체 찾기
    shared_ptr<Player> currentPlayer;
    for (const auto& player : players) {
        if (player->getName() == playerName) {
            currentPlayer = player;
            break;
        }
    }

    if (!currentPlayer || !currentPlayer->getCanUseAbility()) {
        cout << playerName << "님은 현재 능력을 사용할 수 없습니다.\n";
        return;
    }

    // 능력을 사용할 대상 선택
    cout << "\n능력을 사용할 대상을 선택하세요:\n";
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i]->getName() != playerName && players[i]->checkAlive()) {
            cout << i + 1 << ". " << players[i]->getName() << "\n";
        }
    }

    int choice;
    cout << "선택 (번호 입력): ";
    cin >> choice;
    choice--; // 0-based index로 변환

    if (choice >= 0 && choice < static_cast<int>(players.size())) {
        currentPlayer->action(*players[choice]);
    } else {
        cout << "잘못된 선택입니다.\n";
    }

    cout << "\n계속하려면 아무 키나 누르세요...";
    cin.ignore();
    cin.get();
}

#endif // FUNCTION_H