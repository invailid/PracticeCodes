/**
 *  Copyright (c) 2011-2021 Acceletrade Technologies Private Limited. All rights
 * reserved.
 *
 *  Author  : Anil Gupta
 *  Date    : 15 Feb 2021
 *  Purpose : Connects to kafka and dumps orders and trades.
 */

#include <common/kafka_connector.h>
#include <stdio.h>

namespace acce {
namespace common {

KafkaConnector::KafkaConnector(std::string& host, std::string& port,
                               std::string& orders_topic,
                               std::string& trades_topic, bool is_enabled)
    : m_host(host),
      m_port(port),
      m_orders_topic(orders_topic),
      m_trades_topic(trades_topic),
      m_is_enabled(is_enabled) {
        if(m_is_enabled) {
          makeKafkaConnection();
        }
}

bool KafkaConnector::makeKafkaConnection() { // resolve KAFKA_ADVERTISED_HOST_NAME
  // std::cout << "Connecting to kafka... Host: " << m_host << " Port: " << m_port << std::endl; 
  cppkafka::Configuration config = {
      {"metadata.broker.list", m_host + ":" + m_port}};
  m_producer = new Producer(config);
  m_orderBuilder = new MessageBuilder(m_orders_topic);
  m_tradeBuilder = new MessageBuilder(m_trades_topic);
  isKafkaConnection = true;
  return true;
}

bool KafkaConnector::canSendMessage() {
  return m_is_enabled && isKafkaConnection;
}

bool KafkaConnector::dumpOrderMessage(std::string& /*orderMessage*/) { // add try catch, as it sometimes crash coz of this!!
//   m_producer->produce(m_orderBuilder->partition(0).payload(
//       orderMessage));  // should we get partition number from config??
//   // sleep(5);
//   m_producer->flush();
  return true;
}

bool KafkaConnector::dumpTradeMessage(std::string& tradeMessage) {
  try {
	m_producer->produce(m_tradeBuilder->partition(0).payload(
      tradeMessage));  // should we get partition number from config??
    // sleep(5);
    m_producer->flush();
  } catch (std::exception& e) {
	std::cerr << "Exception caught : " << e.what() << std::endl;
	std::cout << "Message : " << tradeMessage << std::endl;
  }
  return true;
}

}  // namespace common
}  // namespace acce
