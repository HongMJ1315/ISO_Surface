#ifndef READER_H
#define READER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <libraw/libraw.h>

int read(std::string filename, std::string fileinfo, std::vector<unsigned char> &data);






#endif // READER_H