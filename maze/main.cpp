#include "DxLib.h"
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <ctime>
#include <algorithm>

// マスの設定
const int MAZE_SIZE = 21;      // 迷路のサイズ（奇数にしてください）
const int CELL_MAZE_SIZE = 30; // 1マスの大きさ
enum CellType { WALL, PATH, SEARCHING, ROUTE, START, GOAL };
int mapData[MAZE_SIZE][MAZE_SIZE];

// 座標を管理する構造体
struct Pos {
    int x, y;
    bool operator<(const Pos& o) const { return x < o.x || (x == o.x && y < o.y); }
    bool operator==(const Pos& o) const { return x == o.x && y == o.y; }
};

// A*探索用のデータ
struct Node {
    Pos pos;
    int g;    // スタートからの歩数
    double f; // スコア（g + ゴールまでの直線距離）
    bool operator>(const Node& o) const { return f > o.f; }
};

// 1. 簡易迷路生成（棒倒し法）
void generateMaze() {
    for (int y = 0; y < MAZE_SIZE; y++) {
        for (int x = 0; x < MAZE_SIZE; x++) {
            if (x == 0 || x == MAZE_SIZE - 1 || y == 0 || y == MAZE_SIZE - 1) mapData[y][x] = WALL;
            else mapData[y][x] = PATH;
        }
    }
    srand((unsigned int)time(NULL));
    for (int y = 2; y < MAZE_SIZE - 2; y += 2) {
        for (int x = 2; x < MAZE_SIZE - 2; x += 2) {
            mapData[y][x] = WALL; // 柱を立てる
            int dx[] = { 1, -1, 0, 0 }, dy[] = { 0, 0, 1, -1 };
            int dir = rand() % 4;
            mapData[y + dy[dir]][x + dx[dir]] = WALL; // 倒す
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ChangeWindowMode(TRUE);
    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    generateMaze();
    Pos start = { 1, 1 }, goal = { MAZE_SIZE - 7, MAZE_SIZE - 7};
    mapData[start.y][start.x] = START;
    mapData[goal.y][goal.x] = GOAL;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList;
    std::map<Pos, Pos> parentMap;
    openList.push({ start, 0, 0.0 });
    bool goalReached = false;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
        ClearDrawScreen();

        // --- 探索処理（1フレームに1ステップ進む） ---
        if (!openList.empty() && !goalReached) {
            Node cur = openList.top(); openList.pop();

            if (cur.pos == goal) {
                goalReached = true;
                Pos r = goal;
                while (!(r == start)) { // 最短経路を赤く塗る
                    if (!(r == goal)) mapData[r.y][r.x] = ROUTE;
                    r = parentMap[r];
                }
            }
            else {
                if (!(cur.pos == start)) mapData[cur.pos.y][cur.pos.x] = SEARCHING;
                int dx[] = { 1, -1, 0, 0 }, dy[] = { 0, 0, 1, -1 };
                for (int i = 0; i < 4; i++) {
                    Pos next = { cur.pos.x + dx[i], cur.pos.y + dy[i] };
                    if (mapData[next.y][next.x] == PATH || mapData[next.y][next.x] == GOAL) {
                        if (parentMap.find(next) == parentMap.end()) {
                            parentMap[next] = cur.pos;
                            double h = sqrt(pow(next.x - goal.x, 2) + pow(next.y - goal.y, 2));
                            openList.push({ next, cur.g + 1, (double)cur.g + 1 + h });
                        }
                    }
                }
            }
        }

        // --- 描画処理 ---
        for (int y = 0; y < MAZE_SIZE; y++) {
            for (int x = 0; x < MAZE_SIZE; x++) {
                unsigned int c = GetColor(60, 60, 60); // 壁
                if (mapData[y][x] == PATH)      c = GetColor(255, 255, 255);
                if (mapData[y][x] == SEARCHING) c = GetColor(0, 200, 255);
                if (mapData[y][x] == ROUTE)     c = GetColor(255, 0, 0);
                if (mapData[y][x] == START)     c = GetColor(255, 255, 0);
                if (mapData[y][x] == GOAL)      c = GetColor(255, 0, 255);
                DrawBox(x * CELL_MAZE_SIZE, y * CELL_MAZE_SIZE, (x + 1) * CELL_MAZE_SIZE, (y + 1) * CELL_MAZE_SIZE, c, TRUE);
            }
        }
        ScreenFlip();
        Sleep(30); // 探索の様子が見えるように少し待つ
    }
    DxLib_End();
    return 0;
}