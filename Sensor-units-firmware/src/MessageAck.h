#include "global_include.h"

#define BEGINACKLEN 8
class MessageAck final {
public:
  MessageAck();
  // Returns the msgID of a finished packet list if one exists
  unsigned long long insert(unsigned long long msgID, const Packet &p);
  ackListItem remove(unsigned long long msgID);
  ~MessageAck();

private:
  ackListItem *ackArr{nullptr};
  void resize();
  size_t capacity{BEGINACKLEN};
  size_t size{0};
  size_t arrSize{};
};
