#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

// 전방 선언
class Player;
class Doctor;

class GameState { // 게임 상태 관리
private:
    static GameState* instance;
    vector<shared_ptr<Player>> players;
    GameState() {}


public:
    static GameState* getInstance() {
        if (instance == nullptr) {
            instance = new GameState();
        }
        return instance;
    }

    void addPlayer(shared_ptr<Player> player) {
        players.push_back(player);
    }

    const vector<shared_ptr<Player>>& getPlayers() const {
        return players;
    }
};

class Player {
protected: // 상속받은 클래스에서 사용하기 위해 protected로 선언
    string name; // 이름
    bool isAlive; // 생존 여부
    bool canVote; // 투표 가능 여부
    bool canUseAbility; // 능력 사용 가능 여부

public:
    Player(string n) : name(n), isAlive(true), canVote(true), canUseAbility(true) {}
    virtual ~Player() {}

    virtual void setName(string n) { name = n; } // 이름 설정
    string getName() const { return name; }
    bool checkAlive() const { return isAlive; } // 생존 여부
    void setAlive(bool alive) { isAlive = alive; } // bool 함수로 생사 여부를 확인
    void setCanVote(bool can) { canVote = can; } // bool 함수로 투표 가능 여부를 확인
    bool getCanVote() const { return canVote; }
    void setCanUseAbility(bool can) { canUseAbility = can; } // bool 함수로 고유 능력 사용 여부를 확인
    bool getCanUseAbility() const { return canUseAbility; }

    virtual void action(Player& target) = 0; // 직업 고유 능력을 구현하기 위한 가상함수 설정
    virtual string getRole() const = 0; // 정체를 드러내기 위한 가상 함수
};

class Mafia : public Player { // 마피아
private:
    bool exposed;  // 정체가 발각되었는지 여부

public:
    Mafia(string n) : Player(n), exposed(false) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        if (target.checkAlive()) {
            target.setAlive(false);
            cout << target.getName() << " (이)가 총을 맞고 사망했습니다.\n";
        }
    }

    string getRole() const override { return "Mafia"; }
};

class Spy : public Player { // 스파이
private:
    bool contactedMafia; // 마피아와 접촉했는지 여부 확인

public:
    Spy(string n) : Player(n), contactedMafia(false) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        cout << target.getName() << "의 직업은 " << target.getRole() << "입니다.\n";
        if (target.getRole() == "Mafia") {
            contactedMafia = true;
            cout << "마피아와 접선했습니다!\n";
        }
    }

    string getRole() const override { return "Spy"; }
};

class Werewolf : public Player { // 늑대인간
private:
    bool tamed; // 길들여졌는지 여부

public:
    Werewolf(string n) : Player(n), tamed(false) {}
    
    void action(Player& target) override {
        if (!canUseAbility) return;
        if (tamed && target.checkAlive()) {
            target.setAlive(false);
            cout << target.getName() << "이(가) 늑대인간에게 살육당했습니다.\n";
        }
    }
    
    void setTamed(bool isTamed) { tamed = isTamed; }
    bool isTamed() const { return tamed; }
    string getRole() const override { return "Werewolf"; }
};

class Madame : public Player { // 마담
private:
    bool contactedMafia;
    
public:
    Madame(string n) : Player(n), contactedMafia(false) {}
    
    void action(Player& target) override {
        if (!canUseAbility) return;
        target.setCanUseAbility(false);
        cout << target.getName() << "이(가) 유혹당해 능력을 사용할 수 없게 되었습니다.\n";
        
        if (target.getRole() == "Mafia") {
            contactedMafia = true;
            cout << "마피아와 접선했습니다!\n";
        }
    }
    
    string getRole() const override { return "Madame"; }
};

class Scientist : public Player { // 과학자
private:
    bool hasResurrection; // 부활했는지 여부 확인
    bool contactedMafia; // 마피아와 접선했는지
    
public:
    Scientist(string n) : Player(n), hasResurrection(true), contactedMafia(false) {}
    
    void action(Player&) override {
        // 과학자는 능동적인 행동이 없음
    }
    
    void onDeath(bool byVote) {
        if (!byVote && hasResurrection) {
            contactedMafia = true;
            hasResurrection = false;
            // 다음 날 부활 로직은 Game 클래스에서 처리
        }
    }
    
    bool hasJoinedMafia() const { return contactedMafia; }
    string getRole() const override { return "Scientist"; }
};

class Police : Player { // 경찰
public:
    Police(string n) : Player(n) {}

    void action(Player& target) override {
        cout << target.getName() << " (은)는 " << (dynamic_cast<Mafia*>(&target) ? "마피아 입니다." : "마피아가 아닙니다.") << ".\n";
    }
};

class Doctor : public Player { // 의사
private:
    Player* protectedTarget; // 보호할 플레이어를 포인터로 선언

public:
    Doctor(string n) : Player(n), protectedTarget(nullptr) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        protectedTarget = &target;
        cout << "오늘밤 " << target.getName() << " (을)를 보호합니다.\n";
    }

    void healIfAttacked(Player& target) {
        if (protectedTarget == &target && !target.checkAlive()) {
            target.setAlive(true);
            cout << "의사가 " << target.getName() << " (을)를 치료했습니다.\n";
        }
    }

    string getRole() const override { return "Doctor"; }
};

class Soldier : public Player { // 군인
private:
    bool armorActive;

public:
    Soldier(string n) : Player(n), armorActive(true) {}

    void action(Player& target) override {
        // 군인은 능동적인 행동이 없음
    }

    bool isArmorActive() const { return armorActive; }
    void useArmor() { armorActive = false; }

    string getRole() const override { return "Soldier"; }
};

class Thug : public Player { // 건달
public:
    Thug(string n) : Player(n) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        target.setCanVote(false);
        cout << target.getName() << " (이)가 협박당해 투표할 수 없게 되었습니다.\n";
    }

    string getRole() const override { return "Thug"; }
};

class Politician : public Player { // 정치인
public:
    Politician(string n) : Player(n) {}

    void action(Player& target) override {
        // 정치인은 능동적인 행동이 없음
    }

    int getVoteWeight() const { return 2; }

    string getRole() const override { return "Politician"; }
};

class Reporter : public Player { // 기자
private:
    bool hasUsedAbility;

public:
    Reporter(string n) : Player(n), hasUsedAbility(false) {}

    void action(Player& target) override {
        if (!canUseAbility || hasUsedAbility) return;
        if (isAlive) {
            cout << "다음 날 아침, " << target.getName() << "의 직업이 " << "공개됩니다.\n";
            hasUsedAbility = true;
        }
    }

    string getRole() const override { return "Reporter"; }
};

class Terrorist : public Player {
private:
    Player* lastTarget;
    
public:
    Terrorist(string n) : Player(n), lastTarget(nullptr) {}
    
    void action(Player& target) override {
        if (!canUseAbility) return;
        lastTarget = &target;
        cout << target.getName() << "을(를) 대상으로 지정했습니다.\n";
    }
    
    void checkRetaliation() {
        if (lastTarget && !isAlive) {
            lastTarget->setAlive(false);
            cout << lastTarget->getName() << "이(가) 테러리스트와 함께 사망했습니다.\n";
        }
    }
    
    string getRole() const override { return "Terrorist"; }
};

class Nurse : public Player { // 간호사
private:
    bool contactedDoctor;
    bool canHeal;
    
public:
    Nurse(string n) : Player(n), contactedDoctor(false), canHeal(false) {}
    
    void action(Player& target) override {
        if (!canUseAbility) return;
        if (target.getRole() == "Doctor") {
            contactedDoctor = true;
            cout << "의사와 접선했습니다!\n";
        }
    }
    
    void onDoctorDeath() {
        if (contactedDoctor) {
            canHeal = true;
            cout << "간호사가 의사의 역할을 이어받았습니다.\n";
        }
    }
    
    void heal(Player& target) {
        if (canHeal && !target.checkAlive()) {
            target.setAlive(true);
            cout << "간호사가 " << target.getName() << "을(를) 치료했습니다.\n";
        }
    }
    
    string getRole() const override { return "Nurse"; }
};

class Mercenary : public Player { // 용병
private:
    Player* client;
    bool canKill;
    
public:
    Mercenary(string n) : Player(n), client(nullptr), canKill(false) {}
    
    void setClient(Player* p) {
        if (p && p->getRole() != "Mafia") {
            client = p;
        }
    }
    
    void checkClientStatus() {
        if (client && !client->checkAlive()) {
            canKill = true;
        }
    }
    
    void action(Player& target) override {
        if (!canUseAbility || !canKill) return;
        if (target.checkAlive()) {
            target.setAlive(false);
            cout << target.getName() << "이(가) 용병에게 처형당했습니다.\n";
        }
    }
    
    string getRole() const override { return "Mercenary"; }
};

class Caveman : public Player { // 도굴꾼

};

class Cleric : public Player { // 성직자

};

class Game {
private:
    vector<shared_ptr<Player>> players;
    bool isNight;
    int dayCount;
    
public:
    Game() : isNight(false), dayCount(1) {}
    
    void addPlayer(shared_ptr<Player> player) {
        players.push_back(player);
    }
    
    void startDay() {
        isNight = false;
        cout << "\n=== " << dayCount << "일차 낮이 되었습니다 ===\n";
        
        // 사망자 확인
        checkDeaths();
        
        // 투표 진행
        conductVoting();
    }
    
    void startNight() {
        isNight = true;
        cout << "\n=== " << dayCount << "일차 밤이 되었습니다 ===\n";
        
        // 각 직업별 야간 행동 수행
        for (auto& player : players) {
            if (player->checkAlive() && player->getCanUseAbility()) {
                // 실제 게임에서는 여기서 플레이어의 선택을 받아야 함
                cout << player->getName() << "(" << player->getRole() << ")의 차례입니다.\n";
            }
        }
        
        dayCount++;
    }
    
private:
    void checkDeaths() {
        cout << "\n사망자 확인:\n";
        for (const auto& player : players) {
            if (!player->checkAlive()) {
                cout << player->getName() << "(" << player->getRole() << ") 이(가) 사망했습니다.\n";
            }
        }
    }
    
    void conductVoting() {
        cout << "\n투표를 진행합니다.\n";
        // 투표 로직 구현
        // 각 플레이어의 투표 권한 확인
        // 정치인의 투표 가중치 적용
        // 가장 많은 표를 받은 플레이어 처형
    }
    
    bool checkGameEnd() {
        int mafiaCount = 0;
        int citizenCount = 0;
        
        for (const auto& player : players) {
            if (player->checkAlive()) {
                if (player->getRole() == "Mafia") {
                    mafiaCount++;
                } else {
                    citizenCount++;
                }
            }
        }
        
        if (mafiaCount == 0) {
            cout << "시민 팀이 승리했습니다!\n";
            return true;
        } else if (mafiaCount >= citizenCount) {
            cout << "마피아 팀이 승리했습니다!\n";
            return true;
        }
        
        return false;
    }
};