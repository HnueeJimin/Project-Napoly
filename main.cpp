#include "header.h"
#include <random>
#include <algorithm>
#include <map>
#include <chrono>
#include <thread>

// 직업 분배를 위한 함수
void assignRoles(vector<shared_ptr<Player>>& players) {
    vector<string> roles;
    
    // 기본 직업 구성 (8~10명 기준)
    roles = {
        "Mafia", "Police", "Doctor", // 기본 구성
        "Spy", "Soldier", "Reporter", // 추가 직업
        "Politician", "Thug" // 8명 기본 구성
    };
    
    // 9~10명일 경우 추가 직업
    if (players.size() >= 9) roles.push_back("Nurse");
    if (players.size() == 10) roles.push_back("Mercenary");
    
    // 직업 무작위 배정
    random_device rd;
    mt19937 gen(rd());
    shuffle(roles.begin(), roles.end(), gen);
    
    // 각 플레이어에게 직업 할당
    for (size_t i = 0; i < players.size(); i++) {
        string name = players[i]->getName();
        if (roles[i] == "Mafia") players[i] = make_shared<Mafia>(name);
        else if (roles[i] == "Police") players[i] = make_shared<Police>(name);
        else if (roles[i] == "Doctor") players[i] = make_shared<Doctor>(name);
        else if (roles[i] == "Spy") players[i] = make_shared<Spy>(name);
        else if (roles[i] == "Soldier") players[i] = make_shared<Soldier>(name);
        else if (roles[i] == "Reporter") players[i] = make_shared<Reporter>(name);
        else if (roles[i] == "Politician") players[i] = make_shared<Politician>(name);
        else if (roles[i] == "Thug") players[i] = make_shared<Thug>(name);
        else if (roles[i] == "Nurse") players[i] = make_shared<Nurse>(name);
        else if (roles[i] == "Mercenary") players[i] = make_shared<Mercenary>(name);
    }
}

// 게임 진행을 위한 클래스
class GameManager {
private:
    vector<shared_ptr<Player>>& players;
    int day;
    bool isNight;
    
    // 마피아 수 확인
    int getMafiaCount() {
        int count = 0;
        for (const auto& player : players) {
            if (player->checkAlive() && player->getRole() == "Mafia") count++;
        }
        return count;
    }
    
    // 시민 수 확인
    int getCitizenCount() {
        int count = 0;
        for (const auto& player : players) {
            if (player->checkAlive() && player->getRole() != "Mafia") count++;
        }
        return count;
    }
    
    // 투표 진행
    void handleVoting() {
        map<shared_ptr<Player>, int> votes;
        vector<shared_ptr<Player>> alivePlayers;
        
        // 생존자 목록 작성
        for (const auto& player : players) {
            if (player->checkAlive()) {
                alivePlayers.push_back(player);
                votes[player] = 0;
            }
        }
        
        // 투표 진행
        cout << "\n투표를 시작합니다.\n";
        for (const auto& voter : players) {
            if (!voter->checkAlive() || !voter->getCanVote()) continue;
            
            cout << "\n" << voter->getName() << "의 투표\n";
            for (size_t i = 0; i < alivePlayers.size(); i++) {
                cout << i + 1 << ". " << alivePlayers[i]->getName() << endl;
            }
            
            int choice;
            cout << "선택 (1-" << alivePlayers.size() << "): ";
            cin >> choice;
            
            if (choice > 0 && choice <= static_cast<int>(alivePlayers.size())) {
                int weight = (voter->getRole() == "Politician") ? 2 : 1;
                votes[alivePlayers[choice - 1]] += weight;
            }
        }
        
        // 투표 결과 처리
        auto maxVotes = max_element(votes.begin(), votes.end(),
            [](const auto& p1, const auto& p2) { return p1.second < p2.second; });
            
        if (maxVotes->first->getRole() != "Politician") {  // 정치인은 투표로 처형되지 않음
            cout << "\n" << maxVotes->first->getName() << "이(가) 처형되었습니다.\n";
            maxVotes->first->setAlive(false);
        }
    }
    
    // 밤 행동 처리
    void handleNightActions() {
        cout << "\n밤이 되었습니다. 각자의 능력을 사용할 시간입니다.\n";
        
        vector<shared_ptr<Player>> alivePlayers;
        for (const auto& player : players) {
            if (player->checkAlive()) alivePlayers.push_back(player);
        }
        
        // 각 플레이어의 능력 사용
        for (const auto& player : players) {
            if (!player->checkAlive() || !player->getCanUseAbility()) continue;
            
            cout << "\n" << player->getName() << "(" << player->getRole() << ")의 차례\n";
            cout << "대상을 선택하세요:\n";
            
            for (size_t i = 0; i < alivePlayers.size(); i++) {
                cout << i + 1 << ". " << alivePlayers[i]->getName() << endl;
            }
            
            int choice;
            cout << "선택 (1-" << alivePlayers.size() << "): ";
            cin >> choice;
            
            if (choice > 0 && choice <= static_cast<int>(alivePlayers.size())) {
                player->action(*alivePlayers[choice - 1]);
            }
        }
    }
    
    // 승리 조건 확인
    bool checkGameEnd() {
        int mafiaCount = getMafiaCount();
        int citizenCount = getCitizenCount();
        
        if (mafiaCount == 0) {
            cout << "\n모든 마피아가 제거되었습니다. 시민 팀의 승리!\n";
            return true;
        }
        
        if (mafiaCount >= citizenCount) {
            cout << "\n시민의 수가 마피아 이하가 되었습니다. 마피아 팀의 승리!\n";
            return true;
        }
        
        return false;
    }
    
public:
    GameManager(vector<shared_ptr<Player>>& p) : players(p), day(1), isNight(false) {}
    
    void runGame() {
        assignRoles(players);
        
        // 각 플레이어에게 역할 알림
        cout << "\n각 플레이어의 역할이 분배되었습니다.\n";
        for (const auto& player : players) {
            cout << player->getName() << "님의 역할: " << player->getRole() << endl;
            this_thread::sleep_for(chrono::seconds(2));
            cout << string(50, '\n');  // 화면 지우기
        }
        
        // 게임 메인 루프
        while (true) {
            if (!isNight) {
                cout << "\n=== " << day << "일차 낮 ===\n";
                handleVoting();
                
                if (checkGameEnd()) break;
                
                isNight = true;
            } else {
                cout << "\n=== " << day << "일차 밤 ===\n";
                handleNightActions();
                
                if (checkGameEnd()) break;
                
                isNight = false;
                day++;
            }
        }
        
        // 게임 종료 시 최종 결과 출력
        cout << "\n=== 게임 종료 ===\n";
        cout << "생존자 목록:\n";
        for (const auto& player : players) {
            if (player->checkAlive()) {
                cout << player->getName() << " (" << player->getRole() << ")\n";
            }
        }
    }
};

int main() {
    vector<shared_ptr<Player>> players;
    bool isGameRunning = true;
    int playerCount = 0;
    
    cout << "========== 마피아 게임 ==========\n\n";
    
    while (isGameRunning) {
        int choice;
        cout << "1. 플레이어 추가\n";
        cout << "2. 게임 시작\n";
        cout << "3. 게임 규칙 설명\n";
        cout << "4. 종료\n";
        cout << "선택: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: {
                if (playerCount >= 10) {
                    cout << "더 이상 플레이어를 추가할 수 없습니다. (최대 10명)\n\n";
                    break;
                }
                
                string playerName;
                cout << "플레이어 이름을 입력하세요: ";
                getline(cin, playerName);
                
                if (playerName.empty()) {
                    cout << "이름은 비워둘 수 없습니다.\n\n";
                    break;
                }
                
                players.push_back(make_shared<Civilian>(playerName));
                playerCount++;
                
                cout << "현재 플레이어 수: " << playerCount << "/10\n\n";
                break;
            }
            
            case 2: {
                if (playerCount < 8) {
                    cout << "플레이어가 부족합니다. 최소 8명이 필요합니다. (현재: " 
                         << playerCount << "명)\n\n";
                    break;
                }
                
                cout << "\n게임을 시작합니다!\n";
                cout << "총 " << playerCount << "명의 플레이어가 참가합니다.\n\n";
                
                GameManager game(players);
                game.runGame();
                
                isGameRunning = false;
                break;
            }
            
            case 3: {
                cout << "\n========== 게임 규칙 ==========\n";
                cout << "1. 게임은 낮과 밤으로 진행됩니다.\n";
                cout << "2. 낮에는 토론을 통해 마피아로 의심되는 사람을 투표로 처형합니다.\n";
                cout << "3. 밤에는 각자 직업의 고유 능력을 사용합니다.\n";
                cout << "4. 마피아를 모두 처형하면 시민 팀 승리, 시민의 수가 마피아 이하가 되면 마피아 팀 승리입니다.\n";
                cout << "==============================\n\n";
                break;
            }
            
            case 4: {
                cout << "게임을 종료합니다.\n";
                return 0;
            }
            
            default: {
                cout << "잘못된 선택입니다. 다시 선택해주세요.\n\n";
                break;
            }
        }
    }
    
    return 0;
}