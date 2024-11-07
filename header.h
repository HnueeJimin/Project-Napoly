#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

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

};

class Madame : public Player { // 마담

};

class Scientist : public Player { // 과학자

};

class Police : Player { // 경찰
public:
    Police(string n) : Player(n) {}

    void action(Player& target) override {
        cout << target.getName() << " (은)는 " << (dynamic_cast<Mafia*>(&target) ? "마피아 입니다." : "마피아가 아닙니다.") << ".\n";
    }
};

class Doctor : public Player { // 의사 ddddddddddd
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

class Terrorist : public Player { // 테러리스트

};

class Nurse : public Player { // 간호사

};

class Mercenary : public Player { // 용병

};

class Caveman : public Player { // 도굴꾼

};

class Cleric : public Player { // 성직자

};
