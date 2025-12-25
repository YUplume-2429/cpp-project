#ifndef SUDOKULOGIC_H
#define SUDOKULOGIC_H

#include <array>
#include <vector>
#include <QString>

class SudokuLogic {
public:
    using Grid = std::array<std::array<int,9>,9>;

    // 生成完整解盘：随机回溯求解，确保充分随机性
    static Grid generateSolved();
    // 生成唯一解题目：根据期望线索数（clues），从解盘移除数字并保持唯一解
    // 生成唯一解题面：
    // - 保证唯一解（解计数==1），线索数精确等于传入值
    // - 支持对称（美观），并进行美学校验（避免行/列≥3的严格单调序列）
    static Grid makeUniquePuzzle(const Grid& solved, int clues, bool symmetric=true);
    static bool isValidMove(const Grid& grid, int r, int c, int val);
    static bool isComplete(const Grid& grid);
    // 解计数：返回解的数量（超过2可提前剪枝）
    static int countSolutions(Grid grid, int limit=2);

    // 美学校验：避免任意行或列中存在长度≥3的严格递增或递减连续序列
    static bool passesAesthetic(const Grid& grid);

    static QString toText(const Grid& grid);
    static Grid fromText(const QString& text);
private:
    static bool canPlace(const Grid& grid, int r, int c, int v);
    static bool generateSolvedBacktrack(Grid& grid);
    static void clueStats(const Grid& grid, int row[9], int col[9], int box[9]);
};

#endif // SUDOKULOGIC_H