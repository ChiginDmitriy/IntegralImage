/*! \file IntegralImgComputer.cpp
 *  \brief Тут весь код библиотеки, выполняющей подсчёт
 *         интегрального изображения.
 */

#include "include/integral_image/IntegralImgComputer.hpp"
#include <opencv2/opencv.hpp>

#include <string>    // std::string
#include <sstream>   // std::ostringstream
#include <fstream>   // std::ofstream
#include <stdexcept> // std::runtime_error
#include <cmath>     // std::abs
#include <typeinfo>  // std::type_info

namespace
{

inline std::ofstream prepareOutputFile(const std::string &filePath)
{
  std::ofstream ofs(filePath, std::ofstream::trunc);
  if (ofs.fail())
  {
    throw std::runtime_error("Couldn't open output file " + filePath);
  }
  return ofs;
}

/*!
 *  С помощью этого пространства имён и сущностей в нём
 *  мы будем универсальным способом вычленять значения пикселей
 *  из cv::Mat. Универсальность означает следующее: независимо от
 *  типа содержимого матрицы cv::Mat::type() способ получения содержимого
 *  всегда один.
 */
namespace ElemGetter
{

class UnknownTypeError: public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

typedef double (*DoubleGetter)(const cv::Mat &mat, const int row, const int col);

inline double getUChar(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<uchar>(row, col);
}

inline double getSChar(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<schar>(row, col);
}

inline double getUShort(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<ushort>(row, col);
}

inline double getShort(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<short>(row, col);
}

inline double getInt(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<int>(row, col);
}

inline double getFloat(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<float>(row, col);
}

inline double getDouble(const cv::Mat &mat, const int row, const int col)
{
  return mat.at<double>(row, col);
}

inline DoubleGetter chooseGetter(const cv::Mat &mat)
{
  switch (mat.type())
  {
    case CV_8U  : return &getUChar  ;
    case CV_8S  : return &getSChar  ;
    case CV_16U : return &getUShort ;
    case CV_16S : return &getShort  ;
    case CV_32S : return &getInt    ;
    case CV_32F : return &getFloat  ;
    case CV_64F : return &getDouble ;
    default     : throw UnknownTypeError(std::to_string(mat.type()));
  }
} // inline DoubleGetter chooseGetter(const cv::Mat &mat)

} // ElemGetter

class ChannelPrintError: public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

inline void printChannel(const cv::Mat &mat, std::ofstream &ofs)
{
  if (mat.rows < 0 || mat.cols < 0)
  {
    throw ChannelPrintError("Row/col count is wrong. Sorry.");
  }

  ElemGetter::DoubleGetter getter = nullptr;
  try
  {
    getter = ElemGetter::chooseGetter(mat);
  }
  catch (const ElemGetter::UnknownTypeError &e)
  {
    std::ostringstream stream;
    stream << "Can't work with cv::Mat of type " << e.what() << ". Sorry.";
    throw ChannelPrintError(stream.str());
  }

  assert(mat.rows >= 0 && mat.cols >= 0 && getter != nullptr);

  const size_t matRows = mat.rows;
  const size_t matCols = mat.cols;
  std::vector<double> prevRow(matRows, 0.0);

  ofs << std::fixed << std::setprecision(1);

  for (size_t rows_i = 0; rows_i != matRows; ++rows_i)
  {
    double rowSum = 0;
    for (size_t cols_i = 0; cols_i != matCols; ++cols_i)
    {
      const double currMatElem = std::abs(getter(mat, rows_i, cols_i));
      const double currVal     = prevRow[cols_i] + rowSum + currMatElem;
      ofs << currVal;
      if (cols_i != matCols - 1)
      {
        ofs << ' ';
      }
      prevRow[cols_i] = currVal;
      rowSum += currMatElem;
    }
    if (rows_i != matRows - 1)
    {
      ofs << '\n';
    }
  }
} // inline void printChannel(const cv::Mat &mat, std::ofstream &ofs)


inline void go_impl(const std::string &inFilePath, const std::string &outFilePath)
{
  cv::Mat image = cv::imread(inFilePath, cv::IMREAD_COLOR);
  if (image.data == nullptr)
  {
    throw std::runtime_error("Couldn't open file");
  }
  if (image.channels() < 0)
  {
    throw std::runtime_error("Channels count is wrong. Sorry.");
  }

  assert((image.data != nullptr) && image.channels() >= 0);
  const size_t imageChanCount = image.channels();
  cv::Mat channels[imageChanCount];
  cv::split(image, channels);

  std::ofstream ofstream = prepareOutputFile(outFilePath);

  for (size_t currChanNum = 0; currChanNum != imageChanCount; ++currChanNum)
  {
    try
    {
      printChannel(channels[currChanNum], ofstream);
    }
    catch (const ChannelPrintError &e)
    {
      std::ostringstream stream;
      stream << "Channel printing error occured. Channel N" << currChanNum
             << ", message: " << e.what();
      throw std::runtime_error(stream.str());
    }

    if (currChanNum != imageChanCount - 1)
    {
      ofstream << "\n\n";
    }
    else
    {
      ofstream.close();
    }
    if (!ofstream)
    {
      throw std::runtime_error("An error occured while writing integral image.");
    }
  }
} // inline void go_impl(const std::string &inFilePath, const std::string &outFilePath)

} // anonymous


void IntegralImage::go(const std::string &inFilePath, const std::string &outFilePath)
{
  try
  {
    go_impl(inFilePath, outFilePath);
  }
  catch (const std::exception &e)
  {
    std::ostringstream stream;
    stream << "Exception occured with input file " << inFilePath
           << " with message " << e.what();
    throw std::runtime_error(stream.str());
  }
  catch (...)
  {
    throw std::runtime_error("Unknown error occured with file " + inFilePath);
  }
} // void IntegralImage::go(const std::string &inFilePath, const std::string &outFilePath)

