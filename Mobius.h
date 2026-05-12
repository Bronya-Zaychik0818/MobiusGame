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
    QPixmap stickerGreenWin;
    QPixmap stickerYellowWin;
    QList<QPoint> snake2;
    enum Direction { Up, Down, Left, Right };
    Direction dir2;
    int score2;
    enum class GameState { WaitingToStart, Playing, GameOver };
    QList<QPoint> obstacles;
    QList<QPoint> foods;
    int foodCount = 5;
    int obstacleCount = 3;
    void spawnObstacles();
    void initGame();
    void updateGame();
    void startBackgroundMusic();
    void stopBackgroundMusic();
    QTimer* countdownTimer;
    int timeLeft;
    QSoundEffect* m_backgroundMusic;
    QTimer* timer;
    QList<QPoint> snake;
    Direction dir;
    void respawnSnake1();
    void respawnSnake2();
    void spawnOneFood(); 
    void updateCountdown();
    bool isGameOver;
    GameState gameState;
    int score;
    int speedInterval = 150;
    static const int cellSize = 15;
    static const int gridWidth = 40;
    static const int gridHeight = 30;
};
