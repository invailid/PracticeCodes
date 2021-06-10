/**
 *  Copyright (c) 2011-2021 Acceletrade Technologies Private Limited. All rights
 * reserved.
 *
 *  Author  : Anil Gupta
 *  Date    : 15 Feb 2021
 *  Purpose : Connects to kafka and dumps orders and trades.
 */

#pragma once
#include <iostream>
#include "cppkafka/configuration.h"
#include "cppkafka/producer.h"

using cppkafka::Configuration;
using cppkafka::MessageBuilder;
using cppkafka::Producer;
using cppkafka::Topic;

namespace acce {
namespace common {

class KafkaConnector {
 public:
  explicit KafkaConnector(std::string& host, std::string& port,
                          std::string& orders_topic, std::string& trades_topic, bool is_enabled);
  bool makeKafkaConnection();
  bool canSendMessage();
  bool dumpOrderMessage(std::string& orderMessage);
  bool dumpTradeMessage(std::string& tradeMessage);

 private:
  std::string m_host = "";
  std::string m_port = "";
  std::string m_orders_topic = "";
  std::string m_trades_topic = "";
  bool m_is_enabled = false;
  bool isKafkaConnection = false;
  cppkafka::Producer* m_producer;
  cppkafka::MessageBuilder* m_orderBuilder;
  cppkafka::MessageBuilder* m_tradeBuilder;
};

}  // namespace common
}  // namespace acce
