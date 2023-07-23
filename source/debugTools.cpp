#pragma once

#include "debugTools.h"
#include <stdio.h>
#include <stdarg.h>

namespace Debugger {

    std::vector<std::string> transientLines;
    std::vector<std::string> permanentLines;
    std::string errorMessage = "";

    void resetLines() {
        transientLines.clear();
    }

    void resetErrorMessage() {
        errorMessage = "";
    }


    void framePrint(std::string toFormat, ...) {
        char buf[512];
        va_list args;
        va_start(args, toFormat);
        vsprintf(buf, toFormat.c_str(), args);
        va_end(args);
        std::string formattedString = std::string(buf);

        transientLines.push_back(formattedString);
    }

    void print(std::string toFormat, ...) {
        char buf[512];
        va_list args;
        va_start(args, toFormat);
        vsprintf(buf, toFormat.c_str(), args);
        va_end(args);
        std::string formattedString = std::string(buf);

        permanentLines.push_back(formattedString);
    }

    void error(std::string toFormat, ...) {
        char buf[512];
        va_list args;
        va_start(args, toFormat);
        vsprintf(buf, toFormat.c_str(), args);
        va_end(args);
        std::string formattedString = std::string(buf);

        errorMessage = "ERR: " + formattedString;
        //errorMessage = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAABB";
    }

    void render() {
        iprintf("\x1b[1;1H%s", errorMessage.c_str());
        //find how many lines errorMessage takes up on dsi
        int lineOffset = errorMessage.length() / 32 + 2; //can fit 31 chars on a line, div by 32 to make sure it rounds down to 0
        if (errorMessage == "") {
            lineOffset = 1;
        }
        
        size_t i;
		for (i = 0; i < permanentLines.size(); i++) {
            //iprintf("\x1b[%i;1H%i", i + lineOffset, lineOffset);
			iprintf("\x1b[%i;1H%s", i + lineOffset, permanentLines[i].c_str());
		}

        if (permanentLines.size() > 0) {
            lineOffset += permanentLines.size() + 1;
        }
        
        for (i = 0; i < transientLines.size(); i++) {
			iprintf("\x1b[%i;1H%s", i + lineOffset, transientLines[i].c_str());
		}
    }

};
