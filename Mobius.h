#pragma once

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QList>
#include <QRandomGenerator>
#include <QKeyEvent>
#include <QPainter>

class MobiusGame : public QWidget
{
    Q_OBJECT

public:
    explicit MobiusGame(QWidget* parent = nullptr);
private:
    QPixmap backgroundPixmap;
    QColor backgroundColor;
protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    enum Direction { Up, Down, Left, Right };

    void initGame();
    void updateGame();
    void spawnFood();

    QTimer* timer;
    QList<QPoint> snake;
    Direction dir;
    QPoint food;
    bool isGameOver;
    int score;

    static const int cellSize = 15;
    static const int gridWidth = 40;
    static const int gridHeight = 30;
};
