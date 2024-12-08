// function.h
#ifndef FUNCTION_H
#define FUNCTION_H

#include <ctime>
#include <functional>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <locale>
#include <codecvt>
#include "jobs.h"

using namespace std;
using namespace std::chrono;

// 전방 선언
class NightPhaseManager;
void checkWerewolfTaming(shared_ptr<Player> currentPlayer, shared_ptr<Player> target);
string formatActionMessage(const string&, const string&, bool);
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
    bool isDeathMessage; // 사망 메시지 여부 저장
};

vector<NightResult> nightResults;

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
    string mafiaTarget;
    string werewolfTarget;
    bool werewolfTamed;
    vector<shared_ptr<Player>>& mafiaPlayers;
    vector<shared_ptr<Player>>& players;
    shared_ptr<Player>& werewolfPlayer;
    map<shared_ptr<Player>, bool> healedPlayers;    // 치료된 플레이어 추적
    map<shared_ptr<Player>, bool> killedPlayers;    // 죽은 플레이어 추적
    map<shared_ptr<Player>, bool> defendedPlayers;  // 방어에 성공한 플레이어 추적
    map<shared_ptr<Player>, bool> willDiePlayers;   // 죽을 예정인 플레이어

public:
    NightPhaseManager(
        vector<shared_ptr<Player>>& mafia_players,
        vector<shared_ptr<Player>>& all_players,
        shared_ptr<Player>& werewolf_player)
        : mafiaTarget(""),
        werewolfTamed(false),
        mafiaPlayers(mafia_players),
        players(all_players),
        werewolfPlayer(werewolf_player)
    {
        actionPriorities = {
            {"늑대인간", 1},
            {"의사", 2},
            {"마피아", 3},
            {"군인", 4},
            {"시민", 5} };
    }

    bool wasDefended(const shared_ptr<Player>& player) const {
        auto it = defendedPlayers.find(player);
        return it != defendedPlayers.end() && it->second;
    }

    string getDefendedPlayerName() const {
        for (const auto& pair : defendedPlayers) {
            if (pair.second) {
                return pair.first->getName();
            }
        }
        return "";
    }

    void setWerewolfTamed(bool tamed) {
        werewolfTamed = tamed;
    }

    bool isWerewolfTamed() const {
        return werewolfTamed;
    }

    void setMafiaTarget(const string& target) {
        mafiaTarget = target;
    }

    const string& getMafiaTarget() const {
        return mafiaTarget;
    }

    const vector<NightAction>& getActions() const {
        return actions;
    }

    void clear()
    {
        actions.clear();
        mafiaTarget.clear();
        healedPlayers.clear();
        killedPlayers.clear();
        defendedPlayers.clear();
    }

    void removeAction(shared_ptr<Player> actor, const string& actionType)
    {
        actions.erase(
            remove_if(actions.begin(), actions.end(),
                [actor, actionType](const NightAction& action)
                {
                    return action.actor == actor && action.actionType == actionType;
                }),
            actions.end());
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
        healedPlayers.clear();
        killedPlayers.clear();
        defendedPlayers.clear();
        willDiePlayers.clear();

        bool werewolfTargetMatch = false;
        bool targetKilled = false;
        shared_ptr<Player> matchedTarget = nullptr;

        // 우선순위에 따라 정렬
        sort(actions.begin(), actions.end(),
            [this](const NightAction& a, const NightAction& b)
            {
                return actionPriorities[a.actor->getRole()] <
                    actionPriorities[b.actor->getRole()];
            });

        // 각 액션 처리
        for (const auto& action : actions)
        {
            if (!action.actor->checkAlive() || !action.actor->getCanUseAbility())
                continue;

            if (action.actor->getRole() == "마피아")
            {
                // 늑대인간을 공격하는 경우 즉시 접선
                if (action.target == werewolfPlayer && !werewolfTamed) {
                    werewolfTamed = true;
                    auto werewolf = dynamic_pointer_cast<Werewolf>(werewolfPlayer);
                    if (werewolf) {
                        werewolf->setTamed(true);
                        mafiaPlayers.push_back(werewolfPlayer);

                        // 마피아팀 메시지
                        for (const auto& mafia : mafiaPlayers) {
                            if (mafia->getRole() == "마피아") {
                                nightResults.push_back({
                                mafia->getName(),
                                werewolfPlayer->getName(),
                                werewolfPlayer->getName() + "님은 늑대인간이며 당신에게 길들여졌습니다!",
                                true,
                                false
                                });
                            }
                        }

                        // 늑대인간 메시지
                        string mafiaTeamInfo = "";
                        for (const auto& mafia : mafiaPlayers) {
                            if (mafia->getRole() == "마피아" && mafia->checkAlive()) {
                                if (!mafiaTeamInfo.empty()) {
                                    mafiaTeamInfo += ", ";
                                }
                                mafiaTeamInfo += mafia->getName();
                            }
                        }

                        nightResults.push_back({
                            werewolfPlayer->getName(),
                            "",
                            mafiaTeamInfo + "님이 마피아이며 당신과 접선하였습니다!",
                            true,
                            false
                        });
                    }
                }
                // 일반적인 마피아의 공격 처리 (늑대인간 제외)
                else if (action.target != werewolfPlayer) {
                    
                    bool shouldKill = true;
                    if (auto soldier = dynamic_cast<Soldier*>(action.target.get()))
                    {
                        if (soldier->isArmorActive()) {
                            soldier->defendShot(); // Armor 소모
                            defendedPlayers[action.target] = true;
                            healedPlayers.erase(action.target); // 이 때 의사의 치료는 무효
                            shouldKill = false;

                            // 군인에게 보내는 게인 메시지
                            nightResults.push_back({
                                action.target->getName(),
                                action.actor->getName(),
                                "마피아가 당신에게 총을 겨누었지만, 방탄복으로 버텨냈습니다.",
                                true,
                                false
                                });
                        }
                    }
                    if (shouldKill) {
                        killedPlayers[action.target] = true;
                        if (action.target->getName() == mafiaTarget) {
                            matchedTarget = action.target;
                        }
                    }
                }
            }
            else if (action.actor->getRole() == "늑대인간")
            {
                auto werewolf = dynamic_pointer_cast<Werewolf>(action.actor);
                if (werewolf && !werewolf->isTamed() && action.target->getName() == mafiaTarget)
                {
                    werewolfTargetMatch = true;
                }
                else if (werewolf && werewolf->isTamed())
                {
                    // 길들여진 늑대인간의 공격은 무조건 성공
                    killedPlayers[action.target] = true;
                    healedPlayers.erase(action.target); // 의사의 치료 무시
                }
            }
            else if (action.actor->getRole() == "의사")
            {
                if (!defendedPlayers[action.target]) { // 방어되지 않은 대상만 치료
                    healedPlayers[action.target] = true;
                }
            }
            // 다른 직업들의 능력은 즉시 처리
            else
            {
                action.actor->action(*action.target);
            }
        }

        // 늑대인간 접선 조건 체크
        if (werewolfTargetMatch && !werewolfTamed && matchedTarget)
        {
            // 해당 타겟이 실제로 죽는지 확인
            if (killedPlayers[matchedTarget] && !healedPlayers[matchedTarget] && !defendedPlayers[matchedTarget])
            {
                auto werewolf = dynamic_pointer_cast<Werewolf>(werewolfPlayer);
                if (werewolf) {
                    werewolfTamed = true;
                    werewolf->setTamed(true);
                    mafiaPlayers.push_back(werewolfPlayer);

                    // 마피아팀 메시지
                    for (const auto& mafia : mafiaPlayers) {
                        if (mafia->getRole() == "마피아") {
                            nightResults.push_back({
                                mafia->getName(),
                                werewolfPlayer->getName(),
                                werewolfPlayer->getName() + "님은 늑대인간이며 당신에게 길들여졌습니다!",
                                true,
                                false
                            });
                        }
                    }

                    // 늑대인간 메시지
                    string mafiaTeamInfo = "";
                    for (const auto& mafia : mafiaPlayers) {
                        if (mafia->getRole() == "마피아" && mafia->checkAlive()) {
                            if (!mafiaTeamInfo.empty()) mafiaTeamInfo += ", ";
                            mafiaTeamInfo += mafia->getName();
                        }
                    }

                    nightResults.push_back({
                        werewolfPlayer->getName(),
                        "",
                        mafiaTeamInfo + "님이 마피아이며 당신과 접선하였습니다!",
                        true,
                        false
                    });
                }
            }
        }

        bool anyEvent = false;

        // 방어 성공 메시지 출력
        for (const auto& pair : defendedPlayers) {
            if (pair.second) {
                cout << pair.first->getName() << "님이 방탄복으로 마피아의 총격을 버텨냈습니다!\n";
                anyEvent = true;
            }
        }

        // 사망 처리 및 메시지 생성

        for (const auto& pair : killedPlayers)
        {
            if (pair.second && !healedPlayers[pair.first] && !defendedPlayers[pair.first])
            {
                willDiePlayers[pair.first] = true;
                nightResults.push_back({
                    pair.first->getName(),
                    "",
                    "당신은 사망하셨습니다.",
                    true,
                    true
                });
            }
        }

        // willDiePlayers 정보를 다음 날 사용하기 위해 저장
        for (const auto& pair : willDiePlayers) {
            nightResults.push_back({
                "SYSTEM",
                pair.first->getName(),
                "DEATH_MARK", // 다음 날 처리를 위한 마커
                false,
                true
            });
        }
    }
};

// 전역 변수 선언
vector<string> playlist;
vector<shared_ptr<Player>> players;
vector<shared_ptr<Player>> mafiaPlayers;
shared_ptr<Player> mafiaTargetPlayer;
shared_ptr<Player> previousMafia;
shared_ptr<Player> werewolfPlayer;
static NightPhaseManager nightManager(mafiaPlayers, players, werewolfPlayer);
string mafiaTarget;
string werewolfTarget;
int currentDay = 1;
bool werewolfTamed = false;

// 유틸리티 함수
void clearInputBuffer()
{ // 입력 버퍼를 비우는 함수
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void showPlayerList()
{ // 플레이어 관리 함수들
    cout << "===플레이어 목록 ===\n\n";
    for (size_t i = 0; i < playlist.size(); i++)
    {
        cout << i + 1 << ". " << playlist[i] << "\n";
    }
    cout << "\n전체: " << playlist.size() << "명\n";
}

void showResults(const string& playerName)
{
    bool foundResult = false;

    auto currentPlayer = find_if(players.begin(), players.end(),
        [&playerName](const auto& player) {return player->getName() == playerName; });

    if (currentPlayer != players.end() && !(*currentPlayer)->checkAlive()) {
        cout << "[받은 영향] 당신은 사망하셨습니다.\n";
        foundResult = true;
    }

    // 행동 결과
    for (const auto& result : nightResults)
    {
        if (result.playerName == playerName && result.isPrivate)
        {
            cout << "[행동 결과] " << result.message << "\n";
            foundResult = true;
        }
    }

    if (!foundResult)
    {
        cout << "[받은 영향] 아무런 일도 일어나지 않았습니다...\n";
    }
}

string formatActionMessage(const string& actorRole, const string& action, bool isReceived = false) {
    if (isReceived) {
        if (actorRole == "마피아") return "마피아에게 공격받았습니다.";
    }
    else {
        if (actorRole == "마피아") return "님을 처지 대상으로 지정합니다.";
        if (actorRole == "의사") return "을(를) 치료합니다.";
        if (actorRole == "경찰") return "을(를) 조사합니다.";
        if (actorRole == "늑대인간") return "을(를) 먹잇감으로 선정합니다.";
    }
    return "";
}

void yourTurn(shared_ptr<Player> currentPlayer)
{
    if (!currentPlayer->getCanUseAbility()) // 구현은 했지만, 직업 삭제로 사용 x
    {
        cout << "현재 능력을 사용할 수 없습니다.\n";
        return;
    }

    // 1. 능력이 없는 직업 체크
    if (currentPlayer->getRole() == "군인" || currentPlayer->getRole() == "시민")
    {
        cout << "당신은 밤에 수행할 수 있는 역할이 없습니다.\n";
        return;
    }

    // 3. 유효한 타겟 목록 표시
    vector<shared_ptr<Player>> validTargets;
    for (const auto& player : players)
    {
        if (player->checkAlive())
        {
            validTargets.push_back(player);
            cout << validTargets.size() << ". " << player->getName() << "\n";
        }
    }

    // 4. 마피아 특별 처리
    if (currentPlayer->getRole() == "마피아")
    {
        cout << "\n=== 마피아 팀 정보 ===\n";
        for (const auto& mafia : mafiaPlayers) {
            if (mafia->getName() != currentPlayer->getName() && mafia->getRole() == "마피아") {
                cout << mafia->getName() << "님은 " << mafia->getRole() << "입니다.\n";
            }
        }

        // 늑대인간의 타겟 정보 표시
        auto werewolf = dynamic_pointer_cast<Werewolf>(werewolfPlayer);
        if (werewolf && werewolf->isTamed()) {
            cout << werewolfPlayer->getName() << "님은 늑대인간입니다.\n";
            if (!werewolfTarget.empty()) {
                cout << "\n늑대인간이 " << werewolfTarget << "님을 살육의 대상으로 지정했습니다.\n";
            }
        }

        // 이미 다른 마피아가 타겟을 선택했는지 확인
        if (!mafiaTarget.empty())
        {
            cout << "\n다른 마피아가 " << mafiaTarget << "님을 처치 대상으로 지목했습니다.\n";
            cout << "바꾸시겠습니까? (Y/N): ";

            char choice = 'N';
            clearInputBuffer();

            while (!(cin >> choice))
            {
                clearInputBuffer();
                cout << "잘못된 입력입니다. Y 또는 N을 입력해주세요: ";
            }

            choice = toupper(choice);

            if (choice != 'Y' && choice != 'N')
            {
                cout << "잘못된 입력입니다. 타겟을 변경하지 않습니다.\n";
                return;
            }

            if (choice == 'N')
            {
                for (const auto& validTarget : validTargets) {
                    if (validTarget->getName() == mafiaTarget) {
                        nightResults.push_back({
                            currentPlayer->getName(),
                            validTarget->getName(),
                            validTarget->getName() + formatActionMessage("마피아", "attack"),
                            true,
                            false
                            });
                        nightManager.addAction(currentPlayer, validTarget, currentPlayer->getRole());
                        break;
                    }
                }
                return;
            }

            if (choice > 0 && choice <= static_cast<int>(validTargets.size()))
            {
                shared_ptr<Player> target = validTargets[choice - 1];
                checkWerewolfTaming(currentPlayer, target);
            }
        }
    }
    else if (currentPlayer->getRole() == "늑대인간") {
        auto werewolf = dynamic_pointer_cast<Werewolf>(currentPlayer);
        if (werewolf && werewolf->isTamed()) {
            cout << "\n=== 마피아 팀 정보 ===\n";
            for (const auto& member : mafiaPlayers) {
                if (member->getRole() == "마피아" && member->checkAlive()) {
                    cout << member->getName() << "님은 " << member->getRole() << "입니다.\n";
                }
            }

            // 마피아의 타겟 정보 표시
            if (!mafiaTarget.empty()) {
                cout << "\n마피아가 " << mafiaTarget << "님을 처치 대상으로 지목했습니다.\n";
            }
        }
    }


    // 5. 타겟 선택 처리
    int choice;
    while (true)
    {
        cout << "\n능력을 사용할 대상을 선택하세요 (0: 능력 사용하지 않음): ";
        if (!(cin >> choice))
        {
            clearInputBuffer();
            cout << "잘못된 입력입니다.\n";
            continue;
        }

        if (choice == 0)
        {
            cout << "능력 사용을 취소했습니다.\n";
            return;
        }

        if (choice > 0 && choice <= static_cast<int>(validTargets.size()))
        {
            shared_ptr<Player> target = validTargets[choice - 1];

            // 6. 직업별 능력 사용 처리
            // 6.1 경찰 능력
            if (currentPlayer->getRole() == "경찰")
            {
                string result;
                if (target->getRole() == "마피아")
                {
                    result = "마피아입니다.";
                }
                else
                {
                    result = "마피아가 아닙니다.";
                }
                nightResults.push_back({ currentPlayer->getName(),
                                        target->getName(),
                                        target->getName() + "(은)는 " + result,
                                        true });
                nightManager.addAction(currentPlayer, target, currentPlayer->getRole());
            }
            // 6.2 마피아 능력
            else if (currentPlayer->getRole() == "마피아")
            {
                // 이전 마피아의 액션이 있었다면 제거
                if (previousMafia)
                {
                    nightManager.removeAction(previousMafia, "마피아");
                    string prevMafiaName = previousMafia->getName();
                    // 이전 결과 제거
                    nightResults.erase(
                        remove_if(nightResults.begin(), nightResults.end(),
                            [prevMafiaName](const NightResult& result)
                            {
                                return result.playerName == prevMafiaName ||
                                    (result.playerName == "마피아" &&
                                        result.targetName == mafiaTarget);
                            }),
                        nightResults.end());
                }

                // 새로운 타겟 정보 저장
                mafiaTarget = target->getName();
                nightManager.setMafiaTarget(target->getName());
                mafiaTargetPlayer = target;
                previousMafia = currentPlayer;

                // 행동 결과 저장 - 공격자 시점
                nightResults.push_back({
                    currentPlayer->getName(),
                    target->getName(),
                    target->getName() + formatActionMessage("마피아", "attack"),
                    true
                    });

                // 타겟 시점의 메시지
                nightResults.push_back({
                    "마피아",
                    target->getName(),
                    formatActionMessage("마피아", "attack", true),
                    true
                    });

                nightManager.addAction(currentPlayer, target, currentPlayer->getRole());
            }
            // 6.3 의사 능력
            else if (currentPlayer->getRole() == "의사")
            {
                nightResults.push_back({ currentPlayer->getName(),
                                        target->getName(),
                                        target->getName() + "을(를) 치료하기로 했습니다.",
                                        true });
                nightManager.addAction(currentPlayer, target, currentPlayer->getRole());
            }
            // 6.4 늑대인간 능력
            else if (currentPlayer->getRole() == "늑대인간")
            {
                nightResults.push_back({ currentPlayer->getName(),
                                        target->getName(),
                                        target->getName() + "님을 대상으로 지정했습니다.",
                                        true });
                nightManager.addAction(currentPlayer, target, currentPlayer->getRole());
                werewolfTarget = target->getName(); // 늑대인간의 타겟 저장

                if (!mafiaTarget.empty() && target->getName() == mafiaTarget) {
                    checkWerewolfTaming(currentPlayer, target);
                }
            }

            cout << "능력 사용이 완료되었습니다.\n";
            break;
        }
        cout << "잘못된 선택입니다. 다시 선택해주세요.\n";
    }

    clearInputBuffer();
}

// 게임 로직 함수
void playerModify()
{ // 플레이어 수정 및 관리 함수
    system("cls");

    int player_cnt = playlist.size();
    while (true)
    {
        showPlayerList();
        cout << "\n1. 플레이어 추가\n";
        cout << "\n2. 플레이어 삭제\n";
        cout << "\n3. 돌아가기\n\n";
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
                    cout << "이름이 비어있습니다. 다시 입력해주세요. \n\n";
                    --i; // 반복 횟수 보정
                    continue;
                }
                playlist.push_back(name);
                cout << name << " 플레이어가 등록 되었습니다.\n\n";
                ++player_cnt;
            }
            system("cls");
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
                system("cls");
            }
            else
            {
                cout << "잘못된 번호입니다.\n";
            }
            break;
        }
        case 3:
            system("cls");
            return;
        default:
            cout << "잘못된 입력입니다.\n";
        }
    }
}

shared_ptr<Player> createRole(const string& name, int roleType)
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
    players.clear();
    mafiaPlayers.clear();
    werewolfTamed = false;

    int totalPlayers = playlist.size();
    vector<bool> assigned(totalPlayers, false);
    random_device rd;
    mt19937 gen(rd());

    // 필수 직업 할당
    uniform_int_distribution<> dis(0, totalPlayers - 1); // 1. 경찰 할당
    int policeIndex = dis(gen);
    players.push_back(make_shared<Police>(playlist[policeIndex]));
    assigned[policeIndex] = true;

    int doctorIndex; // 2. 의사 할당
    do
    {
        doctorIndex = dis(gen);
    } while (assigned[doctorIndex]);
    players.push_back(make_shared<Doctor>(playlist[doctorIndex]));
    assigned[doctorIndex] = true;

    int mafiaCount = (totalPlayers == 8) ? 2 : 1; // 3. 마피아 할당 (8명일 때만 2명, 그 외에는 1명)
    for (int i = 0; i < mafiaCount; i++)
    {
        int mafiaIndex;
        do
        {
            mafiaIndex = dis(gen);
        } while (assigned[mafiaIndex]);
        auto mafia = make_shared<Mafia>(playlist[mafiaIndex]);
        players.push_back(mafia);
        mafiaPlayers.push_back(mafia); // 마피아 플레이어 저장
        assigned[mafiaIndex] = true;
    }

    int werewolfIndex; // 4. 늑대인간 할당
    do
    {
        werewolfIndex = dis(gen);
    } while (assigned[werewolfIndex]);
    werewolfPlayer = make_shared<Werewolf>(playlist[werewolfIndex]);
    players.push_back(werewolfPlayer);
    assigned[werewolfIndex] = true;

    if (totalPlayers == 8) { // 5. 군인은 8명일 때만 할당
        int soldierIndex;
        do {
            soldierIndex = dis(gen);
        } while (assigned[soldierIndex]);
        players.push_back(make_shared<Soldier>(playlist[soldierIndex]));
        assigned[soldierIndex] = true;
    }

    for (int i = 0; i < totalPlayers; i++)
    { // 6. 나머지는 모두 시민으로 할당
        if (!assigned[i])
        {
            players.push_back(make_shared<Citizen>(playlist[i]));
        }
    }

    shuffle(players.begin(), players.end(), gen); // 플레이어 순서 랜덤
}

void checkWerewolfTaming(shared_ptr<Player> currentPlayer, shared_ptr<Player> target)
{
    if (!werewolfPlayer || werewolfTamed)
        return;

    auto werewolf = dynamic_pointer_cast<Werewolf>(werewolfPlayer);
    if (!werewolf)
        return;

    // 1. 마피아가 늑대인간 공격한 경우
    if (currentPlayer->getRole() == "마피아" && target == werewolfPlayer) {
        // 이 경우는 즉시 접선 (늑대인간이 직접 타겟이 된 경우)
        werewolfTamed = true;
        werewolf->setTamed(true);
        mafiaPlayers.push_back(werewolfPlayer); // 마피아팀과 공유

        string mafiaMessage = target->getName() + "님은 늑대인간이며 당신과 접선하였습니다!";
        string werewolfMessage = target->getName() + "님은 마피아이며 당신과 접선하였습니다!";

        // 마피아 팀 전체에 동일 메시지 전달
        for (const auto& mafia : mafiaPlayers) {
            nightResults.push_back({
                mafia->getName(),
                werewolfPlayer->getName(),
                mafiaMessage,
                true,
                false
                });
        }

        // 늑대인간에게 메시지 전달
        nightResults.push_back({
            werewolfPlayer->getName(),
            currentPlayer->getName(),
            werewolfMessage,
            true,
            false
            });
    }

    // 2. 마피아와 늑대인간의 타겟 일치
    else if (currentPlayer->getRole() == "늑대인간" && !mafiaTarget.empty() && target->getName() == mafiaTarget)
    {
        nightResults.push_back({
            "SYSTEM",
            "CHECK_TAMING",
            target->getName(),  // 타겟 이름 저장
            false,
            false
            });
    }
}

void gameRule()
{
    string originalLocale = setlocale(LC_ALL, nullptr); // 현재 로케일 저장

    cout << "\n=== 마피아 게임 규칙 ===\n\n";

    try {
        setlocale(LC_ALL, ".UTF-8"); // UTF-8 출력을 위한 로케일 설정
        wifstream file("mafiarule.txt");
        if (!file.is_open()) {
            throw runtime_error("Cannot open file");
        }
        file.imbue(locale(locale(), new codecvt_utf8<wchar_t>));
        wcout.imbue(locale(".UTF-8"));
        wstring line;
        while (getline(file, line)) {
            wcout << line << endl;
        }
        setlocale(LC_ALL, originalLocale.c_str()); // 원래 로케일로 복귀
        file.close();
        cout << endl;
    }
    catch (const runtime_error& e) {
        setlocale(LC_ALL, originalLocale.c_str());
        cout << "1. 게임은 최소 6명의 플레이어가 필요합니다.\n";
        cout << "2. 각 플레이어는 게임 시작 시 랜덤으로 직업을 부여받습니다.\n";
        cout << "3. 게임은 낮과 밤으로 진행됩니다.\n";
        cout << "4. 낮에는 토론과 투표를 통해 용의자를 처형합니다.\n";
        cout << "5. 밤에는 각자의 직업에 따른 역할을 수행합니다.\n";
        cout << "6. 마피아를 모두 제거하거나, 마피아가 선량한 시민 수와 같아지면 게임이 종료됩니다.\n\n";
    }
    cout << "계속하려면 Enter키를 눌러주세요...";
    clearInputBuffer();
    cin.get();
    system("cls");
}

// 게임 진행 함수
void startVoting()
{
    cout << "\n=== 투표를 시작합니다 ===\n";
    map<shared_ptr<Player>, int> votes;
    vector<shared_ptr<Player>> alivePlayers;

    // 살아있는 플레이어의 목록 생성
    for (const auto& player : players)
    {
        if (player->checkAlive())
        {
            alivePlayers.push_back(player);
        }
    }
    // 1차 투표 진행
    for (const auto& voter : players)
    {
        if (voter->checkAlive() && voter->getCanVote())
        {
            system("cls");
            cout << "=== 투표 진행 중 ===\n\n";
            cout << voter->getName() << "의 투표\n\n";

            // 투표 가능한 플레이어 목록 표시
            cout << "0. 기권\n";
            for (size_t i = 0; i < alivePlayers.size(); i++) {
                cout << i + 1 << ". " << alivePlayers[i]->getName() << endl;
            }

            int choice;
            while (1) {
                cout << "\n투표할 대상을 선택하세요 : ";
                if (cin >> choice) {
                    if (choice == 0) {
                        cout << "투표를 기권했습니다.\n";
                        break;
                    }
                    else if (choice > 0 && choice <= static_cast<int>(alivePlayers.size())) {
                        votes[alivePlayers[choice - 1]]++;
                        break;
                    }
                }
                clearInputBuffer();
                cout << "잘못된 입력입니다. 다시 선택해주세요.\n";
            }
            clearInputBuffer();
        }
    }

    // 최다 득표자 확인
    shared_ptr<Player> maxVotePlayer = nullptr;
    int maxVotes = 0;
    bool isDuplicate = false;

    for (const auto& vote : votes) {
        if (vote.second > maxVotes) {
            maxVotes = vote.second;
            maxVotePlayer = vote.first;
            isDuplicate = false;
        }
        else if (vote.second == maxVotes) {
            isDuplicate = true;
        }
    }

    // 최다 득표자가 한 명일 경우에만 찬반 투표 진행
    if (maxVotePlayer && !isDuplicate && maxVotes > 0) {
        cout << "\n=== " << maxVotePlayer->getName() << "님에 대한 최종 찬반 투표를 진행합니다 ===\n";
        int agree = 0, disagree = 0;

        for (const auto& voter : players) {
            if (voter->checkAlive() && voter->getCanVote()) {
                cout << voter->getName() << "의 투표 (1: 찬성, 2: 반대): ";
                int choice;
                cin >> choice;
                if (choice == 1) agree++;
                else if (choice == 2) disagree++;
            }
        }
        cout << "\n=== 찬반 투표 결과 ===\n";
        cout << "찬성: " << agree << "표\n";
        cout << "반대: " << disagree << "표\n";

        if (agree > disagree) {
            cout << maxVotePlayer->getName() << "님이 투표로 처형되었습니다.\n";
            maxVotePlayer->setAlive(false);
        }
        else cout << "과반수를 넘기지 않아 무효처리 되었습니다.\n";
    }
    else {
        if (maxVotes == 0) cout << "\n아무도 투표하지 않았습니다\n";
        else cout << "투표자 동률 발생으로 인해 투표가 무효처리 되었습니다\n";
    }
    cout << "5초 후에 게임이 재개됩니다.\n";
    std::this_thread::sleep_for(std::chrono::seconds(5)); // 결과를 볼 수 있도록 5초의 딜레이

    clearInputBuffer();
    cin.ignore();
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
    for (const auto& player : players)
    {
        if (player->checkAlive())
        {
            if (player->getRole() == "마피아" || (player->getRole() == "늑대인간" && werewolfTamed))
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
        cout << "\n 계속하려면 Enter키를 눌러주세요...";
        clearInputBuffer();
        cin.get();
        system("cls");
        return true;
    }
    else if (mafiaCount >= citizenCount)
    {
        cout << "\n마피아 팀이 승리했습니다\n";
        cout << "\n계속하려면 Enter키를 눌러주세요...";
        clearInputBuffer();
        cin.get();
        system("cls");
        return true;
    }
    return false;
}

void startNight()
{
    cout << "\n=== " << currentDay << "번째 밤이 되었습니다 ===\n\n";
    nightResults.clear();
    mafiaTarget.clear(); // 마피아 타겟 초기화
    werewolfTarget.clear(); // 늑대인간 타겟 초기화
    mafiaTargetPlayer = nullptr;
    previousMafia = nullptr;

    // 단계 1: 살아있는 플레이어의 능력 사용
    for (const auto& player : players)
    {
        if (!player->checkAlive())
            continue; // 죽은 플레이어 스킵

        system("cls");
        char input;
        while (true)
        {
            cout << player->getName() << "님이 맞으시다면 Y를 입력해주세요: ";
            cin >> input;
            if (input == 'Y' || input == 'y' || input == 'ㅛ')
                break;
            cout << player->getName() << "님이 아닌 것 같습니다. 해당 플레이어가 직접 시도해주세요.\n";
            clearInputBuffer();
        }

        // 직업 확인 및 능력 사용
        cout << "\n=== " << player->getName() << "님의 차례 ===\n";
        cout << "당신의 직업은 " << player->getRole() << "입니다.\n\n";

        // 능력 사용
        yourTurn(player);

        cout << "\n다음 플레이어로 넘어가려면 Enter키를 눌러주세요...";
        clearInputBuffer();
        cin.get();
    }

    // 단계 2: 행동 결과 처리
    nightManager.processActions();

    // 단계 3: 각 플레이어별 결과 확인
    for (const auto& player : players)
    {
        if (!player->checkAlive())
            continue;

        system("cls");
        char input;
        while (true)
        {
            cout << player->getName() << "님이 맞으시다면 Y를 입력해주세요: ";
            cin >> input;
            if (input == 'Y' || input == 'y' || input == 'ㅛ')
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

void startDay() {
    cout << "\n=== " << currentDay << "번째 날이 밝았습니다 ===\n";

    bool anyEvent = false;
    bool anyAttack = false;
    bool doctorSaved = false;
    string savedPlayerName;
    vector<string> deathMessages; // 사망 메시지 수집용

    // 방어 성공 시 메시지 출력
    string defendedName = nightManager.getDefendedPlayerName();
    if (!defendedName.empty()) {
        cout << defendedName << "님이 방탄복으로 마피아의 총격을 버텨냈습니다!\n";
        anyEvent = true;
        anyAttack = true;
    }

    for (const auto& result : nightResults) {
        if (result.playerName == "SYSTEM" && result.message == "DEATH_MARK") {
            auto target = find_if(players.begin(), players.end(),
                [&result](const shared_ptr<Player>& p) {
                    return p->getName() == result.targetName;
                });
            // 방어에 성공한 플레이어 제외하고 사망
            if (target != players.end() && target->get()->getName() != defendedName) {
                (*target)->setAlive(false);
                deathMessages.push_back(result.targetName + "님이 사망했습니다.");
                anyEvent = true;
                anyAttack = true;
            }
        }
    }

    // 의사 치료 헤크
    for (const auto& action : nightManager.getActions()) {
        if (action.actor->getRole() == "마피아" ||
            (action.actor->getRole() == "늑대인간" &&
                dynamic_pointer_cast<Werewolf>(action.actor)->isTamed())) {
            anyAttack = true;
        }
        else if (action.actor->getRole() == "의사") {
            if (action.target->checkAlive() &&
                any_of(nightManager.getActions().begin(), nightManager.getActions().end(),
                    [&action](const NightAction& attack) {
                        return (attack.actor->getRole() == "마피아" ||
                            (attack.actor->getRole() == "늑대인간" &&
                                dynamic_pointer_cast<Werewolf>(attack.actor)->isTamed())) &&
                            attack.target == action.target;
                    })) {
                doctorSaved = true;
                savedPlayerName = action.target->getName();
            }
        }
    }

    if (doctorSaved) {
        cout << savedPlayerName << "님이 의사의 치료를 받고 살아났습니다!\n";
        anyEvent = true;
    }

    // 메시지 출력
    if (!deathMessages.empty()) {
        for (const auto& msg : deathMessages) {
            cout << msg << endl;
        }
    }
    else if (!anyEvent && !anyAttack) {
        cout << "아무런 일도 일어나지 않았습니다.\n";
    }

    nightManager.clear();

    // 생존자 확인
    cout << "\n=== 생존자 목록 ===\n";
    for (const auto& player : players) {
        if (player->checkAlive()) {
            cout << player->getName() << "\n";
        }
    }

    cout << "\n토론 시간입니다. 30초 후 투표가 시작됩니다...\n";
    std::this_thread::sleep_for(std::chrono::seconds(30));

    startVoting();
}

#endif // FUNCTION_H