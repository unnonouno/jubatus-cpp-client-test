// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2012 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.
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

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <jubatus/client.hpp>
#include "test_util.hpp"
#include <jubatus/core/fv_converter/converter_config.hpp>

using std::string;
using std::make_pair;
using jubatus::common::datum;
using pfi::data::optional;

static const int PORT = 65436;
static const std::string NAME = "";

namespace {

pfi::text::json::json make_simple_config(const string& method) {
  using pfi::text::json::json;
  using pfi::text::json::json_object;
  using pfi::text::json::json_string;
  using pfi::text::json::json_integer;
  using pfi::text::json::to_json;

  json js(new json_object());
  js["method"] = to_json(method);

  json anomaly_config(new json_object());
  anomaly_config["method"] = to_json(string("euclid_lsh"));
  anomaly_config["nearest_neighbor_num"] = to_json(100);
  anomaly_config["reverse_nearest_neighbor_num"] = to_json(30);

  json euclid_conf(new json_object);
  euclid_conf["lsh_num"] = to_json(8);
  euclid_conf["table_num"] = to_json(8);
  euclid_conf["probe_num"] = to_json(8);
  euclid_conf["bin_width"] = to_json(8.2);  // float
  euclid_conf["seed"] = to_json(1234);
  euclid_conf["retain_projection"] = to_json(true);

  anomaly_config["parameter"] = euclid_conf;
  js["parameter"] = anomaly_config;

  jubatus::core::fv_converter::converter_config config;
  jubatus::core::fv_converter::num_rule rule = {
    "*", optional<string>(), "num"
  };
  config.num_rules.push_back(rule);
  js["converter"] = pfi::text::json::to_json(config);

  return js;
}

class anomaly_test : public client_test<jubatus::anomaly::client::anomaly> {
 public:
  anomaly_test()
      : client_test<jubatus::anomaly::client::anomaly>(
          "anomaly", PORT, make_simple_config("lof")) {
  }
};

TEST_F(anomaly_test, small) {
  jubatus::anomaly::client::anomaly& c = get_client();

  {
    datum d;
    d.add_number("f1", 1.0);

    c.add(NAME, d);  // is it good to be inf?
    std::pair<std::string, float> w = c.add(NAME, d);
    float v = c.calc_score(NAME, d);
    ASSERT_DOUBLE_EQ(w.second, v);
  }
  {
    std::vector<std::string> rows = c.get_all_rows(NAME);
    ASSERT_EQ(2u, rows.size());
  }
  c.save(NAME, "id");
  c.clear(NAME);
  {
    std::vector<std::string> rows = c.get_all_rows(NAME);
    ASSERT_EQ(0u, rows.size());
  }
  c.load(NAME, "id");
  {
    std::vector<std::string> rows = c.get_all_rows(NAME);
    ASSERT_EQ(2u, rows.size());
  }
  { c.get_status(NAME);
  }
}

TEST_F(anomaly_test, api_get_client) {
jubatus::anomaly::client::anomaly& cli = get_client();
  string to_get = cli.get_config(NAME);

  msgpack::rpc::client& conn = cli.get_client();
  EXPECT_NO_THROW(conn.close());
}

}  // namespace
