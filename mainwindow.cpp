#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "video_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout();

    QLineEdit *inputField = new QLineEdit(this);
    inputField->setPlaceholderText("请输入网络串流地址");

    QPushButton *confirmButton = new QPushButton("确认", this);

    layout->addWidget(inputField);
    layout->addWidget(confirmButton);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(confirmButton, &QPushButton::clicked, this, [this, inputField, layout]() {
        QString inputText = inputField->text();

        if (!inputText.isEmpty()) {
            // 移除输入框和确认按钮
            QWidget *inputWidget = layout->itemAt(0)->widget();
            QWidget *buttonWidget = layout->itemAt(1)->widget();
            layout->removeWidget(inputWidget);
            layout->removeWidget(buttonWidget);
            inputWidget->deleteLater();
            buttonWidget->deleteLater();

            VideoWidget *videoWidget = new VideoWidget(this, inputText); // 假设VideoWidget构造函数接受一个QString参数
            layout->addWidget(videoWidget);

            QPushButton *pauseButton = new QPushButton("暂停", this);
            connect(pauseButton, &QPushButton::clicked, videoWidget, &VideoWidget::pauseVideo);
            pauseButton->setStyleSheet(
                "QPushButton {"
                "    background-color: #4CAF50;" // 按钮背景色
                "    color: white;" // 按钮文本颜色
                "    border: none;" // 无边框
                "    border-radius: 10px;" // 圆角半径
                "    padding: 10px 20px;" // 内边距
                "    font-size: 16px;" // 字体大小
                "border-style:outset;"
                "}"
                "QPushButton:hover {"
                "    background-color:rgb(50, 155, 50);" // 鼠标悬停时的背景色
                "}"
                "QPushButton:pressed {"
                "    background-color:rgb(255, 128, 64);" // 按下时的背景色
                "}"
                );

            layout->addWidget(pauseButton);
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
