#include "Mobius.h"
#include "QtWidgetsApplication.h"
#include <filesystem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QMessageBox>
QtWidgetsApplication::QtWidgetsApplication(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    MobiusGame* game = new MobiusGame(this);
    auto* slowBtn = new QPushButton(QStringLiteral("慢速 (简单)"), this);//速度选择按钮
    auto* normalBtn = new QPushButton(QStringLiteral("普通 (中等)"), this);
    auto* fastBtn = new QPushButton(QStringLiteral("快速 (困难)"), this);
    auto* restartBtn = new QPushButton(QStringLiteral("重新开始"), this);
    layout->addWidget(game);
    layout->addWidget(slowBtn);
    layout->addWidget(normalBtn);
    layout->addWidget(fastBtn);
    layout->addWidget(restartBtn);
    setCentralWidget(central);
    slowBtn->setStyleSheet("background-color: #81C784; color: white; font-size: 16px; padding: 6px;");
    normalBtn->setStyleSheet("background-color: #4CAF50; color: white; font-size: 16px; padding: 6px;");
    fastBtn->setStyleSheet("background-color: #E53935; color: white; font-size: 16px; padding: 6px;");
    restartBtn->setStyleSheet("background-color: #f44336; color: white; font-size: 16px; padding: 6px;");
    restartBtn->setEnabled(false);
    slowBtn->setEnabled(true);
    normalBtn->setEnabled(true);
    fastBtn->setEnabled(true);
    connect(game, &MobiusGame::gameStarted, this, [=]() {
        slowBtn->setEnabled(false);
        normalBtn->setEnabled(false);
        fastBtn->setEnabled(false);
        restartBtn->setEnabled(false);
        });
    connect(game, &MobiusGame::gameOver, this, [=]() {
        slowBtn->setEnabled(true);
        normalBtn->setEnabled(true);
        fastBtn->setEnabled(true);
        restartBtn->setEnabled(true);
        game->setFocus();
        });
    connect(slowBtn, &QPushButton::clicked, game, &MobiusGame::startSlow);
    connect(normalBtn, &QPushButton::clicked, game, &MobiusGame::startNormal);
    connect(fastBtn, &QPushButton::clicked, game, &MobiusGame::startFast);
    connect(restartBtn, &QPushButton::clicked, game, &MobiusGame::restartGame);
}
QtWidgetsApplication::~QtWidgetsApplication()
{
}
MobiusGame::MobiusGame(QWidget* parent) : QWidget(parent) {//传递参数。
    setFixedSize(gridWidth * cellSize, gridHeight * cellSize);//固定地图。
    setFocusPolicy(Qt::StrongFocus);//保证键盘输入可以收到。
    backgroundPixmap = QPixmap("./Mobius.jpg");  
    if (backgroundPixmap.isNull()) {
        backgroundPixmap = QPixmap("../Mobius.jpg"); 
    }
    if (backgroundPixmap.isNull()) {
        backgroundColor = QColor(30, 30, 30);
    }
    QString path = QCoreApplication::applicationDirPath() + "/Mobius.jpg";
    backgroundPixmap = QPixmap(path);
    qDebug() << "图片加载状态:" << backgroundPixmap.isNull() << "路径:" << path;
    timer = new QTimer(this);//计时器。
    connect(timer, &QTimer::timeout, this, &MobiusGame::updateGame);//Qt通信方式？采用计时器来确定梅比乌斯多久动一次。
    timer->stop();
}
void MobiusGame::startSlow()
{setDifficultyAndStart(200);
}
void MobiusGame::startNormal()
{setDifficultyAndStart(150);
}
void MobiusGame::startFast()
{setDifficultyAndStart(100);
}
void MobiusGame::setDifficultyAndStart(int interval)//速度选择。
{
     qDebug() << "setDifficultyAndStart called with interval:" << interval;
        speedInterval = interval;
        startGame();
}
void MobiusGame::startGame()//开始游戏。
{
    initGame();
    gameState = GameState::Playing;
    timer->start(speedInterval);
    emit gameStarted();
    setFocus();  
    update();
    qDebug() << "startGame called";
}
void MobiusGame::restartGame()//重开游戏。
{
    initGame();
    gameState = GameState::Playing;
    timer->start(speedInterval);
    emit gameStarted();
    setFocus();
    update();
}
void MobiusGame::initGame() {//初始化梅比乌斯。
    snake.clear();
    int startX = gridWidth / 2;
    int startY = gridHeight / 2;//梅比乌斯在地图中间偏左的位置。
    snake.append(QPoint(startX, startY));
    snake.append(QPoint(startX - 1, startY));
    snake.append(QPoint(startX - 2, startY));
    dir = Right;
    isGameOver = false;
    score = 0;
    spawnFood();
}
void MobiusGame::updateGame() {//键盘控制梅比乌斯的移动。
    if (gameState != GameState::Playing) return;
    QPoint newHead = snake.first();
    switch (dir) {
    case Up:    newHead.ry()--; break;
    case Down:  newHead.ry()++; break;
    case Left:  newHead.rx()--; break;
    case Right: newHead.rx()++; break;
    }
    if (newHead.x() < 0 || newHead.x() >= gridWidth ||
        newHead.y() < 0 || newHead.y() >= gridHeight) {
        gameState = GameState::GameOver;
        timer->stop();
        update();
        return;
    }
    bool ateFood = (newHead == food);//检查梅比乌斯有没有撞到墙。
    snake.prepend(newHead);
    if (ateFood) {//检查吃饭。
        score++;
        spawnFood(); }
    else {
        snake.removeLast();
    }
    for (int i = 1; i < snake.size(); ++i) {//检查梅比乌斯有没有撞到自己。
        if (newHead == snake[i]) {
            gameState = GameState::GameOver;
            timer->stop();
            break;
        }
    }
    update();
    emit gameOver();
}
void MobiusGame::spawnFood() {
    QPoint pos;
    do {
        pos.rx() = QRandomGenerator::global()->bounded(gridWidth);
        pos.ry() = QRandomGenerator::global()->bounded(gridHeight);
    } while (snake.contains(pos)); // 食物不会生成在梅比乌斯身上。
    food = pos;
}
void MobiusGame::keyPressEvent(QKeyEvent* event) {//这里特意防止梅比乌斯原地掉头。
    if (gameState != GameState::Playing) {
        QWidget::keyPressEvent(event);
        return;
    }
    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_Up:
        if (dir != Down) dir = Up;
        break;
    case Qt::Key_S: case Qt::Key_Down:
        if (dir != Up) dir = Down;
        break;
    case Qt::Key_A: case Qt::Key_Left:
        if (dir != Right) dir = Left;
        break;
    case Qt::Key_D: case Qt::Key_Right:
        if (dir != Left) dir = Right;
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
void MobiusGame::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);//大背景。
    if (!backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), backgroundPixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    else {
        painter.fillRect(rect(), backgroundColor);
    }
    if (gameState == GameState::WaitingToStart) {
        painter.setPen(Qt::blue);
        painter.setFont(QFont("Microsoft YaHei", 20, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("贪吃的梅比乌斯\n请在下方选择难度"));
        return;
    }
    painter.fillRect(rect(), QColor(0, 0, 0, 100));
    painter.setBrush(Qt::red);//画梅比乌斯的食物。
    painter.setPen(Qt::NoPen);
    painter.drawRect(food.x() * cellSize, food.y() * cellSize, cellSize, cellSize);
    for (int i = 0; i < snake.size(); ++i) {//画梅比乌斯。
        if (i == 0) {
            painter.setBrush(QColor(0, 200, 0));
        }
        else {
            painter.setBrush(Qt::green);
        }
        painter.drawRect(snake[i].x() * cellSize, snake[i].y() * cellSize, cellSize, cellSize);
    }
    if (gameState == GameState::GameOver) {//弹出游戏结束界面。
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 20, QFont::Bold));
        painter.setPen(Qt::cyan);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("游戏结束"));
    }
    painter.setPen(Qt::yellow);//得分。
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 20, QString("Score: %1").arg(score));
}