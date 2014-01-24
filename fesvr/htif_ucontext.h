// See LICENSE for license details.

#ifndef _HTIF_UCONTEXT_H
#define _HTIF_UCONTEXT_H

#include "htif.h"
#include <ucontext.h>
#include <deque>

class htif_ucontext_t : public htif_t
{
 public:
  htif_ucontext_t(const std::vector<std::string>& target_args);
  virtual ~htif_ucontext_t();

  // target inteface
  void send(const void* buf, size_t size);
  void recv(void* buf, size_t size);
  bool recv_nonblocking(void* buf, size_t size);

 protected:
  // host interface
  virtual ssize_t read(void* buf, size_t max_size);
  virtual ssize_t write(const void* buf, size_t size);

  virtual size_t chunk_align() { return 64; }
  virtual size_t chunk_max_size() { return 1024; }

 private:
  ucontext_t host;
  ucontext_t target;
  std::deque<char> th_data;
  std::deque<char> ht_data;
};

#endif