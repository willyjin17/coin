#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>

namespace Ui {
    class OverviewPage;
}
class WalletModel;
class TxViewDelegate;

class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setModel(WalletModel *model);

public slots:
    void setBalance(qint64 balance, qint64 unconfirmedBalance);
    void setNumTransactions(int count);

private:
    Ui::OverviewPage *ui;
    WalletModel *model;
    qint64 currentBalance;
    qint64 currentUnconfirmedBalance;

    TxViewDelegate *txdelegate;

private slots:
    void displayUnitChanged();
};

#endif // OVERVIEWPAGE_H
