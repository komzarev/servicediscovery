#include <QtTest>

//#include "../../serverdiscovery/ssdp_qt.cpp"
#include "../../serverdiscovery/ssdp_qt.hpp"
using namespace ssdp::qt;
class ClientUtility : public QObject
{
    Q_OBJECT

public:
    ClientUtility();
    ~ClientUtility();

private slots:
    void test_ipMaskMatch();
    void test_ipMaskNotMatch();
};

ClientUtility::ClientUtility()
{
}

ClientUtility::~ClientUtility()
{
}

void ClientUtility::test_ipMaskMatch()
{
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.168.50.*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.150", "192.168.50.*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.168.*.*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.*.*.*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.*.50.15"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.*.50.*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "*.168.50.15"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.168.50.1*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.105", "192.168.50.*5"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.150", "1*2.1*8.50.1*"));
    QVERIFY(Client::isIpMatchedToMask("192.168.50.15", "192.168.50.15"));
}

void ClientUtility::test_ipMaskNotMatch()
{

    QVERIFY(!Client::isIpMatchedToMask("192.168.50.15", "192.168.50.15*"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.51.15", "192.168.50.*"));
    QVERIFY(!Client::isIpMatchedToMask("193.168.50.150", "192.168.50.*"));
    QVERIFY(!Client::isIpMatchedToMask("192.169.50.15", "192.168.*.*"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.50.150", "192.*.50.15"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.250.15", "192.*.50.*"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.50.151", "*.168.50.15"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.50.112", "192.168.50.*1"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.50.5", "192.168.50.*5"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.50.211", "1*2.1*8.50.1*"));
    QVERIFY(!Client::isIpMatchedToMask("192.168.50.15", "192.168.50.15*"));
}

QTEST_APPLESS_MAIN(ClientUtility)

#include "tst_clientutility.moc"
