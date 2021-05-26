#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidgetItem>
#include <QDebug>
#include <QPrinter>
#include <QPrintDialog>
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->clear();
    ui->tabWidget->setTabsClosable(true);
    ui->lineEdit->setEchoMode(QLineEdit::Password);

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("mydb");
    db.setUserName("root");

    open = false;
}

MainWindow::~MainWindow()
{
    delete query;
    query = nullptr;
    delete ui;
}

void MainWindow::openTable(QStringList columns, QString name)
{
    QString queryText = "SELECT ";
    queryText.append(columns.join(", "));
    queryText.append(" FROM mydb.");
    queryText.append(name);
    queryText.append(";");
    qDebug() << queryText;
    query->exec(queryText);
    addTable(columns, name, true);
}

void MainWindow::addTable(QStringList columns, QString name, bool main)
{
    if(query->isActive())
    {
        QTableWidget *table = new QTableWidget;
        tables.push_back(table);
        int newIndex = ui->tabWidget->count();
        ui->tabWidget->insertTab(newIndex, table, name);
        int i = 0, j = 0;
        while(query->next())
        {
            table->insertRow(i);
            for(j = 0; j < columns.size(); j++)
            {
                if(i == 0)
                    table->insertColumn(j);
                table->setItem(i, j, new QTableWidgetItem(query->value(j).toString()));
            }
            i++;
        }
        if(main)
        {
            table->insertRow(table->rowCount());

            for(j = 0; j < columns.size(); j++)
            {
                if(table->columnCount() != columns.size())
                    table->insertColumn(j);

                if(j == 0)
                    table->setItem(table->rowCount() - 1, j, new QTableWidgetItem("*"));
                else
                    table->setItem(table->rowCount() - 1, j, new QTableWidgetItem(""));
            }
        }
        table->setHorizontalHeaderLabels(columns);
    }
}



void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if(index > 2)
    {
        tables.remove(index);
        ui->tabWidget->removeTab(index);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    db.setPassword(ui->lineEdit->text());
    bool ok = db.open();
    if(ok)
    {
        ui->label_2->setText("ok");
        query = new QSqlQuery;
        open = true;

        QStringList columns = {"SNUM", "EXNUM", "CLIM", "SLIM", "PDEF"};
        openTable(columns, "Результат");

        columns = {"SNUM", "SNAME", "ENUM", "CURNUM"};
        openTable(columns, "`Серии экспериментов`");

        columns = {"ENUM", "ENAME", "POS", "PLACE", "DEG"};
        openTable(columns, "Сотрудники");
    }
    else
    {
        ui->label_2->setText("no");
        open = false;
    }
}

void MainWindow::on_pushButton_clicked()
{
    int tab = ui->tabWidget->currentIndex();
    int row = tables[ui->tabWidget->currentIndex()]->currentRow();
    int keyCount = 0;

    for(int i = 0; i < tables[tab]->columnCount(); i++)
        if(tables[tab]->item(row, i)->text().isEmpty())
        {
            QMessageBox message;
            message.setText("There are empty cells!");
            message.setIcon(QMessageBox::Warning);
            message.exec();

            return;
        }

    if(tab == 0)
    {
        if(tables[tab]->item(row, 0)->text().toInt() <= 0 || tables[tab]->item(row, 1)->text().toInt() <= 0)
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        for(int i = 0; i < tables[tab]->rowCount(); i++)
        {
            if(i != row && tables[tab]->item(row, 0)->text().toInt() == tables[tab]->item(i, 0)->text().toInt() && tables[tab]->item(row, 1)->text().toInt() == tables[tab]->item(i, 1)->text().toInt())
            {
                QMessageBox message;
                message.setText("The key column contains an invalid value!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                return;
            }
        }

        query->prepare("UPDATE mydb.Результат SET SNUM = ?, EXNUM = ?, CLIM = ?, SLIM = ?, PDEF = ? WHERE SNUM = ? AND EXNUM = ?;");
        keyCount = 2;
    }
    else if(tab == 1)
    {
        if(tables[tab]->item(row, 0)->text().toInt() <= 0)
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        for(int i = 0; i < tables[tab]->rowCount(); i++)
        {
            if(i != row && tables[tab]->item(row, 0)->text().toInt() == tables[tab]->item(i, 0)->text().toInt())
            {
                QMessageBox message;
                message.setText("The key column contains an invalid value!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                return;
            }
        }

        query->prepare("UPDATE mydb.`Серии экспериментов` SET SNUM = ?, SNAME = ?, ENUM = ?, CURNUM = ? WHERE SNUM = ?;");
        keyCount = 1;
    }
    else if(tab == 2)
    {
        if(tables[tab]->item(row, 0)->text().toInt() <= 0)
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        for(int i = 0; i < tables[tab]->rowCount(); i++)
        {
            if(i != row && tables[tab]->item(row, 0)->text().toInt() == tables[tab]->item(i, 0)->text().toInt())
            {
                QMessageBox message;
                message.setText("The key column contains an invalid value!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                return;
            }
        }

        query->prepare("UPDATE mydb.Сотрудники SET ENUM = ?, ENAME = ?, POS = ?, PLACE = ?, DEG = ? WHERE ENUM = ?;");
        keyCount = 1;
    }
    else
    {
        QMessageBox message;
        message.setText("You can't change this table!");
        message.setIcon(QMessageBox::Warning);
        message.exec();
        query->clear();
        return;
    }

    for(int i = 0; i < tables[tab]->columnCount(); i++)
    {
        QString value = tables[tab]->item(row, i)->text();
        if(value[0].isDigit())
            query->addBindValue(value.toInt());
        else
            query->addBindValue(value);
    }

    for(int i = 0; i < keyCount; i++)
    {
        QString value = tables[tab]->item(row, i)->text();
        if(value[0].isDigit())
            query->addBindValue(value.toInt());
        else
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            query->clear();
            return;
        }
    }

    query->exec();
}


void MainWindow::on_pushButton_3_clicked()
{
    int tab = ui->tabWidget->currentIndex();
    int row = tables[ui->tabWidget->currentIndex()]->currentRow();
    int keyCount = 0;

    if(tab == 0)
    {
        query->prepare("DELETE FROM mydb.Результат WHERE SNUM = ? AND EXNUM = ?;");
        keyCount = 2;
    }
    else if(tab == 1)
    {
        query->prepare("DELETE FROM mydb.`Серии экспериментов` WHERE SNUM = ?;");
        keyCount = 1;
    }
    else if(tab == 2)
    {
        query->prepare("DELETE FROM mydb.Сотрудники WHERE ENUM = ?;");
        keyCount = 1;
    }
    else
    {
        QMessageBox message;
        message.setText("You can't change this table!");
        message.setIcon(QMessageBox::Warning);
        message.exec();
        query->clear();
        return;
    }

    if(row == tables[tab]->rowCount() - 1)
        return;

    for(int i = 0; i < keyCount; i++)
    {
        query->addBindValue(tables[tab]->item(row, i)->text().toInt());
    }

    query->exec();

    tables[tab]->removeRow(row);
}


void MainWindow::on_pushButton_4_clicked()
{
    int tab = ui->tabWidget->currentIndex();
    int row = tables[tab]->rowCount() - 1;
    int keyCount = 0;

    for(int i = 0; i < tables[tab]->columnCount(); i++)
        if(tables[tab]->item(row, i)->text().isEmpty())
        {
            QMessageBox message;
            message.setText("There are empty cells!");
            message.setIcon(QMessageBox::Warning);
            message.exec();

            return;
        }

    if(tab == 0)
    {
        if(tables[tab]->item(row, 0)->text().toInt() <= 0 || tables[tab]->item(row, 1)->text().toInt() <= 0)
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        for(int i = 0; i < tables[tab]->rowCount(); i++)
        {
            if(i != row && tables[tab]->item(row, 0)->text().toInt() == tables[tab]->item(i, 0)->text().toInt() && tables[tab]->item(row, 1)->text().toInt() == tables[tab]->item(i, 1)->text().toInt())
            {
                QMessageBox message;
                message.setText("The key column contains an invalid value!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                return;
            }
        }

        query->prepare("INSERT INTO mydb.Результат VALUES (?, ?, ?, ?, ?);");
        keyCount = 2;
    }
    else if(tab == 1)
    {
        if(tables[tab]->item(row, 0)->text().toInt() <= 0)
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        for(int i = 0; i < tables[tab]->rowCount(); i++)
        {
            if(i != row && tables[tab]->item(row, 0)->text().toInt() == tables[tab]->item(i, 0)->text().toInt())
            {
                QMessageBox message;
                message.setText("The key column contains an invalid value!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                return;
            }
        }

        query->prepare("INSERT INTO mydb.`Серии экспериментов` VALUES (?, ?, ?, ?);");
        keyCount = 1;
    }
    else if(tab == 2)
    {
        if(tables[tab]->item(row, 0)->text().toInt() <= 0)
        {
            QMessageBox message;
            message.setText("The key column contains an invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        for(int i = 0; i < tables[tab]->rowCount(); i++)
        {
            if(i != row && tables[tab]->item(row, 0)->text().toInt() == tables[tab]->item(i, 0)->text().toInt())
            {
                QMessageBox message;
                message.setText("The key column contains an invalid value!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                return;
            }
        }

        query->prepare("INSERT INTO mydb.Сотрудники VALUES (?, ?, ?, ?, ?);");
        keyCount = 1;
    }
    else
    {
        QMessageBox message;
        message.setText("You can't change this table!");
        message.setIcon(QMessageBox::Warning);
        message.exec();
        query->clear();
        return;
    }

    for(int i = 0; i < tables[tab]->columnCount(); i++)
    {
        QString value = tables[tab]->item(row, i)->text();
        if(value[0].isDigit())
            query->addBindValue(value.toInt());
        else
            query->addBindValue(value);
    }

    for(int i = 0; i < keyCount; i++)
    {
        QString value = tables[tab]->item(row, i)->text();
        if(!value[0].isDigit())
        {
            QMessageBox message;
            message.setText("The key column contains invalid value!");
            message.setIcon(QMessageBox::Warning);
            message.exec();
            query->clear();
            return;
        }
    }

    query->exec();

    tables[tab]->insertRow(tables[tab]->rowCount());

    for(int j = 0; j < tables[tab]->columnCount(); j++)
    {
        if(j == 0)
            tables[tab]->setItem(tables[tab]->rowCount() - 1, j, new QTableWidgetItem("*"));
        else
            tables[tab]->setItem(tables[tab]->rowCount() - 1, j, new QTableWidgetItem(""));
    }
}


void MainWindow::on_pushButton_5_clicked()
{
    if(open)
    {
        if(ui->radioButton->isChecked())
        {
            bool ok;
            QString ename = QInputDialog::getText(this, tr("Input"),
                                                    tr("Experimenter name:"), QLineEdit::Normal, "", &ok);
            if(ok)
            {
                query->prepare("SELECT ENUM, ENAME, SNUM, SNAME FROM (mydb.`Серии экспериментов` NATURAL JOIN mydb.Сотрудники) WHERE ENAME = ?;");
                query->addBindValue(ename);
                query->exec();
                QStringList columns = {"Number", "Name", "Series number", "Series name"};
                QString name = "";
                name.append(ename);
                name.append("'s series");
                addTable(columns, name, false);
            }
        }
        else if(ui->radioButton_2->isChecked())
        {
            bool ok;
            QString curname = QInputDialog::getText(this, tr("Input"),
                                                    tr("Curator name:"), QLineEdit::Normal, "", &ok);
            if(ok)
            {
                query->prepare("SELECT CURNUM, ENAME, SNUM, SNAME FROM (mydb.`Серии экспериментов` LEFT JOIN mydb.Сотрудники ON mydb.`Серии экспериментов`.CURNUM = mydb.Сотрудники.ENUM) WHERE ENAME = ?;");
                query->addBindValue(curname);
                query->exec();
                QStringList columns = {"Number", "Name", "Series number", "Series name"};
                QString name = "";
                name.append(curname);
                name.append("'s series");
                addTable(columns, name, false);
            }
        }
    }
}


void MainWindow::on_pushButton_6_clicked()
{
    if(open)
    {
        if(ui->radioButton_3->isChecked())
        {
            int tab = ui->tabWidget->currentIndex();
            int series = tables[tab]->currentRow() + 1;

            if(tab != 1 || series == 0 || series == tables[tab]->rowCount())
            {
                QMessageBox message;
                message.setText("Please, open the series table and choose a series!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                query->clear();
                return;
            }

            query->prepare("SELECT P1.SNUM, P1.SNAME, ENAME, CURNAME FROM ((SELECT ENAME AS CURNAME, SNAME, SNUM FROM (mydb.`Серии экспериментов` LEFT JOIN mydb.Сотрудники ON `Серии экспериментов`.CURNUM = Сотрудники.ENUM) ) P1 LEFT JOIN (SELECT ENAME, SNAME, SNUM FROM (mydb.`Серии экспериментов` LEFT JOIN mydb.Сотрудники USING (ENUM)) ) P2 USING(SNUM)) WHERE P1.SNUM = ?;");
            query->addBindValue(series);
            query->exec();
            QStringList columns = {"Series number", "Series name", "Experimenter", "Curator"};
            QString name = tables[ui->tabWidget->currentIndex()]->item(series - 1, 1)->text();
            name.append(" with names");
            addTable(columns, name, false);
        }
        else if(ui->radioButton_4->isChecked())
        {
            int tab = ui->tabWidget->currentIndex();
            int series = tables[tab]->currentRow() + 1;

            if(tab != 1 || series == 0 || series == tables[tab]->rowCount())
            {
                QMessageBox message;
                message.setText("Please, open the series table and choose a series!");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                query->clear();
                return;
            }

            query->prepare("SELECT mydb.`Серии экспериментов`.SNUM, mydb.`Серии экспериментов`.SNAME, AVG(mydb.Результат.CLIM) AS CLIM, AVG(mydb.Результат.SLIM) AS SLIM, AVG(mydb.Результат.PDEF) AS PDEF FROM (mydb.`Серии экспериментов` NATURAL JOIN mydb.Результат) WHERE mydb.`Серии экспериментов`.SNUM = ? GROUP BY mydb.`Серии экспериментов`.SNUM;");
            query->addBindValue(series);
            query->exec();
            QStringList columns = {"Series number", "Series name", "Creep limit", "Strength limit", "Permanent deformation"};
            QString name = "Averaged results of ";
            name.append(tables[ui->tabWidget->currentIndex()]->item(series - 1, 1)->text());
            addTable(columns, name, false);
        }
    }
}

