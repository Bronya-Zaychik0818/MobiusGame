#pragma once

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QList>
#include <QRandomGenerator>
#include <QKeyEvent>
#include <QPainter>

class QSoundEffect;
class MobiusGame : public QWidget
{
    Q_OBJECT

public:
    explicit MobiusGame(QWidget* parent = nullptr);
public slots: 
    void startGame();
    void restartGame();
    void setDifficultyAndStart(int interval);
    void startSlow();
    void startNormal();
    void startFast();
signals:
    void gameStarted();
    void gameOver();

private:
    QPixmap backgroundPixmap;
    QColor backgroundColor;
protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    enum class GameState { WaitingToStart, Playing, GameOver };
    enum Direction { Up, Down, Left, Right };
    QList<QPoint> obstacles;
    int obstacleCount = 3;
    void spawnObstacles();
    void initGame();
    void updateGame();
    void spawnFood();
    void startBackgroundMusic();
    void stopBackgroundMusic();
    QSoundEffect* m_backgroundMusic;
    QTimer* timer;
    QList<QPoint> snake;
    Direction dir;
    QPoint food;
    bool isGameOver;
    GameState gameState;
    int score;
    int speedInterval = 150;
    static const int cellSize = 15;
    static const int gridWidth = 40;
    static const int gridHeight = 30;
};
