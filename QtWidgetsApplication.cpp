#include "Mobius.h"
#include "QtWidgetsApplication.h"
QtWidgetsApplication::QtWidgetsApplication(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    MobiusGame* game = new MobiusGame(this);
    setCentralWidget(game);
}
QtWidgetsApplication::~QtWidgetsApplication()
{
}
MobiusGame::MobiusGame(QWidget* parent) : QWidget(parent) {//传递参数。
    setFixedSize(gridWidth * cellSize, gridHeight * cellSize);//固定地图。
    setFocusPolicy(Qt::StrongFocus);//保证键盘输入可以收到。
    backgroundPixmap = QPixmap(":/images/background.png"); 
    if (backgroundPixmap.isNull()) {
        backgroundColor = QColor(30, 30, 30);
    }
    timer = new QTimer(this);//计时器。
    connect(timer, &QTimer::timeout, this, &MobiusGame::updateGame);//Qt通信方式？采用计时器来确定梅比乌斯多久动一次。
    initGame();
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
    timer->start(150); 
}
void MobiusGame::updateGame() {//键盘控制梅比乌斯的移动。
    if (isGameOver) return;
    QPoint newHead = snake.first();
    switch (dir) {
    case Up:    newHead.ry()--; break;
    case Down:  newHead.ry()++; break;
    case Left:  newHead.rx()--; break;
    case Right: newHead.rx()++; break;
    }
    if (newHead.x() < 0 || newHead.x() >= gridWidth ||
        newHead.y() < 0 || newHead.y() >= gridHeight) {
        isGameOver = true;
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
            isGameOver = true;
            timer->stop();
            break;
        }
    }
    update();
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
    case Qt::Key_Space:
        if (isGameOver) {
            initGame();   // 重开游戏。
            timer->start();
        }
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
    if (isGameOver) {//弹出游戏结束界面。
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 20, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "Game Over\nPress Space to Restart");
    }
    painter.setPen(Qt::yellow);//得分。
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 20, QString("Score: %1").arg(score));
}