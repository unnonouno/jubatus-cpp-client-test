// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef JUBATUS_CLIENT_TEST_UTIL_HPP_
#define JUBATUS_CLIENT_TEST_UTIL_HPP_

#include <unistd.h>
#include <string>

#include <gtest/gtest.h>
#include <jubatus/msgpack/rpc/client.h>
#include <pficommon/text/json.h>

class client_test_base : public testing::Test {
 public:
  client_test_base(const std::string& name,
                   int port,
                   const pfi::text::json::json& config);

  int get_port() const {
    return port_;
  }

 protected:
  void kill_server();
  void exec_server();

 private:
  std::string make_config_path() const;

  std::string name_;
  int port_;
  pid_t child_;
};

template <typename Client>
class client_test : public client_test_base {
 public:
  client_test(const std::string& name,
              int port,
              const pfi::text::json::json& config)
      : client_test_base(name, port, config) {
  }

  virtual void SetUp() {
    exec_server();
    client_ = pfi::lang::shared_ptr<Client>(
        new Client("127.0.0.1", get_port(), 10));
  }

  virtual void TearDown() {
    client_.reset();
    kill_server();
  }

  Client& get_client() {
    return *client_;
  }

 private:
  pfi::lang::shared_ptr<Client> client_;
};

#endif  // JUBATUS_CLIENT_TEST_UTIL_HPP_
