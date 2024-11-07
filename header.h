#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Player {
protected:
    string name; // 이름
    bool isAlive; // 생존 여부
public:
    Player(string n) : name(n), isAlive(true) {} // 생성자

    virtual void setName(string n) { name = n; } // 이름 설정
    string getName() const { return name; }

    bool checkAlive() const { return isAlive; } // 생존 여부

    virtual void action(Player &target) = 0; // 직업마다 능력 구현 가상 함수
};

class Mafia : public Player { // 마피아
public:
    Mafia(string n) : Player(n) {}

    void action(Player& target) override {
        if (target.checkAlive()) {
            cout << target.getName() << " (이)가 총을 맞고 사망했습니다.\n";
        }
    }
};

class Spy : public Player { // 스파이

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

class Doctor : public Player { // 의사
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

            cout << "의사가 " << target.getName() << " (을)를 치료했습니다.\n";
        }
    }
};

class Soldier : public Player { // 군인

};

class Thug : public Player { // 건달

};

class Politician : public Player { // 정치인

};

class Reporter : public Player { // 기자

};

class Terrorist : public Player { // 테러리스트

};

class Nurse : public Player { // 간호사

};

class Mercenary : public Player { // 용병

}
