#include "snake_ai.h"
#include "snake.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QMap>
#include <cmath>

SnakeAI::SnakeAI(snake* game) 
    : gameInstance(game), stuckCounter(0)
{
    // 初始化AI
}

SnakeAI::~SnakeAI()
{
}

int SnakeAI::getNextDirection()
{
    updateGameState();
    
    QPoint currentPos = rectToGridPoint(gameInstance->vSnakeHead);
    
    // 检查当前路径是否仍然有效且安全
    if (!currentPath.isEmpty()) {
        QPoint nextPos = currentPath.first();
        
        // 验证路径的前几步是否都安全
        bool pathSafe = true;
        QPoint checkPos = currentPos;
        int stepsToCheck = qMin(3, currentPath.size());
        
        for (int i = 0; i < stepsToCheck; ++i) {
            if (i < currentPath.size()) {
                checkPos = currentPath[i];
                if (!isPositionSafe(checkPos)) {
                    pathSafe = false;
                    break;
                }
            }
        }
        
        if (pathSafe && isPositionSafe(nextPos)) {
            currentPath.removeFirst();
            QPoint direction = nextPos - currentPos;
            
            // 额外的安全检查：确保下一步不是障碍
            if (!isBoomPosition(nextPos)) {
                // 转换为游戏方向
                if (direction == QPoint(0, -1)) return 1; // UP
                if (direction == QPoint(0, 1)) return 2;  // DOWN
                if (direction == QPoint(-1, 0)) return 3; // LEFT
                if (direction == QPoint(1, 0)) return 4;  // RIGHT
            } else {
                // 如果下一步是障碍，清空路径并重新规划
                currentPath.clear();
                stuckCounter++;
            }
        } else {
            // 当前路径不安全，清空并重新规划
            currentPath.clear();
            stuckCounter++;
        }
    }
    
    // 如果没有有效路径或路径失效，重新规划
    if (currentPath.isEmpty()) {
        PathTarget target = selectBestTarget();
        QPoint start = rectToGridPoint(gameInstance->vSnakeHead);
        
        // 检查是否陷入困境（多次重新规划）
        if (stuckCounter > 5) {
            // 使用保守策略：尝试到达尾部
            if (canReachTail()) {
                QPoint tailPos = rectToGridPoint(gameInstance->vSnakeRect.last());
                target = PathTarget(tailPos, 1000, "emergency_tail");
            }
            stuckCounter = 0; // 重置计数器
        }
        
        // 首先尝试A*算法
        currentPath = findPathAStar(start, target.position);
        
        // 如果A*失败，尝试BFS
        if (currentPath.isEmpty()) {
            currentPath = findPathBFS(start, target.position);
        }
        
        // 如果都失败了，尝试寻找临时安全位置
        if (currentPath.isEmpty()) {
            QPoint safestPos = findSafestNearbyPosition(start);
            if (safestPos != start) {
                currentPath = findPathBFS(start, safestPos);
            }
        }
        
        // 执行第一步
        if (!currentPath.isEmpty()) {
            QPoint nextPos = currentPath.first();
            
            // 额外的安全检查：确保下一步不是炸弹
            if (!isBoomPosition(nextPos)) {
                currentPath.removeFirst();
                QPoint direction = nextPos - start;
                
                if (direction == QPoint(0, -1)) return 1; // UP
                if (direction == QPoint(0, 1)) return 2;  // DOWN
                if (direction == QPoint(-1, 0)) return 3; // LEFT
                if (direction == QPoint(1, 0)) return 4;  // RIGHT
            } else {
                // 如果下一步是炸弹，清空路径
                currentPath.clear();
            }
        }
    } else {
        // 路径有效时重置陷入计数器
        stuckCounter = 0;
    }
    
    // 如果所有策略都失败，返回紧急方向
    return getEmergencyDirection();
}

void SnakeAI::updateGameState()
{
    // 略
}

QList<QPoint> SnakeAI::findPathAStar(QPoint start, QPoint goal)
{
    if (!isValidPosition(goal) || isOccupied(goal) || isBoomPosition(goal)) {
        return QList<QPoint>();
    }
    
    std::priority_queue<AStarNode> openList;
    QSet<QPoint> closedList;
    std::map<QPoint, AStarNode*> allNodes;
    
    // 使用改进的启发式函数
    int initialH = manhattanDistance(start, goal) + calculateSafetyScore(goal) / 10;
    AStarNode* startNode = new AStarNode(start, 0, initialH);
    openList.push(*startNode);
    allNodes[start] = startNode;
    
    while (!openList.empty()) {
        AStarNode current = openList.top();
        openList.pop();
        
        // 找到最短路径，返回列表并清理内存
        if (current.position == goal) {
            // 重建路径
            QList<QPoint> path;
            AStarNode* node = &current;
            while (node && node->position != start) {
                path.prepend(node->position);
                node = node->parent;
            }
            
            // 清理内存
            for (auto it = allNodes.begin(); it != allNodes.end(); ++it) {
                delete it->second;
            }
            
            return path;
        }
        
        closedList.insert(current.position);
        
        // 检查四个方向
        for (int i = 0; i < 4; ++i) {
            QPoint neighbor = current.position + directions[i];
            
            if (!isValidPosition(neighbor) || closedList.contains(neighbor)) {
                continue;
            }
            
            // 检查是否安全（除了目标点）
            if (neighbor != goal && !isPositionSafe(neighbor)) {
                continue;
            }
            
            // 改进的代价计算
            int gCost = current.gCost + 1;
            
            // 如果位置不够安全，增加额外代价
            if (neighbor != goal) {
                int safetyScore = calculateSafetyScore(neighbor);
                if (safetyScore < 20) {
                    gCost += 5; // 不安全的位置代价更高
                }
            }
            
            // 改进的启发式函数
            int hCost = manhattanDistance(neighbor, goal);
            
            // 考虑到目标的安全性
            if (neighbor != goal) {
                int targetSafety = calculateSafetyScore(goal);
                hCost += (50 - targetSafety) / 10; // 目标越不安全，启发式代价越高
            }
            
            if (allNodes.find(neighbor) == allNodes.end()) {
                AStarNode* newNode = new AStarNode(neighbor, gCost, hCost, allNodes[current.position]);
                allNodes[neighbor] = newNode;
                openList.push(*newNode);
            } else if (gCost < allNodes[neighbor]->gCost) {
                allNodes[neighbor]->gCost = gCost;
                allNodes[neighbor]->hCost = hCost;
                allNodes[neighbor]->parent = allNodes[current.position];
            }
        }
    }
    
    // 清理内存
    for (auto it = allNodes.begin(); it != allNodes.end(); ++it) {
        delete it->second;
    }
    
    return QList<QPoint>(); // 没找到路径
}

QList<QPoint> SnakeAI::findPathBFS(QPoint start, QPoint goal)
{
    if (!isValidPosition(goal) || isBoomPosition(goal)) {
        return QList<QPoint>();
    }
    
    QQueue<QPoint> queue;
    QSet<QPoint> visited;
    std::map<QPoint, QPoint> parent;
    
    queue.enqueue(start);
    visited.insert(start);
    
    while (!queue.isEmpty()) {
        QPoint current = queue.dequeue();
        
        if (current == goal) {
            // 重建路径
            QList<QPoint> path;
            QPoint node = goal;
            while (node != start) {
                path.prepend(node);
                node = parent[node];
            }
            return path;
        }
        
        // 检查四个方向
        for (int i = 0; i < 4; ++i) {
            QPoint neighbor = current + directions[i];
            
            if (!isValidPosition(neighbor) || visited.contains(neighbor)) {
                continue;
            }
            
            // 检查是否安全（除了目标点）
            if (neighbor != goal && !isPositionSafe(neighbor)) {
                continue;
            }
            
            visited.insert(neighbor);
            parent[neighbor] = current;
            queue.enqueue(neighbor);
        }
    }
    
    return QList<QPoint>(); // 没找到路径
}

bool SnakeAI::isPositionSafe(QPoint pos)
{
    // 检查是否在游戏区域内
    if (!isValidPosition(pos)) {
        return false;
    }
    
    // 检查是否被蛇身占用
    if (isOccupied(pos)) {
        return false;
    }
    
    // 检查是否是炸弹位置（使用统一函数）
    if (isBoomPosition(pos)) {
        return false;
    }
    
    // 使用统一的出路检查函数
    return countEscapeRoutes(pos) >= 2;
}

bool SnakeAI::willCreateDeadEnd(QPoint pos, int direction)
{
    // 检查下一步位置是否安全
    QPoint nextPos = pos + directions[direction - 1];
    return !isPositionSafe(nextPos);
}

int SnakeAI::calculateSafetyScore(QPoint pos)
{
    int score = 0;
    
    // 基础出路评分（使用统一函数）
    int escapeRoutes = countEscapeRoutes(pos);
    score += escapeRoutes * 10;
    
    // 距离边界的距离（越远越安全）
    int distToEdge = qMin(qMin(pos.x(), GRID_WIDTH - 1 - pos.x()),
                         qMin(pos.y(), GRID_HEIGHT - 1 - pos.y()));
    score += distToEdge * 3;
    
    // 开放空间奖励
    int openSpaceScore = 0;
    for (int radius = 1; radius <= 2; ++radius) {
        int openCount = 0;
        int totalCount = 0;
        
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                if (qAbs(dx) + qAbs(dy) != radius) continue;
                
                QPoint checkPos = pos + QPoint(dx, dy);
                if (isValidPosition(checkPos)) {
                    totalCount++;
                    if (!isOccupied(checkPos) && !isBoomPosition(checkPos)) {
                        openCount++;
                    }
                }
            }
        }
        
        if (totalCount > 0) {
            openSpaceScore += (openCount * (3 - radius) * 5) / totalCount;
        }
    }
    
    score += openSpaceScore;
    return score;
}

PathTarget SnakeAI::selectBestTarget()
{
    QList<PathTarget> targets = getAllTargets();
    
    if (targets.isEmpty()) {
        QPoint tailPos = rectToGridPoint(gameInstance->vSnakeRect.last());
        return PathTarget(tailPos, 10, "tail");
    }
    
    QPoint currentPos = rectToGridPoint(gameInstance->vSnakeHead);
    int snakeLength = gameInstance->vSnakeRect.size();
    bool isLongSnake = snakeLength > 8;
    
    // 统一的目标评估函数
    auto evaluateTarget = [&](const PathTarget& target) -> int {
        // 基础可达性检查
        QList<QPoint> pathToTarget = findPathAStar(currentPos, target.position);
        if (pathToTarget.isEmpty()) return -1000; // 不可达
        
        // 安全性检查（长蛇更严格）
        int escapeRoutes = countEscapeRoutes(target.position);
        int minRequiredRoutes = isLongSnake ? 2 : 1;
        
        if (escapeRoutes < minRequiredRoutes) {
            return -500; // 不够安全
        }
        
        // 路径安全性（长蛇检查更多节点）
        if (isLongSnake && pathToTarget.size() > 3) {
            int unsafeNodes = 0;
            int checkNodes = qMin(5, pathToTarget.size());
            for (int i = 0; i < checkNodes; ++i) {
                if (countEscapeRoutes(pathToTarget[i]) < 2) {
                    unsafeNodes++;
                }
            }
            if (unsafeNodes > checkNodes / 2) return -300; // 路径太危险
        }
        
        // 计算综合评分
        int distance = manhattanDistance(currentPos, target.position);
        int safetyScore = calculateSafetyScore(target.position);
        
        int score = target.priority - (distance * 2) + (safetyScore / 5);
        
        // 长蛇特殊奖励
        if (isLongSnake) {
            score += escapeRoutes * 15; // 出路奖励
            if (escapeRoutes >= 3) score += 20; // 额外安全奖励
        }
        
        // 特殊物品处理
        if (target.type == "special_food" && gameInstance->timeLimitCounter < 30) {
            score += isLongSnake ? (escapeRoutes >= 3 ? 30 : -20) : 50;
        }
        
        if ((target.type == "speed_up" || target.type == "slow_down") && isLongSnake) {
            score += escapeRoutes >= 3 ? 10 : -20;
        }
        
        return score;
    };
    
    // 评估所有目标并选择最佳
    PathTarget bestTarget = targets.first();
    int bestScore = evaluateTarget(bestTarget);
    
    for (const auto& target : targets) {
        int score = evaluateTarget(target);
        if (score > bestScore) {
            bestScore = score;
            bestTarget = target;
        }
    }
    
    // 如果没有合适目标，返回尾部
    if (bestScore < -200) {
        QPoint tailPos = rectToGridPoint(gameInstance->vSnakeRect.last());
        return PathTarget(tailPos, 10, "emergency_tail");
    }
    
    return bestTarget;
}

QList<PathTarget> SnakeAI::getAllTargets()
{
    QList<PathTarget> targets;
    
    // 特殊食物（限时食物）
    if (gameInstance->showTimeLimitFood) {
        QPoint pos = rectToGridPoint(gameInstance->TimeLimitFood);
        if (isValidPosition(pos) && !isBoomPosition(pos)) {
            targets.append(PathTarget(pos, 100, "special_food"));
        }
    }
    
    // 普通食物
    QPoint foodPos = rectToGridPoint(gameInstance->Food);
    if (isValidPosition(foodPos) && !isBoomPosition(foodPos)) {
        targets.append(PathTarget(foodPos, 50, "food"));
    }
    
    // 加速道具
    if (!gameInstance->SpeedUpItem.isNull()) {
        QPoint pos = rectToGridPoint(gameInstance->SpeedUpItem);
        if (isValidPosition(pos) && isPositionSafe(pos)) {
            targets.append(PathTarget(pos, 30, "speed_up"));
        }
    }
    
    // 减速道具
    if (!gameInstance->SlowDownItem.isNull()) {
        QPoint pos = rectToGridPoint(gameInstance->SlowDownItem);
        if (isValidPosition(pos) && isPositionSafe(pos)) {
            targets.append(PathTarget(pos, 20, "slow_down"));
        }
    }
    
    return targets;
}

QPoint SnakeAI::rectToGridPoint(const QRect& rect)
{
    int x = (rect.x() - GAME_AREA_X) / GRID_SIZE;
    int y = (rect.y() - GAME_AREA_Y) / GRID_SIZE;
    return QPoint(x, y);
}

QRect SnakeAI::gridPointToRect(const QPoint& point)
{
    int x = GAME_AREA_X + point.x() * GRID_SIZE;
    int y = GAME_AREA_Y + point.y() * GRID_SIZE;
    return QRect(x, y, GRID_SIZE, GRID_SIZE);
}

int SnakeAI::manhattanDistance(QPoint a, QPoint b)
{
    return qAbs(a.x() - b.x()) + qAbs(a.y() - b.y());
}

bool SnakeAI::isValidPosition(QPoint pos)
{
    return pos.x() >= 0 && pos.x() < GRID_WIDTH && 
           pos.y() >= 0 && pos.y() < GRID_HEIGHT;
}

bool SnakeAI::isOccupied(QPoint pos)
{
    QRect posRect = gridPointToRect(pos);
    
    // 检查蛇身
    for (const QRect& segment : gameInstance->vSnakeRect) {
        if (segment == posRect) {
            return true;
        }
    }
    
    return false;
}

int SnakeAI::getEmergencyDirection()
{
    QPoint currentPos = rectToGridPoint(gameInstance->vSnakeHead);
    int currentDir = gameInstance->currentDirection;
    
    // 尝试每个方向，选择最安全的
    int bestDirection = currentDir;
    int bestScore = -1000;
    
    for (int dir = 1; dir <= 4; ++dir) {
        // 避免反向移动
        if ((currentDir == 1 && dir == 2) || (currentDir == 2 && dir == 1) ||
            (currentDir == 3 && dir == 4) || (currentDir == 4 && dir == 3)) {
            continue;
        }
        
        QPoint nextPos = currentPos + directions[dir - 1];
        
        if (!isValidPosition(nextPos) || isOccupied(nextPos) || isBoomPosition(nextPos)) {
            continue;
        }
        
        int score = 0;
        
        // 基础安全性评分
        score += calculateSafetyScore(nextPos);
        
        // 检查是否会创造死路
        if (willCreateDeadEnd(currentPos, dir)) {
            score -= 100;
        }
        
        // 检查能否到达尾部（重要的生存策略）
        QPoint tailPos = rectToGridPoint(gameInstance->vSnakeRect.last());
        QList<QPoint> pathToTail = findPathBFS(nextPos, tailPos);
        if (!pathToTail.isEmpty()) {
            score += 50; // 能到达尾部是好事
        } else {
            score -= 50; // 不能到达尾部很危险
        }
        
        // 检查前进几步的安全性（前瞻性）
        QPoint futurePos = nextPos;
        for (int step = 0; step < 3; ++step) {
            futurePos = futurePos + directions[dir - 1];
            if (isValidPosition(futurePos) && !isOccupied(futurePos) && !isBoomPosition(futurePos)) {
                score += 5; // 前方越空旷越好
            } else {
                break;
            }
        }
        
        // 偏向于保持当前方向（避免频繁转向）
        if (dir == currentDir) {
            score += 10;
        }
        
        // 避免靠近边界
        int distToEdge = qMin(qMin(nextPos.x(), GRID_WIDTH - 1 - nextPos.x()),
                             qMin(nextPos.y(), GRID_HEIGHT - 1 - nextPos.y()));
        score += distToEdge * 3;
        
        if (score > bestScore) {
            bestScore = score;
            bestDirection = dir;
        }
    }
    
    // 如果所有方向都很危险，至少选择一个不会立即死亡的方向
    if (bestScore < -500) {
        for (int dir = 1; dir <= 4; ++dir) {
            if ((currentDir == 1 && dir == 2) || (currentDir == 2 && dir == 1) ||
                (currentDir == 3 && dir == 4) || (currentDir == 4 && dir == 3)) {
                continue;
            }
            
            QPoint nextPos = currentPos + directions[dir - 1];
            if (isValidPosition(nextPos) && !isOccupied(nextPos) && !isBoomPosition(nextPos)) {
                return dir; // 至少不会立即死亡
            }
        }
    }
    
    return bestDirection;
}

bool SnakeAI::canReachTail()
{
    if (gameInstance->vSnakeRect.isEmpty()) return false;
    
    QPoint headPos = rectToGridPoint(gameInstance->vSnakeHead);
    QPoint tailPos = rectToGridPoint(gameInstance->vSnakeRect.last());
    
    // 使用BFS检查是否能到达尾部
    QList<QPoint> path = findPathBFS(headPos, tailPos);
    return !path.isEmpty();
}

QPoint SnakeAI::findSafestNearbyPosition(QPoint start)
{
    QPoint safestPos = start;
    int bestScore = -1000;
    
    // 在附近区域搜索最安全的位置
    for (int radius = 1; radius <= 5; ++radius) {
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                if (qAbs(dx) != radius && qAbs(dy) != radius) continue; // 只检查边界
                
                QPoint pos = start + QPoint(dx, dy);
                if (!isValidPosition(pos) || isOccupied(pos) || isBoomPosition(pos)) continue;
                
                int score = calculateSafetyScore(pos);
                
                // 距离起点越近越好（在安全的前提下）
                int distance = manhattanDistance(start, pos);
                score -= distance * 5;
                
                // 检查能否从这个位置到达尾部
                QPoint tailPos = rectToGridPoint(gameInstance->vSnakeRect.last());
                QList<QPoint> pathToTail = findPathBFS(pos, tailPos);
                if (!pathToTail.isEmpty()) {
                    score += 100;
                }
                
                if (score > bestScore) {
                    bestScore = score;
                    safestPos = pos;
                }
            }
        }
        
        // 如果在当前半径找到了足够安全的位置，就停止搜索
        if (bestScore > 50) break;
    }
    
    return safestPos;
}

int SnakeAI::countEscapeRoutes(QPoint pos)
{
    int escapeCount = 0;
    
    // 检查四个方向的出路
    for (const QPoint& dir : directions) {
        QPoint nextPos = pos + dir;
        if (isValidPosition(nextPos) && !isOccupied(nextPos) && !isBoomPosition(nextPos)) {
            escapeCount++;
        }
    }
    
    return escapeCount;
}

bool SnakeAI::isBoomPosition(QPoint pos)
{
    QRect posRect = gridPointToRect(pos);
    for (const QRect& boom : gameInstance->Booms) {
        if (posRect == boom) {
            return true;
        }
    }
    return false;
}
