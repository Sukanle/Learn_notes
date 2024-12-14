#pragma once
#ifndef _TICKTOCK_H_
#define _TICKTOCK_H_
#include <tbb/tick_count.h>
#include <iostream>
#define TICK(x) auto bench_##x = tbb::tick_count::now();
#define TOCK(x) std::cout << #x ": " << (tbb::tick_count::now() - bench_##x).seconds() << "s" << std::endl;
#endif
