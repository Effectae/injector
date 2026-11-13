#include "payload.h"

static const char payload_[] = {
#embed "payload.bin"
};

const char* payload = payload_;
const size_t payload_size = sizeof(payload_) / sizeof(payload_[0]);
