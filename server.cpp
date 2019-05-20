#include <iostream>
#include <sstream>
#include <memory>

#include <boost/asio.hpp>
#include <libasync/async.h>

#include "input_mux.h"

class Session : public std::enable_shared_from_this<Session>
{
    boost::asio::ip::tcp::socket m_socket;
    char m_buf[1024];

    async::handle_t m_shared_context;
    async::handle_t m_private_context;

    InputMux *m_mux;

public:
    Session(boost::asio::ip::tcp::socket sock, 
            async::handle_t shared, 
            int bulk)
        : m_socket(std::move(sock)), m_shared_context(shared)

    {
        async::handle_t m_private_context = async::connect(bulk);
        m_mux = new InputMux(m_shared_context, m_private_context);
    }

    ~Session() 
    {
        m_socket.close();
        async::disconnect(m_private_context);
        delete m_mux;
    }

    void start() 
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        m_socket.async_read_some(boost::asio::buffer(m_buf),
            [this, self](const boost::system::error_code& err, std::size_t n)
            {
                if (!err) {
                    m_mux->receive(m_buf, n);
                    do_read();
                }
            });
    }
};

class Server 
{
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket   m_socket;

    async::handle_t m_shared_context;
    int m_bulk;

public:
    Server(boost::asio::io_service& service, short port, int bulk) 
        : 
        m_acceptor(service, 
                     boost::asio::ip::tcp::endpoint(
                         boost::asio::ip::tcp::v4(), port)), 
        m_socket(service),
        m_bulk(bulk)
    {
        async::init();
        m_shared_context = async::connect(bulk);
        do_accept();
    }

    ~Server()
    {
        m_socket.close();
        async::disconnect(m_shared_context);
        async::close();
    }

private:
    void do_accept()
    {
        m_acceptor.async_accept(m_socket, 
            [this](const boost::system::error_code& err) 
            {
                if (!err) {
                    std::make_shared<Session>(std::move(m_socket), 
                                              m_shared_context,
                                              m_bulk)->start();
                }
                this->do_accept();
            });
    }
};

int main(int argc, char* argv[]) 
{
    boost::asio::io_service service;
    Server server(service, 9000, 3);
    service.run();

    return 0;
}
