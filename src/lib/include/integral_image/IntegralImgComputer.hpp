/*! \file IntegralImgComputer.hpp
 *  \brief Заголовочный файл библиотеки задания.
 *  Функция go торчит из бибилиотеки наружу. Её можно использовать извне.
 */

#ifndef LIB_INTEGRAL_IMG_COMPUTER_H
#define LIB_INTEGRAL_IMG_COMPUTER_H

#include <string>

namespace IntegralImage
{

//! Посчитать интегральное изображение для файла inFilePath.
//! Результат положить в outFilePath.
/*!
 * \param inFilePath Путь до файла с изображением.
 * \param outFilePath Путь до файла, куда нужно записать интегральное изображение.
 *                    Формат интегрального изображения описан в задании (файл task).
 */
void go(const std::string &inFilePath, const std::string &outFilePath);

} // IntegralImage

#endif // LIB_INTEGRAL_IMG_COMPUTER_H

