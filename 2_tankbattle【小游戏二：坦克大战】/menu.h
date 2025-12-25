#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QFont>
#include <QPainter>

class Menu {
public:
    Menu();

    void displayMenu(QPainter& painter, const QRect& area);
    void navigate(const QString& direction);
    QString selectOption() const;

    int selectedIndex() const { return m_selectedOption; }
    void setFontFamily(const QString& family) { m_fontFamily = family; }

private:
    void printText(QPainter& painter, const QFont& font, int x, int y, const QString& text, const QColor& color = QColor(255,255,255));

private:
    QVector<QString> m_options;
    int m_selectedOption{0};
    QString m_fontFamily{QString::fromUtf8("SimHei")};
};

#endif // MENU_H
