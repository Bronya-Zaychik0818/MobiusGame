#include <QSoundEffect>
#include "Mobius.h"
#include "QtWidgetsApplication.h"
#include <filesystem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
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
    m_backgroundMusic = new QSoundEffect(this);//背景音乐。
    m_backgroundMusic->setSource(QUrl::fromLocalFile("./bgm.wav"));
    m_backgroundMusic->setLoopCount(QSoundEffect::Infinite);
    m_backgroundMusic->setVolume(0.5);
    QPixmap backgroundPixmap("./Mobius.jpg");
    timer = new QTimer(this);//计时器。
    connect(timer, &QTimer::timeout, this, &MobiusGame::updateGame);//Qt通信方式？采用计时器来确定梅比乌斯多久动一次。
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &MobiusGame::updateCountdown);
    stickerGreenWin = QPixmap("./green_win.jpg");
    stickerYellowWin = QPixmap("./yellow_win.jpg");
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
void MobiusGame::startGame()
{
    initGame();
    gameState = GameState::Playing;
    timer->start(speedInterval);
    countdownTimer->start(1000);
    emit gameStarted();
    setFocus();
    update();
}
void MobiusGame::restartGame()//重开游戏。
{
    initGame();
    gameState = GameState::Playing;
    timer->start(speedInterval);
    countdownTimer->start(1000);
    emit gameStarted();
    setFocus();
    startBackgroundMusic();
    update();
}
void MobiusGame::startBackgroundMusic()//播放音乐。
{
    if (m_backgroundMusic && !m_backgroundMusic->isPlaying()) {
        m_backgroundMusic->play();
    }
}
void MobiusGame::stopBackgroundMusic()
{
    if (m_backgroundMusic) {
        m_backgroundMusic->stop();
    }
}
void MobiusGame::updateCountdown()
{
    if (gameState != GameState::Playing) return;
    timeLeft--;
    update();
    if (timeLeft <= 0) {
        gameState = GameState::GameOver;
        timer->stop();
        countdownTimer->stop();
        stopBackgroundMusic();
        emit gameOver();
        update();
    }
}
void MobiusGame::initGame() {//初始化梅比乌斯。
    startBackgroundMusic();
    snake.clear();
    int startX = gridWidth / 4;
    int startY = gridHeight / 2;//梅比乌斯在地图偏左的位置。
    snake.append(QPoint(startX, startY));
    snake.append(QPoint(startX - 1, startY));
    snake.append(QPoint(startX - 2, startY));
    dir = Right;
    isGameOver = false;
    snake2.clear();
    int startX2 = 3 * gridWidth / 4;     // 希希芙在地图偏右的位置。
    int startY2 = gridHeight / 2;
    snake2.append(QPoint(startX2, startY2));
    snake2.append(QPoint(startX2 + 1, startY2));
    snake2.append(QPoint(startX2 + 2, startY2));
    dir2 = Left;
    foods.clear();
    for (int i = 0; i < foodCount; ++i) {
        spawnOneFood();
    }
    obstacleCount = 10;
    spawnObstacles();
    timeLeft = 60;
}
void MobiusGame::spawnOneFood()//生成新食物。
{
    QPoint pos;
    int attempts = 0;
    do {
        pos.rx() = QRandomGenerator::global()->bounded(gridWidth);
        pos.ry() = QRandomGenerator::global()->bounded(gridHeight);
        attempts++;
        if (attempts > 1000) break; 
    } while (snake.contains(pos) || snake2.contains(pos) || foods.contains(pos) || obstacles.contains(pos));
    foods.append(pos);
}

void MobiusGame::updateGame() {//键盘控制梅比乌斯和希希芙的移动。
    qDebug() << "updateGame called";//测试。
    if (gameState != GameState::Playing) return;
    QPoint newHead = snake.first();
    switch (dir) {
    case Up:    newHead.ry()--; break;
    case Down:  newHead.ry()++; break;
    case Left:  newHead.rx()--; break;
    case Right: newHead.rx()++; break;
    }
    QPoint newHead2 = snake2.first();
    switch (dir2) {
    case Up:    newHead2.ry()--; break;
    case Down:  newHead2.ry()++; break;
    case Left:  newHead2.rx()--; break;
    case Right: newHead2.rx()++; break;
    }
    bool dead1 = false, dead2 = false;
    if (newHead.x() < 0 || newHead.x() >= gridWidth ||//判断梅比乌斯的死亡条件。
        newHead.y() < 0 || newHead.y() >= gridHeight) {
        dead1 = true;
    }
    if (!dead1 && obstacles.contains(newHead)) dead1 = true;
    if (!dead1) {
        for (int i = 0; i < snake.size(); i++) {
            if (newHead == snake[i]) { dead1 = true; break; }
        }
    }
    if (!dead1) {
        for (int i = 0; i < snake2.size(); i++) {
            if (newHead == snake2[i]) { dead1 = true; break; }
        }
    }
    if (newHead.x() < 0 || newHead.x() >= gridWidth ||
        newHead.y() < 0 || newHead.y() >= gridHeight) {
        dead1 = true;
    }
    if (newHead2.x() < 0 || newHead2.x() >= gridWidth ||//判断希希芙的死亡条件。
        newHead2.y() < 0 || newHead2.y() >= gridHeight) {
        dead2 = true;
    }
    if (!dead2 && obstacles.contains(newHead2)) dead2 = true;
    if (!dead2) {
        for (int i = 0; i < snake2.size(); i++) {
            if (newHead2 == snake2[i]) { dead2 = true; break; }
        }
    }
    if (!dead2) {
        for (int i = 0; i < snake.size(); i++) {
            if (newHead2 == snake[i]) { dead2 = true; break; }
        }
    }
    if (!dead1 && !dead2 && newHead == newHead2) {
        dead1 = dead2 = true;   // 两条蛇一起撞了。
    }
    if (dead1 || dead2) {
        if (dead1) respawnSnake1();
        if (dead2) respawnSnake2();
        update();   
        return;
    }
    int eatenIndex1 = -1;
    for (int i = 0; i < foods.size(); ++i) {
        if (newHead == foods[i]) {
            eatenIndex1 = i;
            break;
        }
    }
    int eatenIndex2 = -1;
    if (eatenIndex1 == -1) { 
        for (int i = 0; i < foods.size(); ++i) {
            if (newHead2 == foods[i]) {
                eatenIndex2 = i;
                break;
            }
        }
    }
    else {
        for (int i = 0; i < foods.size(); ++i) {
            if (i != eatenIndex1 && newHead2 == foods[i]) {
                eatenIndex2 = i;
                break;
            }
        }
    }
    if (eatenIndex1 != -1) {
        foods.removeAt(eatenIndex1);
        spawnOneFood();  
    }
    if (eatenIndex2 != -1) {
        foods.removeAt(eatenIndex2);
        spawnOneFood();
    }
    snake.prepend(newHead);
    if (eatenIndex1 == -1) snake.removeLast();

    snake2.prepend(newHead2);
    if (eatenIndex2 == -1) snake2.removeLast();
    update();
}
void MobiusGame::respawnSnake1()//梅比乌斯复活。
{
    snake.clear();
    int startX = gridWidth / 4;
    int startY = gridHeight / 2;
    snake.append(QPoint(startX, startY));
    snake.append(QPoint(startX - 1, startY));
    snake.append(QPoint(startX - 2, startY));
    dir = Right; 
    while (obstacles.contains(snake.first()) || snake2.contains(snake.first())) {//防止堵出生点。
        for (int i = 0; i < snake.size(); ++i) {
            snake[i].rx() += 1;
        }
    }
}

void MobiusGame::respawnSnake2()//希希芙复活。
{
    snake2.clear();
    int startX2 = 3 * gridWidth / 4;
    int startY2 = gridHeight / 2;
    snake2.append(QPoint(startX2, startY2));
    snake2.append(QPoint(startX2 + 1, startY2));
    snake2.append(QPoint(startX2 + 2, startY2));
    dir2 = Left; 
    while (obstacles.contains(snake.first()) || snake2.contains(snake.first())) {//防止堵出生点。
        for (int i = 0; i < snake.size(); ++i) {
            snake[i].rx() += 1;
        }
    }
}
void MobiusGame::spawnObstacles() {
    obstacles.clear();
    for (int i = 0; i < obstacleCount; ++i) {
        QPoint pos;
        int attempts = 0;
        do {
            pos.rx() = QRandomGenerator::global()->bounded(gridWidth);
            pos.ry() = QRandomGenerator::global()->bounded(gridHeight);
            ++attempts;
            if (attempts > 1000) break;
        } while (snake.contains(pos) || snake2.contains(pos) || foods.contains(pos) || obstacles.contains(pos));
        obstacles.append(pos);
    }
}
void MobiusGame::keyPressEvent(QKeyEvent* event) {//这里特意防止梅比乌斯原地掉头。
    if (gameState != GameState::Playing) {
        QWidget::keyPressEvent(event);
        return;
    }
    switch (event->key()) {
    case Qt::Key_W: 
        if (dir != Down) dir = Up;
        break;
    case Qt::Key_S: 
        if (dir != Up) dir = Down;
        break;
    case Qt::Key_A: 
        if (dir != Right) dir = Left;
        break;
    case Qt::Key_D: 
        if (dir != Left) dir = Right;
        break;
    }
    switch (event->key()) {
    case Qt::Key_Up:    
        if (dir2 != Down) dir2 = Up; 
        break;
    case Qt::Key_Down:  
        if (dir2 != Up) dir2 = Down;
        break;
    case Qt::Key_Left: 
        if (dir2 != Right) dir2 = Left; 
        break;
    case Qt::Key_Right: 
        if (dir2 != Left) dir2 = Right;
        break;
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
    painter.fillRect(rect(), QColor(0, 0, 0, 100));
    if (gameState == GameState::WaitingToStart) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 20, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("贪吃的两条蛇\n请在下方选择难度"));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("\n\n\n\n\n\n\nWASD 控制绿蛇梅比乌斯\n方向键 控制黄蛇希希芙"));
        return;
    }
    painter.setBrush(Qt::red);//画食物。
    painter.setPen(Qt::NoPen);
    for (const QPoint& f : foods) {
        painter.drawRect(f.x() * cellSize, f.y() * cellSize, cellSize, cellSize);
    }
    painter.setBrush(Qt::white);  // 画障碍物。
    painter.setPen(Qt::NoPen);
    for (const QPoint& obs : obstacles) {
        painter.drawRect(obs.x() * cellSize, obs.y() * cellSize, cellSize, cellSize);
    }
    for (int i = 0; i < snake.size(); ++i) {//画梅比乌斯。
        if (i == 0) {
            painter.setBrush(QColor(0, 200, 0));
        }
        else {
            painter.setBrush(Qt::green);
        }
        painter.drawRect(snake[i].x() * cellSize, snake[i].y() * cellSize, cellSize, cellSize);
    }
    for (int i = 0; i < snake2.size(); ++i) {//画希希芙。
        painter.setBrush(i == 0 ? QColor(255, 255, 0) :
            QColor(200, 200, 0)); 
        painter.drawRect(snake2[i].x() * cellSize, snake2[i].y() * cellSize, cellSize, cellSize);
    }
    if (gameState == GameState::GameOver) {
        painter.fillRect(rect(), QColor(0, 0, 0, 160));
        QPixmap stickerToDraw;
        if (snake.size() > snake2.size()) {
            stickerToDraw = stickerGreenWin;
        }
        else if (snake2.size() > snake.size()) {
            stickerToDraw = stickerYellowWin;
        }
        else {}
        if (!stickerToDraw.isNull()) {
            int stickerW = 200;
            int stickerH = 200;
            int x = (width() - stickerW) / 2;
            int y = height() / 20;
            painter.drawPixmap(x, y, stickerW, stickerH, stickerToDraw);
        }
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
        QString text = "\n\n\n\n\n\n\n时间到！\n";
        text += QString("梅比乌斯长度: %1  希希芙长度: %2\n").arg(snake.size()).arg(snake2.size());
        if (snake.size() > snake2.size()) text += "梅比乌斯获胜！";
        else if (snake2.size() > snake.size()) text += "希希芙获胜！";
        else text += "平局！";
        painter.drawText(rect(), Qt::AlignCenter, text);
    }
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 20, QString("Time: %1s").arg(timeLeft));
    painter.drawText(10, 40, QString("梅比乌斯: %1  希希芙: %2").arg(snake.size()).arg(snake2.size()));
    painter.setPen(Qt::yellow);//得分。
    painter.setFont(QFont("Arial", 12));
}