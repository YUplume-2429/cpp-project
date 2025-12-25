#include "menu.h"

Menu::Menu() {
    m_options = { QString::fromUtf8("开始游戏"), QString::fromUtf8("退出") };
}

void Menu::printText(QPainter& painter, const QFont& font, int x, int y, const QString& text, const QColor& color) {
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText(x, y, text);
}

void Menu::displayMenu(QPainter& painter, const QRect& area) {
    QFont font(m_fontFamily, 24); // 对应 Python 48 号，这里按像素需求可调整
    int startX = area.width() / 2 - 120;
    int startY = 200;
    int lineStep = 60;
    for (int i = 0; i < m_options.size(); ++i) {
        QColor color = (i == m_selectedOption) ? QColor(255, 255, 0) : QColor(255, 255, 255);
        printText(painter, font, startX, startY + i * lineStep, m_options[i], color);
    }
}

void Menu::navigate(const QString& direction) {
    if (direction == QString::fromUtf8("down")) {
        m_selectedOption = (m_selectedOption + 1) % m_options.size();
    } else if (direction == QString::fromUtf8("up")) {
        m_selectedOption = (m_selectedOption - 1 + m_options.size()) % m_options.size();
    }
}

QString Menu::selectOption() const {
    if (m_selectedOption == 0) return QString::fromUtf8("start_game");
    if (m_selectedOption == 1) return QString::fromUtf8("exit");
    return QString();
}
