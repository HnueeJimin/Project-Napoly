// jobs.h 1.1.2 ver

#ifndef JOBS_H
#define JOBS_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <random>

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
    
    bool defendShot() {
        if(armorActive) {
            armorActive = false;
            cout << "총격으로 인한 처치를 막아냈습니다!\n";
            return true;
        }
        return false;
    }

    string getRole() const override { return "군인"; }
};

class Citizen : public Player {
public:
    Citizen(string n) : Player(n) {}

    void action(Player&) override {}
    string getRole() const override { return "시민"; }
};

#endif // JOBS_H