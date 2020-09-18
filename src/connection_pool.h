/*
 * @author : yyhclimacool@gmail.com
 * @date   : 2020-09-18 勿忘国耻，吾辈自强！
 * @brief  : Simple connection pool
 */

#ifndef CONNECTION_POOL_H_
#define CONNECTION_POOL_H_

#include <memory>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>

class ConnectionPool;

class Connection {
  friend class ConnectionPool;
public:
  Connection() : ts_(Timestamp::now()) {}
  Connection(unsigned duration) 
    : ts_(Timestamp::now()), expire_duration_(duration) {}

  virtual ~Connection() = default;

  virtual int open() = 0;
  virtual int close() = 0;

private:
  bool expired() const {
    double diff = timeDifference(Timestamp::now(), ts_);
    if (static_cast<int>(diff) > expire_duration_)
      return true;
    else
      return false;
  }

  void openImpl() {
    if (open() != 0)
      return;
    ts_ = Timestamp::now();
  }

  Timestamp ts_;
  unsigned expire_duration_ = 60; // in seconds
};


class ConnectionFactory {
public:
  virtual std::shared_ptr<Connection> CreateConnection() = 0;
};


class ConnectionPool {
  public:
    ConnectionPool(const std::string &name,
        unsigned poolSize,
        std::unique_ptr<ConnectionFactory> factory)
      : poolName_(name), 
        poolSize_(poolSize),
        factory_(std::move(factory)),
        pool_(poolSize_) {

      std::lock_guard guard(entry_mutex_);
      for (size_t i = 0; i < poolSize_; ++i) {
        idle_.push_back(i);
      }
    }

    int Init() {
      return 0;
    }
    const std::string &name() const {
      return poolName_;
    }
    
    // must be thread-safe
    std::shared_ptr<Connection> getConnection() {
      //static std::default_random_engine e;
      //static std::uniform_int_distribution<size_t> u(0, poolSize_);
      //auto &connection = pool_[u(e)];
      auto index = get_idle();
      if (index == -1) { 
      }
      if (connection == nullptr) {
        // Create a connection and open it, if open failed, return nullptr
        connection = factory_->CreateConnection();
        if (connection) {
          return connection;
        } else {
          std::cerr << "Create connection error." << std::endl;
          return nullptr;
        }
      } else {
        if (!connection->expired()) {
          return connection;
        } else {
          connection->close();
          connection->openImpl();
          return connection;
        }
      }
    }
  private:
    int get_idle() {
      int index = -1;
      std::lock_guard guard(entry_mutex_);
      if (!idle_.empty()) {
        index = idle_.top();
        idle_.pop();
      } else {
        std::cerr << "No idle connections." << std::endl;
      }
      return index;
    }
    std::string poolName_;
    unsigned poolSize_;
    std::unique_ptr<ConnectionFactory> factory_;
    mutable std::shared_mutex entry_mutex_;
    std::queue<size_t> idle_;
    std::vector<std::shared_ptr<Connection>> pool_;
};

#endif // CONNECTION_POOL_H_
