#pragma once

#include <vector>
#include <string>

namespace Debugger {

    void resetFrameLines();

    void resetPermanentLines();

    void resetErrorMessage();

    void framePrint(std::string toFormat, ...);

    void print(std::string toFormat, ...);

    void error(std::string toFormat, ...);

    void render();

};
