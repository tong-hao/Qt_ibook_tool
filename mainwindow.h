#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Book;
class Note;
class QPushButton;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onButtonClicked();
    void on_btn_refresh_clicked();
    void on_btn_markdown_clicked();

private:
    void scanBooks();
    void scanNotes();
    void initBookUi();
    QString getBookBody(const QString& assetID);

private:
    Ui::MainWindow *ui;

    std::map<QString, Book> books;
    std::map<QString, std::vector<Note>> notes;
    QPushButton* lastButton = nullptr;
    QPushButton* currentButton = nullptr;

};
#endif // MAINWINDOW_H
