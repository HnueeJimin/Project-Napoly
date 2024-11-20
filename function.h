// function.h
#ifndef FUNCTION_H
#define FUNCTION_H

#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include <functional>
#include <memory>
#include <fstream>
#include <algorithm>
#include "jobs.h"
using namespace std;

// 전방 선언
void yourwork(string playerName);

void clearInputBuffer() { // 입력 버퍼를 비우는 함수
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

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
    
    int player_cnt = playlist.size();
    while (true) {
        showPlayerList();
        cout << "\n1. 플레이어 추가\n";
        cout << "2. 플레이어 삭제\n";
        cout << "3. 돌아가기\n";
        cout << "선택: ";
        
        int choice;
        cin >> choice;

        if (cin.fail()) {
            clearInputBuffer();
            cout << "잘못된 입력입니다. 숫자를 입력해주세요\n";
            continue;
        }
        
        switch(choice) {
            case 1: {
                unsigned int num;
                sel:
                cout << "몇 명의 플레이어를 추가하시겠습니까? (최대 인원 : 14) : ";
                cin >> num;

                if (cin.fail() || num <= 0 ) {
                    clearInputBuffer();
                    cout << "잘못된 입력입니다. 범위 내에서 선택해주세요\n";
                    goto sel;
                }
                if (num + player_cnt > 14) { // 플레이어 숫자 검사
                    cout << "최대 등록할 수 있는 플레이어의 수를 넘었습니다.\n";
                    break;
                }

                clearInputBuffer(); // 숫자 입력후 개행 문자 제거
                for (unsigned int i = 0; i < num; ++i ) {
                string name;
                cout << "추가할 플레이어 이름을 입력하세요: ";
                getline(cin, name);
                if (name.empty()) {
                    cout << "이름이 비어있습니다. 다시 입력해주세요 \n";
                    --i; // 반복 횟수 보정
                    continue;
                }
                playlist.push_back(name);
                cout << name << " 플레이어가 등록 되었습니다.\n";
                ++player_cnt;
                }

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
                    --player_cnt;
                } else {
                    cout << "잘못된 번호입니다.\n";
                }
                break;
            }
            case 3:
                cout << endl;
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
        case 4: return make_shared<Thief>(name);
        case 5: return make_shared<Scientist>(name);
        case 6: return make_shared<Police>(name);
        case 7: return make_shared<Doctor>(name);
        case 8: return make_shared<Soldier>(name);
        case 9: return make_shared<Thug>(name);
        case 10: return make_shared<Politician>(name);
        case 11: return make_shared<Reporter>(name);
        case 12: return make_shared<Terrorist>(name);
        case 13: return make_shared<Nurse>(name);
        case 14: return make_shared<Mercenary>(name);
        case 15: return make_shared<Caveman>(name);
        case 16: return make_shared<Cleric>(name);
        default: return make_shared<Mafia>(name);
    }
}

void assignRoles() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 16); // 직업 번호 생성

    players.clear();
    vector<int> availableIndex(playlist.size());
    iota(availableIndex.begin(), availableIndex.end(), 0);

    int mafiaCnt = 0; // 마피아의 수, 2
    int requiredRoles = 0; // 필수 직업의 수, 4

    for (int i = 0; i < 2; i++) { // 마피아 2명 할당
        uniform_int_distribution<> dis(0, availableIndex.size() - 1);
        int idx = dis(gen);
        int playerIdx = availableIndex[idx];
        players.push_back(make_shared<Mafia>(playlist[playerIdx]));
        availableIndex.erase(availableIndex.begin() + idx);
        mafiaCnt++;
        requiredRoles++;
    }

    {
        uniform_int_distribution<> dis(0, availableIndex.size() - 1);
        int idx = dis(gen);
        int playerIdx = availableIndex[idx];
        players.push_back(make_shared<Police>(playlist[playerIdx]));
        availableIndex.erase(availableIndex.begin() + idx);
        requiredRoles++;
    }

    {
        uniform_int_distribution<> dis(0, availableIndex.size() - 1);
        int idx = dis(gen);
        int playerIdx = availableIndex[idx];
        players.push_back(make_shared<Doctor>(playlist[playerIdx]));
        availableIndex.erase(availableIndex.begin() + idx);
        requiredRoles++;
    }

    int remainingPlayers = availableIndex.size();
    int minCitizenCnt = mafiaCnt + 3;
}

void gamerule() {
    cout << "\n=== 마피아 게임 규칙 ===\n";

    ifstream ruleFile("mafiarule.txt");

    if(!ruleFile.is_open()) { // 룰 파일 열기 실패 시, 적절한 대체 방안 제공
    cout << "1. 게임은 최소 8명의 플레이어가 필요합니다.\n";
    cout << "2. 각 플레이어는 게임 시작 시 랜덤으로 직업을 부여받습니다.\n";
    cout << "3. 게임은 낮과 밤으로 진행됩니다.\n";
    cout << "4. 낮에는 토론과 투표를 통해 용의자를 처형합니다.\n";
    cout << "5. 밤에는 각자의 직업에 따른 특수 능력을 사용할 수 있습니다.\n";
    cout << "6. 마피아를 모두 제거하거나, 마피아가 선량한 시민 수와 같아지면 게임이 종료됩니다.\n\n";
    }

    string line;
    while(getline(ruleFile, line)) {
        cout << line << "\n";
    }

    ruleFile.close();
    cout << "\n";
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
        cout << "\n" << player->getName() << "님이 맞으시다면 (Y/y)를 입력해주세요: ";
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

void startVoting() {
    cout << "\n=== 투표를 시작합니다 ===\n";
    map<shared_ptr<Player>, int> votes;
    
    // 살아있는 플레이어 목록 표시
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i]->checkAlive()) {
            cout << i + 1 << ". " << players[i]->getName() << "\n";
        }
    }
    // 투표 진행
    for (const auto& voter : players) {
        if (voter->checkAlive() && voter->getCanVote()) {
            int choice;
            cout << voter->getName() << "의 투표: ";
            cin >> choice;
            choice--;
            
            if (choice >= 0 && choice < static_cast<int>(players.size())) {
                int voteWeight = 1;
                if (voter->getRole() == "정치인") {
                    voteWeight = 2;
                }
                votes[players[choice]] += voteWeight;
            }
        }
    }
    
    // 투표 결과 처리
    auto maxVotes = max_element(votes.begin(), votes.end(),
        [](const auto& p1, const auto& p2) { return p1.second < p2.second; });
    
    if (maxVotes != votes.end()) {
        cout << "\n" << maxVotes->first->getName() << "이(가) 처형되었습니다.\n";
        maxVotes->first->setAlive(false);
    }
}

struct NightAction {
    shared_ptr<Player> actor; // 고유 행동
    shared_ptr<Player> target; // 대상
    string actionType;
};

vector<NightAction> nightActions;
int currentDay = 1;
bool isNight = false;

void processNightActions() {
    // 직업별 우선순위 설정
    map<string, int> priority = {
        {"마피아", 1},
        {"늑대인간", 2},
        {"의사", 3},
        {"간호사", 4},
        {"경찰", 5},
        {"스파이", 6},
        {"마담", 7},
        {"도둑", 8},
        {"건달", 9},
        {"기자", 10},
        {"테러리스트", 11},
        {"성직자", 12}
    };
    
    // 우선순위에 따라 행동 정렬
    sort(nightActions.begin(), nightActions.end(),
        [&priority](const NightAction& a, const NightAction& b) {
            return priority[a.actor->getRole()] < priority[b.actor->getRole()];
        });
    
    // 정렬된 순서대로 행동 처리
    for (const auto& action : nightActions) {
        // 행동 주체가 살아있고 능력을 사용할 수 있는 상태인지 확인
        if (action.actor->checkAlive() && action.actor->getCanUseAbility()) {
            action.actor->action(*action.target);
        }
    }
    
    // 특수 상황 처리
    // 테러리스트 보복 처리
    for (const auto& player : players) {
        if (auto terrorist = dynamic_cast<Terrorist*>(player.get())) {
            terrorist->checkRetaliation();
        }
    }
    
    // 의사 사망시 간호사 능력 승계 처리
    bool doctorDied = false;
    for (const auto& player : players) {
        if (player->getRole() == "의사" && !player->checkAlive()) {
            doctorDied = true;
            break;
        }
    }
    
    if (doctorDied) {
        for (const auto& player : players) {
            if (auto nurse = dynamic_cast<Nurse*>(player.get())) {
                nurse->onDoctorDeath();
            }
        }
    }
}

void startNight() {
    isNight = true;
    cout << "\n=== " << currentDay << "번째 밤이 되었습니다 ===\n";
    nightActions.clear();
    
    // 각 플레이어의 밤 행동 수집
    for (const auto& player : players) {
        if (player->checkAlive() && player->getCanUseAbility()) {
            cout << "\n" << player->getName() << "님의 차례입니다.\n";
            cout << "당신의 직업은 " << player->getRole() << "입니다.\n";
            
            // 능력을 사용할 수 있는 대상 목록 표시
            cout << "\n능력을 사용할 수 있는 대상:\n";
            for (size_t i = 0; i < players.size(); i++) {
                if (players[i]->getName() != player->getName() && players[i]->checkAlive()) {
                    cout << i + 1 << ". " << players[i]->getName() << "\n";
                }
            }
            
            int choice;
            while (true) {
                cout << "대상을 선택하세요 (0: 능력 사용하지 않음): ";
                cin >> choice;
                
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "잘못된 입력입니다.\n";
                    continue;
                }
                
                if (choice == 0) break;
                
                if (choice > 0 && choice <= static_cast<int>(players.size())) {
                    choice--; // 0-based index로 변환
                    if (players[choice]->getName() != player->getName() && players[choice]->checkAlive()) {
                        // 나중에 처리하기 위해 행동 저장
                        nightActions.push_back({
                            player,
                            players[choice],
                            player->getRole()
                        });
                        break;
                    }
                }
                cout << "잘못된 선택입니다.\n";
            }  
            system("cls"); // Windows의 화면 지우기 기능
        }
    }
}

void startDay() {
    isNight = false;
    cout << "\n=== " << currentDay << "번째 날이 밝았습니다 ===\n";
    
    // 전날 밤의 행동 결과 처리
    processNightActions();
    
    // 사망자 발표
    for (const auto& player : players) {
        if (!player->checkAlive()) {
            cout << player->getName() << "이(가) 사망했습니다.\n";
        }
    }
    
    // 투표 진행
    startVoting();
    currentDay++;
}

#endif // FUNCTION_H