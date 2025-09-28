#pragma once
#include <QMainWindow>

class QPushButton;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QPushButton* loadButton_;
    QWidget* canvas_;
};
