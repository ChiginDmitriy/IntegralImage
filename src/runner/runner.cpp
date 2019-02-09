/*! \file runner.cpp
 *  \brief Головная программа.
 */

#include "aux.hpp"

#include <iostream> // std::cerr
#include <typeinfo> // std::type_info

//! Код всей основной программы.
/*! Основная суть данной программы - выполнить полностью задание.
 *  Условие см. в файле task. Работа программы состоит из следующих шагов.
 *  1. Расчленить аргументы командной строки на части.
 *  2. Понасоздавать потоков
 *  3. В каждом потоке выполнить часть работы. Подсчёт интегрального изображения
 *     и запись оного в файл выполняется в динамической библиотеке. Она тут рядом.
 *     Посмотри Makefile и файл README.
 *  4. Корректно завершиться. В том числе, если что-то пойдёт не так.
 */
int main (const int argc, char *const argv[])
{
  try
  {
    doAllWork(parseArgs(argc, argv));
  }
  catch (const ParseError &e)
  {
    std::cerr << "Error while parsing command line args: " << e.what() << std::endl;
    return -1;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error occured while working. Exception type: " 
              << typeid(e).name() << ", message: "
              << e.what() << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cerr << "Unknown error occured while parsing command line args." << std::endl;
    return -1;
  }

  return 0;
}

