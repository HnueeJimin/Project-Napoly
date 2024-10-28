#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Player {
protected:
    string name;
    bool isAlive;
public:
    Player(string n) : name(n), isAlive(true) {}

    virtual void setName(string n) { name = n; } // 이름 설정
    string getName() const { return name; }

    void kill() { isAlive = false; } 
    bool checkAlive() const { return isAlive; } // 생존 여부

    virtual void action(Player &target) = 0; // 직업마다 능력 구현 가상 함수
};

class Mafia : public Player {
public:
    Mafia(string n) : Player(n) {}
    void action(Player& target) override {
        if (target.checkAlive()) {
            target.kill();
            cout << target.getName() << " (이)가 총을 맞고 사망했습니다.\n";
        }
    }
};

class Police : Player {
public:
    Police(string n) : Player(n) {}

    void action(Player& target) override {
        cout << target.getName() << " (은)는 " << (dynamic_cast<Mafia*>(&target) ? "마피아 입니다." : "마피아가 아닙니다.") << ".\n";
    }
};

class Doctor : public Player {
private:
    Player* protectedTarget = nullptr; // 보호할 대상
public:
    Doctor(string n) : Player(n) {}

    void protect(Player& target) {
        protectedTarget = &target;
        cout << name << " (Doctor) will protect " << target.getName() << " tonight.\n";
    }

    void action(Player& target) override {
        cout << name << " (Doctor) is ready to protect " << target.getName() << ".\n";
    }

    void healIfAttacked(Player& target) {
        if (protectedTarget == &target && !target.checkAlive()) {
            target.kill(false); // 공격을 무효화 (생존 상태 유지)
            cout << "의사가 " << target.getName() << " (을)를 치료했습니다.\n";
        }
    }
};