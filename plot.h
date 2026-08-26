#pragma once
#ifndef RHYTHM_PRINT_H
#define RHYTHM_PRINT_H

#include "plot.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

void printRhythm(const std::string& input, bool isFile = false, int delayMs = 100);

#endif
class Plot {
public:
	void printRhythm(const std::string& input, bool isFile, const std::string& playerName, int delayMs);

};

