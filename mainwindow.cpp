#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QIcon>


#include "utils.h"


QString dir_ibook_db =
    "~/Library/Containers/com.apple.iBooksX/Data/Documents/BKLibrary";
QString dir_notes_db =
    "~/Library/Containers/com.apple.iBooksX/Data/Documents/AEAnnotation";

// sql
std::string sql_list_book = R"(
SELECT ZASSETID, ZTITLE AS Title, ZAUTHOR AS Author
FROM ZBKLIBRARYASSET
WHERE ZTITLE IS NOT NULL
)";

std::string sql_export_node = R"(
SELECT
ZANNOTATIONREPRESENTATIVETEXT as BroaderText,
ZANNOTATIONSELECTEDTEXT as SelectedText,
ZANNOTATIONNOTE as Note,
ZFUTUREPROOFING5 as Chapter,
ZANNOTATIONCREATIONDATE as Created,
ZANNOTATIONMODIFICATIONDATE as Modified,
ZANNOTATIONASSETID,
ZANNOTATIONLOCATION as LOCATION
FROM ZAEANNOTATION
WHERE ZANNOTATIONSELECTEDTEXT IS NOT NULL and ZANNOTATIONDELETED=0
)";


void MainWindow::scanBooks() {
    books.clear();
    auto fBookDb =  findDatabaseFile(dir_ibook_db.toStdString());
    if (!fBookDb) {
        qDebug() << "Cannot get database file for ibook in " << dir_ibook_db;
    }

    QSqlDatabase db= QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fBookDb.value().c_str());
    if(!db.open()) {
        qDebug() << "cannot open db.";
    } else {
        qDebug() << "open db success.";
    }

    QSqlQuery query;
    query.prepare(sql_list_book.c_str());
    query.exec();

    while(query.next()) {
        QSqlRecord r = query.record();
        QString assetID = r.field(0).value().toString();
        QString title = r.field(1).value().toString();
        QString author = r.field(2).value().toString();

        books.emplace(assetID, Book{title, author});

    }
    db.close();
}

void MainWindow::scanNotes() {
    notes.clear();
    auto fBookDb =  findDatabaseFile(dir_notes_db.toStdString());
    if (!fBookDb) {
        qDebug() << "Cannot get database file for ibook in " << dir_notes_db;
    }

    QSqlDatabase db= QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fBookDb.value().c_str());
    if(!db.open()) {
        qDebug() << "cannot open db.";
    } else {
        qDebug() << "open db success.";
    }

    QSqlQuery query;
    query.prepare(sql_export_node.c_str());
    query.exec();

    while(query.next()) {
        QSqlRecord r = query.record();
        QString broaderText = r.field(0).value().toString();
        QString selectedText = r.field(1).value().toString();
        QString markNote = r.field(2).value().toString();
        QString chapter = r.field(3).value().toString();
        QString created = r.field(4).value().toString();
        QString modified = r.field(5).value().toString();
        QString assetID = r.field(6).value().toString();
        QString location = r.field(7).value().toString();

        Note n;
        n.BroaderText = broaderText;
        n.SelectedText = selectedText;
        n.MarkNote = markNote;
        n.Chapter = chapter;
        n.Created = created;
        n.Modified = modified;
        n.Location = location;

        notes[assetID].push_back(n);
    }

    // sort notes
    for (auto &[assetID, noteVec] : notes) {
        qDebug() << assetID << ", count:" << noteVec.size();
        std::sort(noteVec.begin(), noteVec.end(), compareByLocation);
    }

    db.close();

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->textEdit->setStyleSheet("QTextEdit { padding-bottom: 10px; border: 1px solid green;}");
    ui->btn_markdown->setProperty("isMarkdown", true);


    on_btn_refresh_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getBookBody(const QString& assetID){
    QString bookTitle;
    QString body;
    std::vector<Note>& bookNotes = notes[assetID];

    if (books.count(assetID) > 0) {
        bookTitle = books.at(assetID).Title;
        body += "# " + bookTitle + "\n";
        body += "- Author: " + books.at(assetID).Author + "\n";
        body += "- assetID: " + assetID + "\n";
    } else {
        bookTitle = assetID;
        body += "- assetID: " + assetID + "\n";
    }
    body += "\n\n";

    QString curChapter = "";
    for (const auto &noteData : bookNotes) {
        auto broaderText = noteData.BroaderText;
        auto selectedText = noteData.SelectedText;
        auto markNote = noteData.MarkNote;
        auto chapter = noteData.Chapter;

        if (broaderText.isEmpty() && selectedText.isEmpty()) {
            continue;
        }

        QString highlightedText = selectedText;

        if (!chapter.isEmpty() && curChapter != chapter) {
            body += "\n---\n\n";
            body += "### Chapter: " + chapter + "\n";
            curChapter = chapter;
        }

        if (highlightedText == curChapter) {
            continue;
        }

        body += highlightedText + "\n";
        if (!markNote.isEmpty()) {
            body += "> Note: " + markNote + "\n";
        }

        body += "\n";
    }

    return body;
}

void MainWindow::initBookUi() {
    for (int i = ui->verticalLayout->count() - 1; i >= 0; --i) {
        QWidget *widget = ui->verticalLayout->itemAt(i)->widget();
        if (widget) {
            ui->verticalLayout->removeWidget(widget);
            //widget->deleteLater();
        }
    }
    currentButton = nullptr;


    for (const auto &[assetID, _] : notes) {
        QString bookTitle;
        if (books.count(assetID) > 0) {
            bookTitle = books.at(assetID).Title;
        } else {
            bookTitle = assetID;
        }

        QPushButton* p = new QPushButton(this);
        p->setText(bookTitle);
        p->setProperty("assetID", assetID);
        p->setStyleSheet("QPushButton { text-align: left;}");
        if(!currentButton) {
            currentButton = p;
        }

        ui->verticalLayout->addWidget(p, Qt::AlignLeft);
        connect(p, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    }
    ui->area_left->setLayout(ui->verticalLayout);

    if(currentButton) {
        auto currentAssetID = currentButton->property("assetID").toString();
        ui->textEdit->setMarkdown(getBookBody(currentAssetID));
        currentButton->setStyleSheet("QPushButton { text-align: left;color:green;}");
    }


}


void MainWindow::onButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    lastButton = currentButton;
    currentButton = button;
    auto assetID = button->property("assetID").toString();

    bool isMarkdown = ui->btn_markdown->property("isMarkdown").toBool();
    if(isMarkdown) {
        ui->textEdit->setMarkdown(getBookBody(assetID));
    } else {
        ui->textEdit->setText(getBookBody(assetID));
    }

    if(lastButton) {
        lastButton->setStyleSheet("QPushButton { text-align: left;}");
    }
    if(currentButton) {
        currentButton->setStyleSheet("QPushButton { text-align: left;color:green;}");
    }

}


void MainWindow::on_btn_refresh_clicked()
{

    scanBooks();
    scanNotes();
    initBookUi();
}


void MainWindow::on_btn_markdown_clicked()
{
    if(!currentButton) {
        return;
    }
    auto currentAssetID = currentButton->property("assetID").toString();

    QPushButton *button = qobject_cast<QPushButton*>(sender());
    bool isMarkdown = button->property("isMarkdown").toBool();
    if(isMarkdown) {
        ui->textEdit->setText(getBookBody(currentAssetID));
        button->setProperty("isMarkdown", false);
    } else {
       ui->textEdit->setMarkdown(getBookBody(currentAssetID));
        button->setProperty("isMarkdown", true);
    }

}

