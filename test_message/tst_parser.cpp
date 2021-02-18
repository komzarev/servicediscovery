#include <QtTest>

#include "../serverdiscovery/ssdp_message.hpp"

class Parser : public QObject
{
    Q_OBJECT

public:
    Parser();
    ~Parser();

private slots:
    void test_Request();
    void test_Response();
    void test_ResponseRequestMatch();

};

Parser::Parser()
{

}

Parser::~Parser()
{

}

void Parser::test_Request()
{
    ssdp::Request req("mpnet-server","name","42");
    auto str = req.to_string();
    QVERIFY(!str.empty());
    auto req2 = ssdp::Request::from_string(str);
    QVERIFY(req2.has_value());
    QCOMPARE(req.servername, req2->servername);
    QCOMPARE(req.serverdetails, req2->serverdetails);
    QCOMPARE(req.servertype, req2->servertype);
}

void Parser::test_Response()
{
    ssdp::Response resp("mpnet-server", "name","42","127.0.0.1:5454");

    auto str = resp.to_string();
    QVERIFY(!str.empty());
    auto req2 = ssdp::Response::from_string(str);
    QVERIFY(req2.has_value());
    QCOMPARE(resp.servername, req2->servername);
    QCOMPARE(resp.serverdetails, req2->serverdetails);
    QCOMPARE(resp.servertype, req2->servertype);
    QCOMPARE(resp.location, req2->location);

}

void Parser::test_ResponseRequestMatch()
{
    ssdp::Response resp("mpnet-server", "name","42","127.0.0.1:5454");

    QVERIFY(resp.matchRequest(ssdp::Request("mpnet-server","name","42")));
    QVERIFY(resp.matchRequest(ssdp::Request("mpnet-server","name","")));
    QVERIFY(!resp.matchRequest(ssdp::Request("mpnet-server","name","43")));

    QVERIFY(resp.matchRequest(ssdp::Request("mpnet-server","","42")));
    QVERIFY(!resp.matchRequest(ssdp::Request("mpnet-server","namezz","")));
    QVERIFY(!resp.matchRequest(ssdp::Request("mpnet-server","namezz","42")));

    QVERIFY(resp.matchRequest(ssdp::Request("mpnet-server","","")));
    QVERIFY(!resp.matchRequest(ssdp::Request("mpnet-serverzz","","")));
    QVERIFY(!resp.matchRequest(ssdp::Request("mpnet-serverzz","name","42")));
}

QTEST_APPLESS_MAIN(Parser)

#include "tst_parser.moc"
