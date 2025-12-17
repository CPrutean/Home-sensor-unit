#include "global_include.h"
#define BEGINACKLEN 8

struct ackListItem {
  unsigned long long msgID{};
  Packet packet{NULL};
  int count{};
};

class MessageAck final {
public:
  MessageAck();
  void insert(unsigned long long msgID, Packet packet, int count);
  void remove(unsigned long long msgID);
  ~MessageAck();

private:
  ackListItem *ackArr{nullptr};
  void resize();
  size_t capacity{BEGINACKLEN};
  size_t size{0};
  size_t arrSize{};
};
