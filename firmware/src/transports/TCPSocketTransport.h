#pragma once
#include "Transport.h"
#include <WiFiClient.h>
#include <WiFiServer.h>

#define MAX_CLIENTS 10

class TCPSocketTransport : public Transport
{
private:
  WiFiServer server{9090};          // TCP server instance
  WiFiClient clients[MAX_CLIENTS];  // slots for connected clients

public:
  void begin() override;
  void send(void *data, size_t size) override;

  bool hasClients();          // helper: check if any clients are connected
  size_t clientCount();       // helper: count active clients
};
