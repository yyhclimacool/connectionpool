#include "noncopyable.h"

class Connection : noncopyable {
public:
  Connection();
  virtual ~Connection();
  virtual open() = 0;
  virtual close() = 0;
  bool connectionStatus_;
};

// 多线程安全的连接池
class ConnectionPool : noncopyable {
public:
  ConnectionPool(const std::string &name, size_t size)
    : poolName_(name), poolSize_(size) {
  }
private:
  std::string poolName_;
  size_t poolSize_;
};

class Policy {

};
