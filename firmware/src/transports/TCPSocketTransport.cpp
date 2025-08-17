#include "TCPSocketTransport.h"

void TCPSocketTransport::begin()
{
  Serial.println("Connect to TCP socket microphone.local:9090 to try out TCP socket streaming");
  server.begin();
}

void TCPSocketTransport::send(void *data, size_t len)
{
  // Accept new client if available
  WiFiClient newClient = server.available();
  if (newClient)
  {
    Serial.println("New Client connected");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (!clients[i]) // empty slot
      {
        clients[i] = newClient;
        break;
      }
    }
  }

  // Send to active clients, clean up dead ones
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i] && clients[i].connected())
    {
      clients[i].write((uint8_t *)data, len);
    }
    else if (clients[i])
    {
      Serial.println("Client disconnected");
      clients[i].stop();
      clients[i] = WiFiClient(); // reset slot
    }
  }
}

bool TCPSocketTransport::hasClients() const
{
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i] && clients[i].connected())
      return true;
  }
  return false;
}

size_t TCPSocketTransport::clientCount() const
{
  size_t count = 0;
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i] && clients[i].connected())
      count++;
  }
  return count;
}
