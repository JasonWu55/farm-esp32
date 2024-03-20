#include <ModbusMaster.h>
#include <vector>

namespace rs485 {
    std::vector<float> read(ModbusMaster &node, int highh, int loww);
    float round485(float a);
}

namespace upload {
    bool update(std::vector<float> datas, int id);
}
