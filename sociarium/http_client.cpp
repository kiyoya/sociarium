// s.o.c.i.a.r.i.u.m: http_client.cpp
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

/* Based on the code in the following URL:
 * http://www.boost.org/doc/libs/1_40_0/doc/html/boost_asio/example/http/client/async_client.cpp
 */

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the University of Tokyo nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <istream>
#include <ostream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include "http_client.h"

namespace hashimoto_ut {

  using std::string;
  using std::istream;
  using std::ostream;
  using std::stringstream;
  using std::endl;
  using std::getline;
  using std::tr1::shared_ptr;
  using boost::asio::ip::tcp;

  using namespace boost::asio;
  using namespace boost::asio::ip;

  class HTTPClientImpl : public HTTPClient {
  public:
    ////////////////////////////////////////////////////////////////////////////////
    HTTPClientImpl(io_service& io_service,
                   string const& server, string const& path) :
    resolver_(io_service),
    socket_(io_service) {

      ostream request_stream(&request_);

      request_stream << "GET " << path << " HTTP/1.0\r\n";
      request_stream << "Host: " << server << "\r\n";
      request_stream << "User-Agent: sociarium/0.0\r\n";
      request_stream << "Accept: */*\r\n";
      request_stream << "Connection: close\r\n\r\n";

      tcp::resolver::query query(server, "http");

      resolver_.async_resolve(query,
                              boost::bind(&HTTPClientImpl::handle_resolve, this,
                                          placeholders::error,
                                          placeholders::iterator));
    }

    ////////////////////////////////////////////////////////////////////////////////
    ~HTTPClientImpl() {}

    ////////////////////////////////////////////////////////////////////////////////
    string get_contents(void) const {
      return content_.str();
    }

    ////////////////////////////////////////////////////////////////////////////////
    string get_header(void) const {
      return header_;
    }

    ////////////////////////////////////////////////////////////////////////////////
    string get_error_log(void) const {
      return error_log_.str();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////
    void handle_resolve(boost::system::error_code const& err,
                        tcp::resolver::iterator endpoint_iterator) {

      if (err) throw err;

      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
                            boost::bind(&HTTPClientImpl::handle_connect, this,
                                        placeholders::error,
                                        ++endpoint_iterator));
    }

    ////////////////////////////////////////////////////////////////////////////////
    void handle_connect(boost::system::error_code const& err,
                        tcp::resolver::iterator endpoint_iterator) {
      if (!err)
        async_write(socket_, request_,
                    boost::bind(&HTTPClientImpl::handle_write_request, this,
                                placeholders::error));

      else if (endpoint_iterator!=tcp::resolver::iterator()) {
        socket_.close();
        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
                              boost::bind(&HTTPClientImpl::handle_connect, this,
                                          placeholders::error,
                                          ++endpoint_iterator));
      }

      else throw err;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void handle_write_request(boost::system::error_code const& err) {

      if (err) throw err;

      async_read_until(socket_, response_, "\r\n",
                       boost::bind(&HTTPClientImpl::handle_read_status_line, this,
                                   placeholders::error));
    }

    ////////////////////////////////////////////////////////////////////////////////
    void handle_read_status_line(boost::system::error_code const& err) {

      if (err) throw err;

      istream response_stream(&response_);
      string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      string status_message;
      getline(response_stream, status_message);

      if (!response_stream || http_version.substr(0, 5)!="HTTP/") {
        error_log_ << "Invalid response." << endl;
        return;
      }

      if (status_code!=200) {
        error_log_ << "Response returned with status code ";
        error_log_ << status_code << "." << endl;
        return;
      }

      async_read_until(socket_, response_, "\r\n\r\n",
                       boost::bind(&HTTPClientImpl::handle_read_headers, this,
                                   placeholders::error));
    }

    ////////////////////////////////////////////////////////////////////////////////
    void handle_read_headers(boost::system::error_code const& err) {

      if (err) throw err;

      // Process the response headers.
      istream response_stream(&response_);
      string line;
      while (getline(response_stream, line) && line!="\r")
        header_ += line;

      if (response_.size()>0)
        content_ << &response_;

      async_read(socket_, response_,
                 transfer_at_least(1),
                 boost::bind(&HTTPClientImpl::handle_read_content, this,
                             placeholders::error));
    }

    ////////////////////////////////////////////////////////////////////////////////
    void handle_read_content(boost::system::error_code const& err) {

      if (!err) {
        content_ << &response_;

        async_read(socket_, response_,
                   transfer_at_least(1),
                   boost::bind(&HTTPClientImpl::handle_read_content, this,
                               placeholders::error));
      }

      else if (err!=error::eof)
        throw err;
    }

    ////////////////////////////////////////////////////////////////////////////////
    tcp::resolver resolver_;
    tcp::socket socket_;
    streambuf request_;
    streambuf response_;
    string header_;
    stringstream content_;
    stringstream error_log_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  shared_ptr<HTTPClient>
    HTTPClient::create(io_service& io_service,
                       string const& server, string const& path) {
      return shared_ptr<HTTPClient>(new HTTPClientImpl(io_service, server, path));
    }

} // The end of the namespace "hashimoto_ut"
