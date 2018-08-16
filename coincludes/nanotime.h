#include <chrono>

namespace cgold {

    uint64_t nanotime() {

        auto now = std::chrono::system_clock::now();
        auto ts = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
        uint64_t nt = ts.time_since_epoch().count();
        return nt;

    }


}
