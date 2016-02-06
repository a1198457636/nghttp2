/*
 * nghttp2 - HTTP/2 C Library
 *
 * Copyright (c) 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef SHRPX_HTTP_DOWNSTREAM_CONNECTION_H
#define SHRPX_HTTP_DOWNSTREAM_CONNECTION_H

#include "shrpx.h"

#include "http-parser/http_parser.h"

#include "shrpx_downstream_connection.h"
#include "shrpx_io_control.h"
#include "shrpx_connection.h"

namespace shrpx {

class DownstreamConnectionPool;
class Worker;

class HttpDownstreamConnection : public DownstreamConnection {
public:
  HttpDownstreamConnection(DownstreamConnectionPool *dconn_pool, size_t group,
                           struct ev_loop *loop, Worker *worker);
  virtual ~HttpDownstreamConnection();
  virtual int attach_downstream(Downstream *downstream);
  virtual void detach_downstream(Downstream *downstream);

  virtual int push_request_headers();
  virtual int push_upload_data_chunk(const uint8_t *data, size_t datalen);
  virtual int end_upload_data();

  virtual void pause_read(IOCtrlReason reason);
  virtual int resume_read(IOCtrlReason reason, size_t consumed);
  virtual void force_resume_read();

  virtual int on_read();
  virtual int on_write();

  virtual void on_upstream_change(Upstream *upstream);
  virtual size_t get_group() const;

  virtual bool poolable() const { return true; }

  ssize_t read_clear(uint8_t *buf, size_t buflen);
  ssize_t write_clear(struct iovec *iov, size_t iovlen);
  ssize_t read_tls(uint8_t *buf, size_t buflen);
  ssize_t write_tls(struct iovec *iov, size_t iovlen);

  int do_read();
  int do_write();
  int tls_handshake();

  int connected();
  void signal_write();

  int noop();

private:
  Connection conn_;
  std::function<int(HttpDownstreamConnection &)> do_read_, do_write_;
  std::function<ssize_t(HttpDownstreamConnection &, uint8_t *buf,
                        size_t buflen)> read_;
  std::function<ssize_t(HttpDownstreamConnection &, struct iovec *iov,
                        size_t iovlen)> write_;
  Worker *worker_;
  // nullptr if TLS is not used.
  SSL_CTX *ssl_ctx_;
  IOControl ioctrl_;
  http_parser response_htp_;
  size_t group_;
  // index of get_config()->downstream_addrs this object is using
  size_t addr_idx_;
  bool connected_;
};

} // namespace shrpx

#endif // SHRPX_HTTP_DOWNSTREAM_CONNECTION_H
