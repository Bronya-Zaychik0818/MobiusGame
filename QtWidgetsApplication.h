#pragma once
#include <QMainWindow>
#include "ui_QtWidgetsApplication.h"   // 自动生成的ui头文件

class QtWidgetsApplication : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication(QWidget* parent = nullptr);
    ~QtWidgetsApplication();

private:
    Ui::QtWidgetsApplicationClass ui;
};
