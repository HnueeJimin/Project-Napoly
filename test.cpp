#include "jobs.h"

int main() {
    // 플레이어 생성
    Mafia mafia("플레이어 1");
    Doctor doctor("플레이어 2");
    Police police("플레이어 3");
    Spy spy("플레이어 4");

    // 기본 상호작용 테스트
    cout << "게임을 시작합니다!" << endl;

    // 경찰이 마피아의 정체를 조사
    police.action(mafia);

    // 마피아가 스파이를 공격
    mafia.action(spy);

    // 의사가 스파이를 보호
    doctor.action(spy);
    doctor.healIfAttacked(spy);

    // 스파이가 경찰의 정체를 확인
    spy.action(police);

    // 결과 출력
    cout << "스파이의 생존 상태: " << (spy.checkAlive() ? "생존" : "사망") << endl;
    cout << "스파이의 직업: " << spy.getRole() << endl;

    return 0;
}