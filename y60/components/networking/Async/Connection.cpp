//
// Connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "Connection.h"
#include <asl/base/Logger.h>
#include <vector>
#include <boost/bind.hpp>

namespace y60 {
namespace async {
namespace http {

connection::connection(boost::asio::io_service& io_service, ConcurrentQueue<request> & theRequestQueue)
  : strand_(io_service),
    request_queue(theRequestQueue),
    socket_(io_service)
{
    AC_TRACE << "Connection CTOR " << this;
}

connection::~connection() {
    AC_TRACE << "Connection DTOR " << this;
}

boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::start()
{
  AC_TRACE << "starting to read " << this;
  socket_.async_read_some(boost::asio::buffer(buffer_),
      strand_.wrap(
        boost::bind(&connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));
}

void connection::async_respond(const reply & r) {
    AC_TRACE << "queuing response " << this;
    reply_ = r;
    AC_TRACE << "replay: '" << reply_.content << "'";
    boost::asio::async_write(socket_, reply_.to_buffers(),
            strand_.wrap(
                boost::bind(&connection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error)));
}


void connection::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
  AC_TRACE << "read completed " << e << " for " << this;
  if (!e)
  {
    boost::tribool result;
    AC_TRACE << std::string(buffer_.data(), bytes_transferred);
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);

    if (result)
    {
      AC_TRACE << "queuing request to JS " << this;
      request_.conn = shared_from_this();
      request_queue.push(request_);
      request_.conn.reset();
    }
    else if (!result)
    {
        AC_TRACE << "queuing write bad_request " << this;
      reply_ = reply::stock_reply(reply::bad_request);
      boost::asio::async_write(socket_, reply_.to_buffers(),
          strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
              boost::asio::placeholders::error)));
    }
    else
    {
      AC_TRACE << "need to read more: queuing again " << this;
      socket_.async_read_some(boost::asio::buffer(buffer_),
          strand_.wrap(
            boost::bind(&connection::handle_read, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

void connection::handle_write(const boost::system::error_code& e)
{
  AC_TRACE << "response done. Closing connection " << e << " for " << this;
  if (!e)
  {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

} // namespace http
} // namespace async
} // namespace y60
