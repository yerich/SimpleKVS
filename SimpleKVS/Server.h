#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <ctime>
#include <iostream>
#include <string>


using boost::asio::ip::tcp;

class TCPConnection
{
public:
    TCPConnection(boost::asio::io_context& io_context, std::function<void(void)> onClose) :
        mSocket(io_context),
        mIsActive(true),
        mOnClose(mOnClose)
    {}

    tcp::socket& socket()
    {
        return mSocket;
    }

    void start()
    {
        boost::asio::async_write(
            mSocket,
            boost::asio::buffer("MOTD: hello!"),
            [this](const boost::system::error_code& error, size_t bytes_transferred) {}
        );

        doRead();
    }

private:
    void doRead() {
        //Read from client, make json and send appropriate response
        boost::asio::async_read_until(
            mSocket,
            mReadMessage,
            "\n",
            [&](const boost::system::error_code& error, size_t bytes_transferred) {
                if ((boost::asio::error::eof == error) ||
                    (boost::asio::error::connection_reset == error))
                {
                    // handle the disconnect.
                    std::cout << "Disconnected" << std::endl;
                    mIsActive = false;
                    mOnClose();
                    return;
                }

                std::string messageP;
                {
                    std::stringstream ss;
                    ss << &mReadMessage;
                    ss.flush();
                    messageP = ss.str();
                }

                std::cout << "Message:" << messageP << std::endl;
                doRead();
            }
        );
    }

    tcp::socket mSocket;
    std::string mMessage;
    boost::asio::streambuf mReadMessage;
    bool mIsActive;

    std::function<void()>& mOnClose;
};

class Server {
public:
	Server(boost::asio::io_context& io_context, unsigned port = 6278)
        : mIoContext(io_context),
        mAcceptor(io_context, tcp::endpoint(tcp::v4(), 6278))
    {
        startAccept();
    }
private:
    void startAccept()
    {
        // TODO: fix memory leak
        TCPConnection* newConnection = new TCPConnection(mIoContext, [newConnection]() {
            //delete newConnection; this doesn't work, use enable_shared_from_this instead
        });

        mAcceptor.async_accept(
            newConnection->socket(),
            [newConnection, this](const boost::system::error_code& error) {
                Server::handleAccept(newConnection, error);
            }
        );
    }

    void handleAccept(TCPConnection * newConnection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "A client connected" << std::endl;
            newConnection->start();
        }

        startAccept();
    }

    boost::asio::io_context& mIoContext;
    tcp::acceptor mAcceptor;
};
