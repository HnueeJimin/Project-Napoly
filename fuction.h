// function.h

#include <iostream>
#include <string>
#include <random>
#include "header.h"

class GameInterface {
private:
    GameManager gameManager;
    bool gameRunning;

public:
    GameInterface() : gameRunning(false) {}

    void showMainMenu() {
        while (true) {
            cout << "\n=== 마피아 게임 ===\n";
            cout << "1. 게임 시작\n";
            cout << "2. 플레이어 등록\n";
            cout << "3. 게임 규칙\n";
            cout << "4. 종료\n";
            cout << "선택: ";

            int choice;
            cin >> choice;

            switch (choice) {
                case 1: startGame(); break;
                case 2: registerPlayers(); break;
                case 3: showRules(); break;
                case 4: return;
                default: cout << "잘못된 선택입니다.\n";
            }
        }
    }

    void registerPlayers() {
        cout << "등록할 플레이어 수를 입력하세요: ";
        int playerCount;
        cin >> cin.ignore();
        cin >> playerCount;

        for (int i = 0; i < playerCount; i++) {
            cout << "플레이어 " << (i + 1) << " 이름: ";
            string name;
            cin.ignore();
            getline(cin, name);
            
            // 여기서 직업 랜덤 배정 로직 구현
            // 추후 구현
        }
    }

    void startGame() {
        // 게임 시작 로직
        gameRunning = true;
        while (gameRunning) {
            // 게임 진행 로직
            // 추후 구현
        }
    }

    void showRules() {
        cout << "\n=== 게임 규칙 ===\n";
        cout << "1. 게임은 낮과 밤으로 진행됩니다.\n";
        cout << "2. 마피아는 밤에 한 명을 제거할 수 있습니다.\n";
        cout << "3. 시민들은 낮에 투표로 마피아로 의심되는 사람을 처형할 수 있습니다.\n";
        cout << "4. 마피아가 모두 제거되면 시민 팀 승리, 시민 수가 마피아 이하가 되면 마피아 팀 승리\n\n";
        
        cout << "=== 직업별 능력 ===\n";
        // 각 직업별 능력 설명
        // 추후 구현
    }
};