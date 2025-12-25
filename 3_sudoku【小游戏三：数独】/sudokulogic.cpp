#include "sudokulogic.h"
#include <random>
#include <algorithm>
#include <QDateTime>
#include <QStringList>


static bool SudokuLogic_canPlace(const SudokuLogic::Grid& grid, int r, int c, int v){
    for (int i=0;i<9;i++) if (grid[r][i]==v || grid[i][c]==v) return false;
    int br=r/3*3, bc=c/3*3; for (int i=0;i<3;i++) for (int j=0;j<3;j++) if (grid[br+i][bc+j]==v) return false; return true;
}

bool SudokuLogic::canPlace(const Grid& grid, int r, int c, int v){ return SudokuLogic_canPlace(grid,r,c,v); }

static bool genBacktrack(SudokuLogic::Grid& grid, std::mt19937& gen){
    int r=-1,c=-1; for (int i=0;i<9;i++) for (int j=0;j<9;j++) if (grid[i][j]==0){ r=i;c=j; goto found; }
found:
    if (r==-1) return true;
    std::array<int,9> vals{1,2,3,4,5,6,7,8,9}; std::shuffle(vals.begin(), vals.end(), gen);
    for (int k=0;k<9;k++){
        int v=vals[k]; if (!SudokuLogic_canPlace(grid,r,c,v)) continue; grid[r][c]=v; if (genBacktrack(grid,gen)) return true; grid[r][c]=0;
    }
    return false;
}

bool SudokuLogic::generateSolvedBacktrack(Grid& grid){
    auto seed = static_cast<unsigned long long>(QDateTime::currentMSecsSinceEpoch());
    std::mt19937 gen(static_cast<uint32_t>(seed));
    // start empty and fill by randomized backtracking
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) grid[r][c]=0;
    return genBacktrack(grid, gen);
}

SudokuLogic::Grid SudokuLogic::generateSolved() {
    Grid g{};
    generateSolvedBacktrack(g);
    // 通过数字置换进一步随机
    auto seed = static_cast<unsigned long long>(QDateTime::currentMSecsSinceEpoch());
    std::mt19937 gen(static_cast<uint32_t>(seed));
    std::vector<int> digits{1,2,3,4,5,6,7,8,9}; std::shuffle(digits.begin(), digits.end(), gen);
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) g[r][c] = digits[g[r][c]-1];
    // 美学校验，不通过则重新生成
    int attempts=0; const int maxAttempts=20;
    while (attempts<maxAttempts && !passesAesthetic(g)) { generateSolvedBacktrack(g); std::shuffle(digits.begin(), digits.end(), gen); for (int r=0;r<9;r++) for (int c=0;c<9;c++) g[r][c]=digits[g[r][c]-1]; attempts++; }
    return g;
}
bool SudokuLogic::passesAesthetic(const Grid& grid) {
    auto badLine = [](const std::array<int,9>& arr){
        // 检查长度≥3的严格递增或递减连续段
        int upLen = 1, downLen = 1;
        for (int i=1;i<9;i++) {
            if (arr[i] == 0 || arr[i-1] == 0) { upLen=1; downLen=1; continue; }
            if (arr[i] == arr[i-1] + 1) { upLen++; downLen=1; }
            else if (arr[i] == arr[i-1] - 1) { downLen++; upLen=1; }
            else { upLen=1; downLen=1; }
            if (upLen >= 3 || downLen >= 3) return true; // 存在不美观序列
        }
        return false;
    };
    // 行
    for (int r=0;r<9;r++) if (badLine(grid[r])) return false;
    // 列
    for (int c=0;c<9;c++) {
        std::array<int,9> col{}; for (int r=0;r<9;r++) col[r]=grid[r][c];
        if (badLine(col)) return false;
    }
    return true;
}

static bool solveOne(SudokuLogic::Grid& grid, int& count, int limit) {
    // find next empty
    int r = -1, c = -1;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (grid[i][j] == 0) { r = i; c = j; break; }
        }
        if (r != -1) break;
    }
    if (r == -1) { count++; return count >= limit; }
    for (int v = 1; v <= 9; ++v) {
        bool ok = true;
        for (int i = 0; i < 9 && ok; i++) {
            if (grid[r][i] == v || grid[i][c] == v) ok = false;
        }
        int br = r/3*3, bc = c/3*3;
        for (int i = 0; i < 3 && ok; i++) {
            for (int j = 0; j < 3; j++) {
                if (grid[br+i][bc+j] == v) { ok = false; break; }
            }
        }
        if (!ok) {
            continue;
        }
        grid[r][c] = v;
        if (solveOne(grid, count, limit)) return true;
        grid[r][c] = 0;
    }
    return false;
}

int SudokuLogic::countSolutions(Grid grid, int limit){ int cnt=0; solveOne(grid,cnt,limit); return cnt; }

static int boxIndex(int r, int c){ return (r/3)*3 + (c/3); }

void SudokuLogic::clueStats(const Grid& grid, int row[9], int col[9], int box[9]){
    for (int i=0;i<9;i++){ row[i]=0; col[i]=0; box[i]=0; }
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) if (grid[r][c]!=0){ row[r]++; col[c]++; box[boxIndex(r,c)]++; }
}

SudokuLogic::Grid SudokuLogic::makeUniquePuzzle(const Grid& solved, int clues, bool symmetric){
    clues = std::max(17, std::min(81, clues));
    auto seed = static_cast<unsigned long long>(QDateTime::currentMSecsSinceEpoch());
    std::mt19937 gen(static_cast<uint32_t>(seed));

    auto attemptOnce = [&](const Grid& solvedIn, bool sym, int minRow, int minCol, int minBox)->Grid{
        Grid puzzle = solvedIn;
        // 预生成移除顺序（成对对称或单点）
        struct CellPair{int r1,c1,r2,c2;};
        std::vector<CellPair> order;
        if (sym){
            for (int r=0;r<9;r++) for (int c=0;c<9;c++) {
                int r2=8-r, c2=8-c; if (r*9+c <= r2*9+c2) order.push_back({r,c,r2,c2});
            }
        } else {
            for (int r=0;r<9;r++) for (int c=0;c<9;c++) order.push_back({r,c,r,c});
        }
        std::shuffle(order.begin(), order.end(), gen);

        int row[9], col[9], box[9]; clueStats(puzzle,row,col,box);
        int currentClues = 81;
        auto tryRemove = [&](int r, int c){
            if (puzzle[r][c]==0) return false;
            int b = boxIndex(r,c);
            if (row[r]<=minRow || col[c]<=minCol || box[b]<=minBox) return false;
            int backup = puzzle[r][c]; puzzle[r][c]=0;
            Grid test = puzzle; int sols = countSolutions(test, 2);
            bool ok = (sols==1) && passesAesthetic(puzzle);
            if (ok){ row[r]--; col[c]--; box[b]--; currentClues--; return true; }
            puzzle[r][c]=backup; return false;
        };

        for (auto cp : order){
            if (currentClues <= clues) break;
            if (sym){
                bool same = (cp.r1==cp.r2 && cp.c1==cp.c2);
                if (same){ (void)tryRemove(cp.r1,cp.c1); }
                else{
                    int b1=boxIndex(cp.r1,cp.c1), b2=boxIndex(cp.r2,cp.c2);
                    if (puzzle[cp.r1][cp.c1]==0 || puzzle[cp.r2][cp.c2]==0) continue;
                    if (row[cp.r1]<=minRow || col[cp.c1]<=minCol || box[b1]<=minBox) continue;
                    if (row[cp.r2]<=minRow || col[cp.c2]<=minCol || box[b2]<=minBox) continue;
                    int v1=puzzle[cp.r1][cp.c1], v2=puzzle[cp.r2][cp.c2];
                    puzzle[cp.r1][cp.c1]=0; puzzle[cp.r2][cp.c2]=0;
                    Grid test = puzzle; int sols = countSolutions(test, 2);
                    bool ok = (sols==1) && passesAesthetic(puzzle);
                    if (ok){ row[cp.r1]--; col[cp.c1]--; box[b1]--; row[cp.r2]--; col[cp.c2]--; box[b2]--; currentClues-=2; }
                    else { puzzle[cp.r1][cp.c1]=v1; puzzle[cp.r2][cp.c2]=v2; }
                }
            } else {
                (void)tryRemove(cp.r1,cp.c1);
            }
        }

        if (currentClues > clues){
            std::vector<std::pair<int,int>> singles; for (int r=0;r<9;r++) for (int c=0;c<9;c++) if (puzzle[r][c]!=0) singles.emplace_back(r,c);
            std::shuffle(singles.begin(), singles.end(), gen);
            bool progress=true;
            while (progress && currentClues>clues){
                progress=false;
                for (auto rc : singles){
                    if (currentClues<=clues) break;
                    if (tryRemove(rc.first, rc.second)) progress=true;
                }
            }
        }
        return puzzle;
    };

    // 多轮尝试：优先对称且较高下限，必要时降低下限/取消对称，仍不达标则用新解盘重试
    Grid solvedCopy = solved;
    int attempts = 0; const int maxAttempts = 10;
    while (attempts < maxAttempts){
        Grid p1 = attemptOnce(solvedCopy, symmetric, 2, 2, 2);
        int row[9], col[9], box[9]; clueStats(p1,row,col,box);
        int cluesCount=0; for (int r=0;r<9;r++) for (int c=0;c<9;c++) if (p1[r][c]!=0) cluesCount++;
        if (cluesCount == clues) return p1;

        Grid p2 = attemptOnce(solvedCopy, symmetric, 1, 1, 1);
        cluesCount=0; for (int r=0;r<9;r++) for (int c=0;c<9;c++) if (p2[r][c]!=0) cluesCount++;
        if (cluesCount == clues) return p2;

        Grid p3 = attemptOnce(solvedCopy, false, 1, 1, 1);
        cluesCount=0; for (int r=0;r<9;r++) for (int c=0;c<9;c++) if (p3[r][c]!=0) cluesCount++;
        if (cluesCount == clues) return p3;

        // 重新生成完整解盘后再试
        generateSolvedBacktrack(solvedCopy);
        attempts++;
    }
    // 兜底：返回最后一次非对称结果（尽力靠近目标），仍保持唯一解
    Grid last = attemptOnce(solvedCopy, false, 1, 1, 1);
    return last;
}

bool SudokuLogic::isValidMove(const Grid& grid, int r, int c, int val) {
    if (val<1 || val>9) return false;
    for (int i=0;i<9;i++) if (grid[r][i]==val || grid[i][c]==val) return false;
    int br=r/3*3, bc=c/3*3;
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) if (grid[br+i][bc+j]==val) return false;
    return true;
}

bool SudokuLogic::isComplete(const Grid& grid) {
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) if (grid[r][c]==0) return false;
    return true;
}

QString SudokuLogic::toText(const Grid& grid) {
    QString out;
    for (int r=0;r<9;r++) {
        for (int c=0;c<9;c++) {
            out += QString::number(grid[r][c]);
            if (c<8) out += " ";
        }
        out += "\n";
    }
    return out;
}

SudokuLogic::Grid SudokuLogic::fromText(const QString& text) {
    Grid g{};
    QStringList lines = text.split('\n');
    for (int r=0; r<9 && r<lines.size(); r++) {
        QStringList parts = lines[r].split(' ', Qt::SkipEmptyParts);
        for (int c=0; c<9 && c<parts.size(); c++) g[r][c] = parts[c].toInt();
    }
    return g;
}
