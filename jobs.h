// jobs.h

#ifndef JOBS_H
#define JOBS_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>

using namespace std;

class Player {
protected: // 상속받은 클래스에서 사용하기 위해 protected로 선언
    string name; // 이름
    bool isAlive; // 생존 여부
    bool canVote; // 투표 가능 여부
    bool canUseAbility; // 능력 사용 가능 여부

public:
    Player() : isAlive(true), canVote(true), canUseAbility(true) {} // 기본 생성자
    Player(string n) : name(n), isAlive(true), canVote(true), canUseAbility(true) {}
    virtual ~Player() {}

    void setName(string n) { name = n; } // 이름 설정
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
            cout << target.getName() << " (이)가 총을 맞고 '처치'됐습니다.\n";
        }
    }

    string getRole() const override { return "마피아"; }
};

class Spy : public Player { // 스파이
private:
    bool contactedMafia; // 마피아와 접촉했는지 여부 확인

public:
    Spy(string n) : Player(n), contactedMafia(false) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        cout << target.getName() << "의 직업은 " << target.getRole() << "입니다.\n";
        if (target.getRole() == "마피아") {
            contactedMafia = true;
            cout << "마피아와 접선했습니다!\n";
        }
    }

    string getRole() const override { return "스파이"; }
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
            cout << target.getName() << "이(가) 늑대인간에게 '살육'당했습니다.\n";
        }
    }

    void setTamed(bool isTamed) { tamed = isTamed; }
    bool isTamed() const { return tamed; }
    string getRole() const override { return "늑대인간"; }
};

class Madame : public Player { // 마담
private:
    bool contactedMafia;

public:
    Madame(string n) : Player(n), contactedMafia(false) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        target.setCanUseAbility(false);
        cout << target.getName() << "이(가) '유혹'당해 능력을 사용할 수 없게 되었습니다.\n";

        if (target.getRole() == "마피아") {
            contactedMafia = true;
            cout << "마피아와 접선했습니다!\n";
        }
    }

    string getRole() const override { return "마담"; }
};

class Thief : public Player { // 도둑
private:
    bool hasStolen;
    bool contactedMafia;
    string stolenRole;

public:
    Thief(string n) : Player(n), hasStolen(false), contactedMafia(false) {}

    void action(Player& target) override {
        if (!canUseAbility) return;

        if (&target == this && !hasStolen) {
            // 마피아 존재 여부는 외부에서 확인하도록 변경
            cout << "능력을 사용할 수 없습니다. 다른 플레이어를 선택하세요.\n";
        }
        else if (!hasStolen) {
            // 다른 플레이어의 능력 임시 획득
            stolenRole = target.getRole();
            cout << target.getName() << "의 능력을 훔쳤으며 직업은 " << target.getRole() << "입니다\n";
            hasStolen = true;

            if (target.getRole() == "마피아") {
                contactedMafia = true;
                cout << "마피아와 접선했습니다!\n";
            }
        }
    }

    // 마피아 존재 여부를 확인하고 능력을 부여하는 새로운 메소드
    void checkAndStealMafiaAbility(bool mafiaExists) {
        if (!mafiaExists && !hasStolen) {
            cout << "마피아 능력을 획득했습니다!\n";
            stolenRole = "마피아";
            hasStolen = true;
        }
    }

    string getRole() const override {
        return hasStolen ? stolenRole : "도둑";
    }

    bool hasContactedMafia() const {
        return contactedMafia;
    }
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
    string getRole() const override { return "과학자"; }
};

class Police : public Player { // 경찰
public:
    Police(string n) : Player(n) {}

    void action(Player& target) override {
        cout << target.getName() << " (은)는 " << (dynamic_cast<Mafia*>(&target) ? "마피아 입니다." : "마피아가 아닙니다.") << "\n";
    }

    string getRole() const override { return "경찰"; }
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

    bool isProtected(Player& target) const { // 보호 여부 확인 함수
        return protectedTarget == &target;
    }

    void healIfAttacked(Player& target) {
        if (protectedTarget == &target && !target.checkAlive()) {
            target.setAlive(true);
            cout << "의사가 " << target.getName() << " (을)를 치료했습니다.\n";
        }
    }

    string getRole() const override { return "의사"; }
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

    string getRole() const override { return "군인"; }
};

class Thug : public Player { // 건달
public:
    Thug(string n) : Player(n) {}

    void action(Player& target) override {
        if (!canUseAbility) return;
        target.setCanVote(false);
        cout << target.getName() << " (이)가 협박당해 투표할 수 없게 되었습니다.\n";
    }

    string getRole() const override { return "건달"; }
};

class Politician : public Player { // 정치인
public:
    Politician(string n) : Player(n) {}

    void action(Player& target) override {
        // 정치인은 능동적인 행동이 없음
    }

    int getVoteWeight() const { return 2; }

    string getRole() const override { return "정치인"; }
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

    string getRole() const override { return "기자"; }
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

    string getRole() const override { return "테러리스트"; }
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

    string getRole() const override { return "간호사"; }
};

class Mercenary : public Player { // 용병
private:
    Player* client;
    bool canKill;

public:
    Mercenary(string n) : Player(n), client(nullptr), canKill(false) {}

    void setClient(Player* p) {
        if (p && p->getRole() != "마피아") {
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
            cout << target.getName() << " (이)가 총을 맞고 사망했습니다.\n";
        }
    }

    string getRole() const override { return "용병"; }
};

class Caveman : public Player { // 도굴꾼
private:
    bool hasInherited;
    string inheritedRole;
    map<string, string> firstNightInfo;

public:
    Caveman(string n) : Player(n), hasInherited(false) {}

    void action(Player& target) override {
        // 첫째 날 밤에만 사용 가능
        if (!hasInherited) {
            inheritedRole = target.getRole();
            cout << target.getName() << "의 직업을 획득했습니다.\n";
            hasInherited = true;
        }
    }

    void addFirstNightInfo(const string& player, const string& info) {
        firstNightInfo[player] = info;
    }

    string getRole() const override { return hasInherited ? inheritedRole : "도굴꾼"; }
};

class Cleric : public Player {
private:
    bool hasResurrection;

public:
    Cleric(string n) : Player(n), hasResurrection(true) {}

    void action(Player& target) override {
        if (!canUseAbility || !hasResurrection) return;
        if (!target.checkAlive()) {
            target.setAlive(true);
            hasResurrection = false;
            cout << "성직자가 " << target.getName() << "을(를) 부활시켰습니다.\n";
        }
    }

    string getRole() const override { return "성직자"; }
};

#endif // JOBS_H