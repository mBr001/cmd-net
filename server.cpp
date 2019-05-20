#include <iostream>
#include <sstream>

#include <boost/asio.hpp>
#include <libasync/async.h>

class Session
{
    boost::asio::ip::tcp::socket m_socket;
    char m_buf[1024];

public:
    Session(boost::asio::ip::tcp::socket sock)
        : m_socket(std::move(sock))
    {
        do_read();
    }

private:
    void do_read()
    {
        m_socket.async_read_some(boost::asio::buffer(m_buf),
            [this](const boost::system::error_code& err, std::size_t n)
            {
                if (err) {
                    std::cout << err.message() << std::endl;
                    return;
                }               
                do_read();
            });
    }
};

class Server 
{
    boost::asio::ip::tcp::acceptor  m_acceptor;
    boost::asio::ip::tcp::socket    m_server_socket;

    Session *m_client_session;

public:
    Server(boost::asio::io_service& service, short port) 
        : 
        m_acceptor(service, 
                     boost::asio::ip::tcp::endpoint(
                         boost::asio::ip::tcp::v4(), port)), 
        m_server_socket(service),
        m_client_session(nullptr)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        m_acceptor.async_accept(m_server_socket, 
            [this](const boost::system::error_code& err) 
            {
                std::cout << "accepted" << std::endl;
                std::cout << err.message() << std::endl;
                m_client_session = new Session(std::move(m_server_socket));
                this->do_accept();
            });
    }
};

int main(int argc, char* argv[]) 
{

    boost::asio::io_service service;
    Server server(service, 9000);
    service.run();

    return 0;
}
