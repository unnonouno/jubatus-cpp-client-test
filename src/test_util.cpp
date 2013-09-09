#include "test_util.hpp"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <iostream>

#include <pficommon/lang/cast.h>

using std::string;
using std::cout;
using std::endl;

namespace {

void wait_server(int port) {
  msgpack::rpc::client cli("127.0.0.1", port);
  cli.set_timeout(10);
  int64_t sleep_time = 1000;
  // 1000 * \sum {i=0..9} 2^i = 1024000 micro sec = 1024 ms
  for (int i = 0; i < 10; ++i) {
    usleep(sleep_time);
    try {
      cli.call(std::string("dummy")).get<bool>();
      throw std::runtime_error("dummy rpc successed");
    } catch (const msgpack::rpc::no_method_error& e) {
      return;
    } catch (const msgpack::rpc::connect_error& e) {
      // wait until the server bigins to listen
    }
    sleep_time *= 2;
  }
  throw std::runtime_error("cannot connect");
}

pid_t fork_process(
    const std::string& name,
    int port,
    const std::string& config) {
  int pipefd[2];
  pipe(pipefd);
  pid_t child = fork();
  if (child == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDERR_FILENO);

    string cmd = string("juba") + name;

    string port_str = pfi::lang::lexical_cast<std::string>(port);
    const char* const argv[] = {
      cmd.c_str(),
      "-p", port_str.c_str(),
      "-f", config.c_str(),
      "-d", ".",
      NULL };
    int ret = execvp(cmd.c_str(), const_cast<char* const*>(argv));
    if (ret < 0) {
      perror("execvp");
      cout << cmd << " " << child << endl;
    }
    cout << "exec failed"<< endl;
  } else if (child < 0) {
    perror("--");
    return -1;
  }
  close(pipefd[1]);

  try {
    wait_server(port);
  } catch(std::exception& e) {
    char buf;
    while (read(pipefd[0], &buf, 1) > 0)
      write(STDOUT_FILENO, &buf, 1);
    throw;
  }
  return child;
}

void kill_process(pid_t child) {
  if (kill(child, SIGTERM) != 0) {
    perror("");
    return;
  }
  int status = 0;
  waitpid(child, &status, 0);
}

}  // namespace

client_test_base::client_test_base(
    const std::string& name,
    int port,
    const pfi::text::json::json& config)
    : name_(name), port_(port) {
  std::ofstream ofs(make_config_path().c_str());
  config.pretty(ofs, false);
}

std::string client_test_base::make_config_path() const {
  return std::string("./config.") + name_ + ".json";
}

void client_test_base::kill_server() {
  kill_process(this->child_);
}

void client_test_base::exec_server() {
  child_ = fork_process(name_,
                        port_,
                        make_config_path());
}
