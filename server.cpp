#include <iostream>
#include <boost/asio.hpp>
#include <libasync/async.h>

void client_session(boost::asio::ip::tcp::socket sock) {
    while (true) {
        try {
            char data[4];
            size_t len = sock.read_some(boost::asio::buffer(data));
            std::cout << "receive " << len << "=" << std::string{data, len} << std::endl;
            boost::asio::write(sock, boost::asio::buffer("pong", 4));
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }
}

int main(int, char *[]) {

    boost::asio::io_service service;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), 9999);
    
    boost::asio::ip::tcp::acceptor acc(service, ep);

    async::init();
    async::handle_t handle = async::connect(2);
    async::receive(handle, "1\n2\n3\n", 6);
    async::disconnect(handle);
    async::close();

    while (true) {
        auto sock = boost::asio::ip::tcp::socket(service);
        acc.accept(sock);
        client_session(std::move(sock));
    }

    return 0;
}
