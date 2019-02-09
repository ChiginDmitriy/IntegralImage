/*! \file aux.hpp
 *  \brief Набор вспомогательных сущностей, необходимых для работы программы-
 *         исполнителя задания (runner).
 */

#ifndef RUNNER_AUX_H
#define RUNNER_AUX_H

#include <utility> // std::pair
#include <vector>
#include <string>
#include <stdexcept> // std::invalid_argument

class ParseError: public std::invalid_argument
{
public:
  using std::invalid_argument::invalid_argument;
};

/*! Список путей до файлов, для которых нужно посчитать интегральное изображение.  */
typedef std::vector<std::string> FilePathVec;
typedef unsigned int ThreadCount;

/*! Результат обработки аргументов командной строки. */
typedef std::pair<FilePathVec, ThreadCount> ParseResult;
ParseResult parseArgs(const int argc, char *const argv[]);
/*! Непосредственно само выполнение полезной работы. */
void doAllWork(const ParseResult &parseResult);

#endif // RUNNER_AUX_H

