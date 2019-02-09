/*! \file aux.cpp
 *  \brief Код вспомогательных сущностей, необходимых для работы
 *         головной программы.
 */

#include "aux.hpp"
#include <integral_image/IntegralImgComputer.hpp>

#include <iostream>
#include <sstream>
#include <typeinfo>  // std::type_info
#include <thread>
#include <cassert>
#include <utility>   // std::move
 
// for getopt
#include <unistd.h>
#include <getopt.h>

namespace
{

typedef typename FilePathVec::const_iterator FilePathVecCIter;

class UIntConvError: public std::invalid_argument
{
public:
  using std::invalid_argument::invalid_argument;
};

inline unsigned int convertToUInt(const char *const str)
{
  std::istringstream stream(str);
  int result = -1;
  stream >> result;
  if (stream.fail() || !stream.eof())
  {
    throw UIntConvError("Error while parsing unsigned int.");
  }
  if (result < 0)
  {
    throw UIntConvError("The number can't be negative.");
  }
  assert(result >= 0);
  return result;
}


/*! \brief Класс, выполняющий порцию работы.
 *
 *  Весь список файлов делится на некоторое количество частей.
 *  Каждая часть обрабатывается в отдельном потоке.
 *  В конструкторе мы задаём диапазон путей тех файлов, которые 
 *  следует данному работнику обработать.
 *  Оператор () исполняет порцию работы.
 */
class Worker
{
public:
  Worker(FileNameVecCIter beg, FileNameVecCIter end)
  noexcept(noexcept(FileNameVecCIter(std::declval<FileNameVecCIter&>())))
    : m_beg(beg), m_end(end)
  {}
  void operator()() const noexcept
  {
    for (auto currFileIt = m_beg; currFileIt != m_end; ++currFileIt)
    {
      // Очень важно, чтобы данный метод оставался noexcept.
      // Однако, формирование сообщения об ошибке может обернуться исключением.
      // Поэтому, вывод сообщения об ошибке представляет собой нетривиальную задачу,
      // и она вынесена в два отдельных метода.
      try
      {
        IntegralImage::go(*currFileIt, *currFileIt + ".integral");
      }
      catch (const std::exception &e)
      {
        Worker::printStdExcErrMsg(e, currFileIt);
      }
      catch (...)
      {
        Worker::printUnkExcErrMsg(currFileIt);
      }
    }
  }
private:
  //! Безопасно вывести сообщение об ошибке в случае, если сгенерилось исключение
  //! std::exception.
  /*!
   *  \param e Само исключение. Передаётся внутрь для того, чтобы из него можно
   *           было вычленить информацию и вывести в поток ошибок (std::cerr).
   *  \param currFileIt Итэратор, указывающий на путь до файла, на котором
   *                    случилась ошибка.
   */
  static void printStdExcErrMsg(const std::exception &e, const FileNameVecCIter &currFileIt) noexcept
  {
    try
    {
      std::ostringstream stream;
      stream << "Error occured during computing file "
             << (*currFileIt) << ". Type of exception is " << typeid(e).name()
             << ", message " << e.what() << '\n';
      std::cerr << stream.str();
    }
    catch (...)
    {
      std::cerr << "std::exception occured while processing an image.\n";
    }
  }
  //! Безопасно вывести сообщение об ошибке в случае, если сгенерилось исключение
  //! не std::exception.
  /*!
   *  \param currFileIt Итэратор, указывающий на путь до файла, на котором
   *                    случилась ошибка.
   */
  static void printUnkExcErrMsg(const FileNameVecCIter &currFileIt) noexcept
  {
    try
    {
      std::ostringstream stream;
      stream << "Unknown error occured during computing file "
             << (*currFileIt) << '\n';
      std::cerr << stream.str();
    }
    catch (...)
    {
      std::cerr << "Unknown exception occured while processing an image.\n";
    }
  }
private:
  const FileNameVecCIter m_beg;
  const FileNameVecCIter m_end;
};

} // anonymous


ParseResult parseArgs(const int argc, char *const argv[])
{
  ParseResult result;
  bool threadCountMet = false;

  static const char *const optstring = "-i:t:";
  int opt = -1;
  opterr = 0;
  while ((opt = getopt(argc, argv, optstring)) != -1)
  {
    switch (opt)
    {
      case 'i':
      {
        result.first.push_back(optarg);
        break;
      }
      case 't':
      {
        if (threadCountMet)
        {
          throw ParseError
                (
                  "Thread count should be set no more than one time!"
                );
        }
        try
        {
          result.second = convertToUInt(optarg);
        }
        catch (const UIntConvError &e)
        {
          static const std::string msgBeginning =
            "Something went wrong while parsing thread count. "
            "Message from parsing function: ";
          throw ParseError(msgBeginning + e.what());
        }
        threadCountMet = true;
        break;
      }
      default: 
      {
        throw ParseError
              (
                "Error while parsing. Usage: "
                "./executable -i <path_to_image2> [-i <path_to_image2> […]] "
                "[-t <threads number>]"
              );
        break;
      }
    }
  }

  if (result.first.empty())
  {
    throw ParseError("You should specify at least one file!");
  }

  assert((!result.first.empty()));

  return result;
}
 
void doAllWork(const ParseResult &parseResult)
{
  static const ThreadCount hardwareHint = std::thread::hardware_concurrency();
  // Если работы на один поток (по разным причинам), то выполняем её в текущем потоке.
  const size_t elemCount = parseResult.first.size();
  if (
         (parseResult.second == 1)
      ||
         (parseResult.second == 0 && hardwareHint < 2)
      ||
         (elemCount < 2)
     )
  {
    Worker(parseResult.first.cbegin(), parseResult.first.cend())();
    return;
  }

  // А начиная отсюда у нас работы на несколько потоков.
  assert((parseResult.second > 1 || hardwareHint > 1) && (elemCount > 1));
  // Определяем кол-во потоков (общее).
  const ThreadCount thrCount = 
    std::min<ThreadCount>
    (
      (parseResult.second > 1) ? parseResult.second : hardwareHint,
      elemCount
    );
  assert(thrCount > 1);
  // Создаём пул дополнительных потоков.
  std::vector<std::thread> thrPool;
  thrPool.reserve(thrCount - 1);
  // Делим вектор на части. Каждая часть обрабатывается в отдельном потоке.
  // Последняя часть обрабатывается в текущем потоке.
  const size_t step = elemCount / thrCount;
  FileNameVecCIter begOfCurrPart = parseResult.first.cbegin();
  // Дальше начинаются излюбоны. Дело в том, что класс std::thread не является
  // RAII-классом. Стало быть, нужно грамотно работать с исключениями.
  // Первое, что нужно предусмотреть - это грамотно дождаться завершения
  // всех ранее созданных потоков.
  // Второе - если что-то пошло не так, выкинуть наверх исключение.
  bool errorOccured = false;
  try
  {
    for (ThreadCount thrNum = 0; thrNum != thrCount - 1; ++thrNum)
    {
      std::thread newThread(Worker(begOfCurrPart, begOfCurrPart + step));
      thrPool.push_back(std::move(newThread));
      begOfCurrPart += step;
    }
  }
  catch (...)
  {
    errorOccured = true;
  }

  try
  {
    // Worker(...) could be not noexcept! So we surround it with try-catch.
    Worker(begOfCurrPart, parseResult.first.cend())();
  }
  catch (...)
  {
    errorOccured = true;
  }

  for (auto currThrIt = thrPool.begin(); currThrIt != thrPool.end(); ++currThrIt)
  {
    try
    {
      currThrIt->join();
    }
    catch (...)
    {
      errorOccured = true;
    }
  }

  if (errorOccured)
  {
    throw std::runtime_error("An error occured during working.");
  }
}

