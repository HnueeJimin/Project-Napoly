// function.h

void start() { // 시작 함수 구현

}

void player_add(int num) { // 플레이어 추가

}

void gamerule(){ // 게임 규칙 설명
    string line;
    ifstream file("mafiarule.txt");
    if(file.is_open()){
        while(getline(file, line)){
            cout << line << endl;
        }
        file.close();
    }else {
        cout << "Error to open" << endl;
    }
    
}

void call_player(){
    
}
