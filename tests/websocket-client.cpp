#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <filesystem>
#include <iostream>

using NetworkMonitor::WebSocketClient;

BOOST_AUTO_TEST_SUITE(network_monitor); 

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
    // Connection targets
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/echo"};
    const std::string port {"443"};
    const std::string message {"Hello WebSocket"};

    // Always start with an I/O context object.
    boost::asio::io_context ioc {};

    // Create an SSL context object.
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TESTS_CACERT_PEM);

    // The class under test
    WebSocketClient client {url, endpoint, port, ioc, ctx};

    // We use these flags to check that the connection, send, receive functions
    // work as expected.
    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool messageMatches {false};
    bool disconnected {false};

    // Our own callbacks
    auto onSend {[&messageSent](auto ec) {
        messageSent = !ec;
    }};
    auto onConnect {[&client, &connected, &onSend, &message](auto ec) {
        connected = !ec;
        if (!ec) {
            client.Send(message, onSend);
        }
    }};
    auto onClose {[&disconnected](auto ec) {
        disconnected = !ec;
    }};
    auto onReceive {[&client,
                      &onClose,
                      &messageReceived,
                      &messageMatches,
                      &message](auto ec, auto received) {
        messageReceived = !ec;
        messageMatches = message == received;
        client.Close(onClose);
    }};

    // We must call io_context::run for asynchronous callbacks to run.
    client.Connect(onConnect, onReceive);
    ioc.run();

    // When we get here, the io_context::run function has run out of work to do.
    bool ok {
        connected &&
        messageSent &&
        messageReceived &&
        messageMatches &&
        disconnected
    };
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(cacert_pem)
{
    BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}


bool CheckResponse(const std::string& response)
{
    // We do not parse the whole message. We only check that it contains some
    // expected items.
    bool ok {true};
    ok &= response.find("ERROR") != std::string::npos;
    ok &= response.find("ValidationInvalidAuth") != std::string::npos;
    return ok;
}

BOOST_AUTO_TEST_CASE(send_stomp_frame)
{
        // Connection targets for the secure network-events server
        const std::string url {"ltnm.learncppthroughprojects.com"};
        const std::string endpoint {"/network-events"};
        const std::string port {"443"};
    
        // Always start with an I/O context object.
        boost::asio::io_context ioc {};
    
        // Create an SSL context object.
        boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
        ctx.load_verify_file(TESTS_CACERT_PEM);
    
        // The class under test
        WebSocketClient client {url, endpoint, port, ioc, ctx};

        const std::string username {"fake_username"};
        const std::string password {"fake_password"};

        std::stringstream ss {};
        ss << "STOMP" << std::endl
        << "accept-version:1.2" << std::endl
        << "host:transportforlondon.com" << std::endl
        << "login:" << username << std::endl
        << "passcode:" << password << std::endl
        << std::endl // Headers need to be followed by a blank line.
        << '\0'; // The body (even if absent) must be followed by a NULL octet.

        const std::string stompFrame {ss.str()};
    
        // We use these flags to check that the connection, send, receive functions
        // work as expected.
        bool connected {false};
        bool frameSent {false};
        bool responseReceived {false};
        bool responseValid {false};
        bool disconnected {false};

        auto onSend {[&frameSent](auto ec) {
            frameSent = !ec;
        }};
        
        auto onConnect {[&client, &connected, &onSend, &stompFrame](auto ec) {
            connected = !ec;
            if (!ec) {
                client.Send(stompFrame, onSend);
            }
        }};

        auto onClose {[&disconnected](auto ec) {
            disconnected = !ec;
        }};

        auto onReceive {[&client, &onClose, &responseReceived, &responseValid](auto ec, auto received) {
            responseReceived = !ec;
            if (!ec) {
                std::cout << "Received response: " << received << std::endl;
                responseValid = CheckResponse(received);
                std::cout << "Response valid: " << responseValid << std::endl;
            }
            client.Close(onClose);
        }};

        client.Connect(onConnect, onReceive);
        ioc.run();

        // When we get here, the io_context::run function has run out of work to do.
        std::cout << "Test results:" << std::endl;
        std::cout << "  connected: " << connected << std::endl;
        std::cout << "  frameSent: " << frameSent << std::endl;
        std::cout << "  responseReceived: " << responseReceived << std::endl;
        std::cout << "  responseValid: " << responseValid << std::endl;
        std::cout << "  disconnected: " << disconnected << std::endl;
        
        bool ok {
            connected &&
            frameSent &&
            responseReceived &&
            responseValid &&
            disconnected
        };

        BOOST_CHECK(ok);    
}


BOOST_AUTO_TEST_SUITE_END();
