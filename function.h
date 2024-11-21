// function.h
#ifndef FUNCTION_H
#define FUNCTION_H

#include <ctime>
#include <functional>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include "jobs.h"

using namespace std;
using namespace std::chrono;

// 전방 선언
class NightPhaseManager;
void startNight();
void startDay();
void startVoting();
bool checkVictoryCondition();

// 구조체 정의
struct NightResult
{ // 밤의 행동 결과를 저장하는 구조체
    string playerName;
    string targetName;
    string message;
    bool isPrivate; // true면 해당 플레이어만 볼 수 있음
};

struct GameState
{ // 게임 상태 표시
    vector<shared_ptr<Player>> alivePlayers;
    bool isMafiaWin() const
    {
        int mafiaCount = 0;
        int citizenCount = 0;
        for (const auto &player : alivePlayers)
        {
            if (player->checkAlive())
            {
                if (player->getRole() == "마피아")
                    mafiaCount++;
                else
                    citizenCount++;
            }
        }
        return mafiaCount >= citizenCount;
    }
};

struct NightAction
{ // 밤 행동 관리
    shared_ptr<Player> actor;
    shared_ptr<Player> target;
    string actionType;
    int priority; // 우선순위 추가
};

// 클래스 정의
class NightPhaseManager
{
private:
    vector<NightAction> actions;
    map<string, int> actionPriorities;

public:
    NightPhaseManager()
    {
        actionPriorities = {
            {"늑대인간", 1},
            {"의사", 2},
            {"마피아", 3},
            {"군인", 4},
            {"시민", 5}};
    }

    void clear()
    {
        actions.clear();
    }

    void addAction(shared_ptr<Player> actor, shared_ptr<Player> target, string actionType)
    {
        NightAction action;
        action.actor = actor;
        action.target = target;
        action.actionType = actionType;
        action.priority = actionPriorities[actor->getRole()];
        actions.push_back(action);
    }
    void processActions()
    {
        // 우선순위에 따라 정렬
        sort(actions.begin(), actions.end(),
             [this](const NightAction &a, const NightAction &b)
             {
                 return actionPriorities[a.actor->getRole()] <
                        actionPriorities[b.actor->getRole()];
             });

        map<shared_ptr<Player>, bool> healedPlayers; // 치료된 플레이어 추적

        // 각 액션 처리
        for (const auto &action : actions)
        {
            if (!action.actor->checkAlive() || !action.actor->getCanUseAbility())
                continue;

            if (action.actor->getRole() == "마피아")
            {
                // 마피아의 공격
                if (auto soldier = dynamic_cast<Soldier *>(action.target.get()))
                {
                    if (soldier->defendShot())
                        continue;
                }
                action.target->setAlive(false); // 일단 사망 처리
            }
            else if (action.actor->getRole() == "의사" ||
                     action.actor->getRole() == "간호사")
            {
                // 치료 처리
                if (!action.target->checkAlive())
                {
                    action.target->setAlive(true);
                    healedPlayers[action.target] = true;
                }
            }
            // 다른 직업들의 능력은 즉시 처리
            else
            {
                action.actor->action(*action.target);
            }
        }

        // 결과 출력
        for (const auto &action : actions)
        {
            if (action.actor->getRole() == "마피아" && !action.target->checkAlive())
            {
                if (healedPlayers[action.target])
                {
                    cout << "의사가 " << action.target->getName()
                         << "님을 치료했습니다.\n";
                }
                else
                {
                    cout << action.target->getName()
                         << "님이 마피아에게 처치당했습니다.\n";
                }
            }
        }
    }
};

// 전역 변수 선언
vector<string> playlist;
vector<shared_ptr<Player>> players;
vector<NightResult> nightResults;
static NightPhaseManager nightManager;
int currentDay = 1;
bool isNight = false;

// 유틸리티 함수
void clearInputBuffer()
{ // 입력 버퍼를 비우는 함수
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void showPlayerList()
{ // 플레이어 관리 함수들
    cout << "\n=== 현재 등록된 플레이어 목록 ===\n";
    for (size_t i = 0; i < playlist.size(); i++)
    {
        cout << i + 1 << ". " << playlist[i] << "\n";
    }
    cout << "총 " << playlist.size() << "명\n";
}

void showResults(const string &playerName)
{
    bool foundResult = false;

    // 1. 자신이 수행한 행동 결과
    for (const auto &result : nightResults)
    {
        if (result.playerName == playerName && result.isPrivate)
        {
            cout << "[행동 결과] " << result.message << "\n";
            foundResult = true;
        }
    }

    // 2. 자신이 대상이 된 행동 결과
    for (const auto &result : nightResults)
    {
        if (result.targetName == playerName)
        {
            cout << "[받은 영향] " << result.message << "\n";
            foundResult = true;
        }
    }

    // 3. 공개된 정보
    for (const auto &result : nightResults)
    {
        if (!result.isPrivate)
        {
            cout << "[공개 정보] " << result.message << "\n";
            foundResult = true;
        }
    }

    if (!foundResult)
    {
        cout << "당신에게 아무런 일도 일어나지 않았습니다...\n";
    }
}

void yourTurn(shared_ptr<Player> currentPlayer)
{
    if (!currentPlayer->checkAlive() || !currentPlayer->getCanUseAbility())
    {
        cout << "현재 능력을 사용할 수 없습니다.\n";
        return;
    }

    if (currentPlayer->getRole() == "군인" || currentPlayer->getRole() == "시민")
    {
        cout << "당신은 밤에 수행할 수 있는 역할이 없습니다.\n";
        return;
    }

    cout << "\n능력을 사용할 대상을 선택하세요:\n";
    vector<shared_ptr<Player>> validTargets;
    for (const auto &player : players)
    {
        if (player->getName() != currentPlayer->getName() && player->checkAlive())
        {
            validTargets.push_back(player);
            cout << validTargets.size() << ". " << player->getName() << "\n";
        }
    }

    int choice;
    while (true)
    {
        cout << "대상을 선택하세요 (0: 능력 사용하지 않음): ";
        if (!(cin >> choice))
        {
            clearInputBuffer();
            cout << "잘못된 입력입니다.\n";
            continue;
        }

        if (choice == 0)
            break;

        if (choice > 0 && choice <= static_cast<int>(validTargets.size()))
        {
            shared_ptr<Player> target = validTargets[choice - 1];

            // 능력 사용 결과를 저장
            if (currentPlayer->getRole() == "경찰")
            {
                string result = target->getRole() == "마피아" ? "마피아입니다." : "마피아가 아닙니다.";
                nightResults.push_back({currentPlayer->getName(),
                                        target->getName(),
                                        target->getName() + "(은)는 " + result,
                                        true});
            }
            else if (currentPlayer->getRole() == "마피아")
            {
                nightResults.push_back({currentPlayer->getName(),
                                        target->getName(),
                                        target->getName() + "을(를) 공격했습니다.",
                                        true});
            }
            nightManager.addAction(currentPlayer, target, currentPlayer->getRole());
            break;
        }
        cout << "잘못된 선택입니다.\n";
    }
};

// 게임 로직 함수
void playerModify()
{ // 플레이어 수정 및 관리 함수
    cout << "\n=== 플레이어 관리 메뉴 ===\n";

    int player_cnt = playlist.size();
    while (true)
    {
        showPlayerList();
        cout << "\n1. 플레이어 추가\n";
        cout << "2. 플레이어 삭제\n";
        cout << "3. 돌아가기\n";
        cout << "선택: ";

        int choice;
        cin >> choice;

        if (cin.fail())
        {
            clearInputBuffer();
            cout << "잘못된 입력입니다. 숫자를 입력해주세요\n";
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            unsigned int num;
        sel:
            cout << "몇 명의 플레이어를 추가하시겠습니까? (최대 8명) : ";
            cin >> num;

            if (cin.fail() || num <= 0)
            {
                clearInputBuffer();
                cout << "잘못된 입력입니다. 범위 내의 숫자에서 선택해주세요\n";
                goto sel;
            }
            if (num + player_cnt > 8)
            { // 플레이어 숫자 검사
                cout << "최대 등록할 수 있는 플레이어의 수를 넘었습니다.\n";
                break;
            }

            clearInputBuffer();
            for (unsigned int i = 0; i < num; ++i)
            {
                string name;
                cout << "추가할 플레이어 이름을 입력하세요: ";
                getline(cin, name);
                if (name.empty())
                {
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
        case 2:
        {
            if (playlist.empty())
            {
                cout << "삭제할 플레이어가 없습니다.\n";
                break;
            }
            showPlayerList();
            cout << "삭제할 플레이어 번호를 입력하세요: ";
            int index;
            cin >> index;

            if (index > 0 && index <= static_cast<int>(playlist.size()))
            {
                string removedName = playlist[index - 1];
                playlist.erase(playlist.begin() + index - 1);
                cout << removedName << " 플레이어가 삭제되었습니다.\n";
                --player_cnt;
            }
            else
            {
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

shared_ptr<Player> createRole(const string &name, int roleType)
{
    switch (roleType)
    {
    case 0:
        return make_shared<Mafia>(name);
    case 1:
        return make_shared<Werewolf>(name);
    case 2:
        return make_shared<Police>(name);
    case 3:
        return make_shared<Doctor>(name);
    case 4:
        return make_shared<Soldier>(name);
    case 5:
        return make_shared<Citizen>(name);
    default:
        return make_shared<Citizen>(name);
    }
}

void assignRoles()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 5); // 직업 번호 생성

    players.clear();
    for (const auto &name : playlist)
    {
        int roleType = dis(gen);
        players.push_back(createRole(name, roleType));
    }
}

void gameRule()
{
    cout << "\n=== 마피아 게임 규칙 ===\n";

    ifstream ruleFile("mafiarule.txt");

    if (!ruleFile.is_open())
    { // 룰 파일 열기 실패 시, 적절한 대체 방안 제공
        cout << "1. 게임은 최소 6명의 플레이어가 필요합니다.\n";
        cout << "2. 각 플레이어는 게임 시작 시 랜덤으로 직업을 부여받습니다.\n";
        cout << "3. 게임은 낮과 밤으로 진행됩니다.\n";
        cout << "4. 낮에는 토론과 투표를 통해 용의자를 처형합니다.\n";
        cout << "5. 밤에는 각자의 직업에 따른 역할을 수행합니다.\n";
        cout << "6. 마피아를 모두 제거하거나, 마피아가 선량한 시민 수와 같아지면 게임이 종료됩니다.\n\n";
    }

    string line;
    while (getline(ruleFile, line))
    {
        cout << line << "\n";
    }

    ruleFile.close();
    cout << "\n";
}

// 게임 진행 함수
void startVoting()
{
    cout << "\n=== 투표를 시작합니다 ===\n";
    map<shared_ptr<Player>, int> votes;

    // 살아있는 플레이어 목록 표시
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players[i]->checkAlive())
        {
            cout << i + 1 << ". " << players[i]->getName() << "\n";
        }
    }
    // 투표 진행
    for (const auto &voter : players)
    {
        if (voter->checkAlive() && voter->getCanVote())
        {
            int choice;
            cout << voter->getName() << "의 투표: ";
            cin >> choice;
            choice--;
        }
    }

    // 투표 결과 처리
    auto maxVotes = max_element(votes.begin(), votes.end(),
                                [](const auto &p1, const auto &p2)
                                { return p1.second < p2.second; });

    if (maxVotes != votes.end())
    {
        cout << "\n"
             << maxVotes->first->getName() << "이(가) 처형되었습니다.\n";
        maxVotes->first->setAlive(false);
    }
}

void startGame()
{
    if (playlist.size() < 6)
    {
        cout << "\n게임을 시작하기 위해서는 최소 6명의 플레이어가 필요합니다.\n";
        cout << "현재 플레이어 수: " << playlist.size() << "명\n\n";
        return;
    }

    cout << "게임이 시작되었습니다\n\n";
    assignRoles();
    currentDay = 1;

    while (true)
    {
        startNight(); // 밤 진행 (능력 사용)

        if (checkVictoryCondition())
        { // 밤 행동 후 승리 조건 체크
            break;
        }

        startDay(); // 낮 진행 (결과 처리, 생존자 목록, 토론)

        if (checkVictoryCondition())
        { // 투표 후 승리 조건 체크
            break;
        }

        currentDay++;
    }
}

bool checkVictoryCondition()
{
    int mafiaCount = 0;
    int citizenCount = 0;
    for (const auto &player : players)
    {
        if (player->checkAlive())
        {
            if (player->getRole() == "마피아" || player->getRole() == "늑대인간")
            {
                mafiaCount++;
            }
            else
                citizenCount++;
        }
    }

    if (mafiaCount == 0)
    {
        cout << "\n시민 팀이 승리했습니다!\n";
        return true;
    }
    else if (mafiaCount >= citizenCount)
    {
        cout << "\n마피아 팀이 승리했습니다\n";
        return true;
    }
    return false;
}

void startNight()
{
    isNight = true;
    cout << "\n=== " << currentDay << "번째 밤이 되었습니다 ===\n\n";
    nightResults.clear();

    // 단계 1: 모든 플레이어의 능력 사용
    for (const auto &player : players)
    {
        system("cls");
        char input;
        while (true)
        {
            cout << player->getName() << "님이 맞으시다면 (Y/y)를 입력해주세요: ";
            cin >> input;
            if (input == 'y' || input == 'Y')
                break;
            cout << player->getName() << "님이 아닌 것 같습니다. 해당 플레이어가 직접 시도해주세요.\n";
            clearInputBuffer();
        }

        // 직업 확인 및 능력 사용
        cout << "\n=== " << player->getName() << "님의 차례 ===\n";
        cout << "당신의 직업은 " << player->getRole() << "입니다.\n";

        // 능력 사용
        yourTurn(player);

        cout << "\n다음 플레이어로 넘어가려면 아무 키나 누르세요...";
        clearInputBuffer();
        cin.get();
    }

    // 단계 2: 행동 결과 처리
    nightManager.processActions();

    // 단계 3: 각 플레이어별 결과 확인
    for (const auto &player : players)
    {
        system("cls");
        char input;
        while (true)
        {
            cout << player->getName() << "님이 맞으시다면 (Y/y)를 입력해주세요: ";
            cin >> input;
            if (input == 'y' || input == 'Y')
                break;
            cout << player->getName() << "님이 아닌 것 같습니다. 해당 플레이어가 직접 시도해주세요.\n";
            clearInputBuffer();
        }

        cout << "\n=== " << player->getName() << "님의 결과 ===\n";
        showResults(player->getName());

        cout << "\n다음 플레이어로 넘어가려면 아무 키나 누르세요...";
        clearInputBuffer();
        cin.get();
    }
}

void startDay()
{
    isNight = false;
    cout << "\n=== " << currentDay << "번째 날이 밝았습니다 ===\n";

    // 전날 밤의 행동 결과 처리
    nightManager.processActions();
    nightManager.clear();

    // 생존자 확인
    cout << "\n=== 생존자 목록 ===\n";
    for (const auto &player : players)
    {
        if (player->checkAlive())
        {
            cout << player->getName() << "\n";
        }
    }

    cout << "\n토론 시간입니다. 60초 후 투표가 시작됩니다...\n";
    std::this_thread::sleep_for(std::chrono::seconds(60)); // 토론 시간

    startVoting(); // 투표 진행
}

#endif // FUNCTION_H