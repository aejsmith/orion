/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Data stream utility functions.
 */

#include "core/data_stream.h"

/** Read from the stream until the next line break.
 * @param line		String to fill with line content. The actual line
 *			terminator will not be written.
 * @return		Whether the line was successfully read. */
bool DataStream::readLine(std::string &line) {
	line.clear();

	/* Reserve space in the string so we're not repeatedly reallocating. */
	line.reserve(256);

	char ch;
	while(read(&ch, 1) && ch != '\n')
		line.push_back(ch);

	/* Shrink down to actual size. */
	line.reserve();
	return ch == '\n';
}
