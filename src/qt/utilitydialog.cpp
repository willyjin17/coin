// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "utilitydialog.h"

#include "ui_aboutdialog.h"
#include "ui_helpmessagedialog.h"

#include "bitcoingui.h"
#include "clientmodel.h"
#include "guiutil.h"

#include "clientversion.h"
#include "init.h"
#include "util.h"

#include <QLabel>
#include <QRegExp>
#include <QVBoxLayout>

/** "About" dialog box */
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        QString version = tr("Bitcoin Core") + " " + tr("version") + " " + model->formatFullVersion();
        /* On x86 add a bit specifier to the version so that users can distinguish between
         * 32 and 64 bit builds. On other architectures, 32/64 bit may be more ambigious.
         */
#if defined(__x86_64__)
        version += " " + tr("(%1-bit)").arg(64);
#elif defined(__i386__ )
        version += " " + tr("(%1-bit)").arg(32);
#endif

        /// HTML-format the license message from the core
        QString licenseInfo = QString::fromStdString(LicenseInfo());
        // Make URLs clickable
        QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
        uri.setMinimal(true); // use non-greedy matching
        licenseInfo = licenseInfo.replace(uri, "<a href=\"\\1\">\\1</a>");
        // Replace newlines with HTML breaks
        licenseInfo = licenseInfo.replace("\n\n", "<br><br>");

        ui->versionLabel->setText(version + "<br><br>" + licenseInfo);
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
    close();
}

/** "Help message" dialog box */
HelpMessageDialog::HelpMessageDialog(QWidget *parent, bool versionOnly) :
    QDialog(parent),
    ui(new Ui::HelpMessageDialog)
{
    ui->setupUi(this);
    GUIUtil::restoreWindowGeometry("nHelpMessageDialogWindow", this->size(), this);

    QString version = tr("Bitcoin Core") + " " + tr("version") + " " + QString::fromStdString(FormatFullVersion());
    QString header = tr("Usage:") + "\n" +
        "  bitcoin-qt [" + tr("command-line options") + "]                     " + "\n";

    QString coreOptions = QString::fromStdString(HelpMessage(HMM_BITCOIN_QT));

    QString uiOptions = tr("UI options") + ":\n" +
        "  -choosedatadir            " + tr("Choose data directory on startup (default: 0)") + "\n" +
        "  -lang=<lang>              " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                      " + tr("Start minimized") + "\n" +
        "  -rootcertificates=<file>  " + tr("Set SSL root certificates for payment request (default: -system-)") + "\n" +
        "  -splash                   " + tr("Show splash screen on startup (default: 1)");

    ui->helpMessageLabel->setFont(GUIUtil::bitcoinAddressFont());

    // Set help message text
    if(versionOnly)
        ui->helpMessageLabel->setText(version + "\n" + QString::fromStdString(LicenseInfo()));
    else
        ui->helpMessageLabel->setText(version + "\n" + header + "\n" + coreOptions + "\n" + uiOptions);
}

HelpMessageDialog::~HelpMessageDialog()
{
    GUIUtil::saveWindowGeometry("nHelpMessageDialogWindow", this);
    delete ui;
}

void HelpMessageDialog::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    fprintf(stdout, "%s\n", qPrintable(ui->helpMessageLabel->text()));
}

void HelpMessageDialog::showOrPrint()
{
#if defined(WIN32)
        // On Windows, show a message box, as there is no stderr/stdout in windowed applications
        exec();
#else
        // On other operating systems, print help text to console
        printToConsole();
#endif
}

void HelpMessageDialog::on_okButton_accepted()
{
    close();
}


/** "Shutdown" window */
void ShutdownWindow::showShutdownWindow(BitcoinGUI *window)
{
    if (!window)
        return;

    // Show a simple window indicating shutdown status
    QWidget *shutdownWindow = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        tr("Bitcoin Core is shutting down...") + "<br /><br />" +
        tr("Do not shut down the computer until this window disappears.")));
    shutdownWindow->setLayout(layout);

    // Center shutdown window at where main window was
    const QPoint global = window->mapToGlobal(window->rect().center());
    shutdownWindow->move(global.x() - shutdownWindow->width() / 2, global.y() - shutdownWindow->height() / 2);
    shutdownWindow->show();
}
