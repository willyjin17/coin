#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryFile>
#include <QVariant>
#include <QFileOpenEvent>

#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "optionsmodel.h"
#include "paymentservertests.h"
#include "paymentrequestdata.h"
#include "util.h"



X509 *parse_b64der_cert(const char* cert_data)
{
    std::vector<unsigned char> data = DecodeBase64(cert_data);
    assert(data.size() > 0);
    const unsigned char* dptr = &data[0];
    X509 *cert = d2i_X509(NULL, &dptr, data.size());
    assert(cert);
    return cert;
}


//
// Test payment request handling
//

static SendCoinsRecipient handleRequest(PaymentServer* server, std::vector<unsigned char>& data)
{
    RecipientCatcher sigCatcher;
    QObject::connect(server, SIGNAL(receivedPaymentRequest(SendCoinsRecipient)),
                     &sigCatcher, SLOT(getRecipient(SendCoinsRecipient)));

    // Write data to a temp file:
    QTemporaryFile f;
    f.open();
    f.write((const char*)&data[0], data.size());
    f.close();

    // Create a FileOpenEvent and send it directly to the server's event filter:
    QFileOpenEvent event(f.fileName());
    server->eventFilter(NULL, &event);

    QObject::disconnect(server, SIGNAL(receivedPaymentRequest(SendCoinsRecipient)),
                        &sigCatcher, SLOT(getRecipient(SendCoinsRecipient)));

    // Return results from sigCatcher
    return sigCatcher.recipient;
}

void PaymentServerTests::paymentServerTests()
{
    OptionsModel optionsModel;
    PaymentServer* server = new PaymentServer(NULL, false);
    X509_STORE* caStore = X509_STORE_new();
    X509_STORE_add_cert(caStore, parse_b64der_cert(caCert_BASE64));
    PaymentServer::LoadRootCAs(caStore);
    server->setOptionsModel(&optionsModel);
    server->initNetManager();
    server->uiReady();

    // Now feed PaymentRequests to server, and observe signals it produces:
    std::vector<unsigned char> data = DecodeBase64(paymentrequest1_BASE64);
    SendCoinsRecipient r = handleRequest(server, data);
    QString merchant;
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString("testmerchant.org"));

    // Version of the above, with an expired certificate:
    data = DecodeBase64(paymentrequest2_BASE64);
    r = handleRequest(server, data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    // Long certificate chain:
    data = DecodeBase64(paymentrequest3_BASE64);
    r = handleRequest(server, data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString("testmerchant8.org"));

    // Long certificate chain, with an expired certificate in the middle:
    data = DecodeBase64(paymentrequest4_BASE64);
    r = handleRequest(server, data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    // Validly signed, but by a CA not in our root CA list:
    data = DecodeBase64(paymentrequest5_BASE64);
    r = handleRequest(server, data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    // Try again with no root CA's, verifiedMerchant should be empty:
    caStore = X509_STORE_new();
    PaymentServer::LoadRootCAs(caStore);
    data = DecodeBase64(paymentrequest1_BASE64);
    r = handleRequest(server, data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    delete server;
}

void RecipientCatcher::getRecipient(SendCoinsRecipient r)
{
    recipient = r;
}
