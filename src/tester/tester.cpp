/*! \file tester.cpp
 *  \brief Код приложения-тестировщика.
 */

#include <gtest/gtest.h>
#include <integral_image/IntegralImgComputer.hpp>

#include <fstream>
#include <string>

namespace
{

/*! \def TEST_FILES_EQUALITY(file1, file2)
 *  \brief Проверка файлов на равенство в некотором смысле этого слова.
 *  
 *  Вынужден воспользоваться макросом, потому что хз как сработают
 *  гугловские тестилки, если проверка будет производиться внутри
 *  ещё одного метода.
 */
#define TEST_FILES_EQUALITY(file1, file2)                         \
do                                                                \
{                                                                 \
  std::ifstream f1(file1);                                        \
  std::ifstream f2(file2);                                        \
                                                                  \
  ASSERT_EQ(f1.fail(), false) << "Couldn't open file " << file1;  \
  ASSERT_EQ(f2.fail(), false) << "Couldn't open file " << file2;  \
                                                                  \
  f1.seekg(0, std::ifstream::beg);                                \
  f2.seekg(0, std::ifstream::beg);                                \
  while (f1.good() && f2.good())                                  \
  {                                                               \
    ASSERT_EQ(f1.get(), f2.get());                                \
  }                                                               \
  ASSERT_EQ(f1.eof(), true);                                      \
  ASSERT_EQ(f2.eof(), true);                                      \
}                                                                 \
while (false);          

const std::string integralExt = ".integral";
const std::string checkExt    = ".check";

/*! \def DEFINE_FILENAMES(srcFileName)
 *  \brief Определяем нужные строковые константы, содержащие имена файлов.
 */
#define DEFINE_FILENAMES(srcFileName)                             \
static const std::string iFile = (srcFileName);                   \
static const std::string oFile = iFile + integralExt;             \
static const std::string cFile = oFile + checkExt;


TEST(IntegralImageTest, green2x2jpg)
{
  DEFINE_FILENAMES("../test/2x2green.jpg");
  ASSERT_NO_THROW({IntegralImage::go(iFile, oFile);});
  TEST_FILES_EQUALITY(oFile, cFile);
}

TEST(IntegralImageTest, black8x8)
{
  DEFINE_FILENAMES("../test/8x8black.jpg");
  ASSERT_NO_THROW({IntegralImage::go(iFile, oFile);});
  TEST_FILES_EQUALITY(oFile, cFile);
}

TEST(IntegralImageTest, white9x9)
{
  DEFINE_FILENAMES("../test/9x9white.jpg");
  ASSERT_NO_THROW({IntegralImage::go(iFile, oFile);});
  TEST_FILES_EQUALITY(oFile, cFile);
}

TEST(IntegralImageTest, yellow10x10)
{
  DEFINE_FILENAMES("../test/10x10yellow.jpg");
  ASSERT_NO_THROW({IntegralImage::go(iFile, oFile);});
  TEST_FILES_EQUALITY(oFile, cFile);
}

#undef DEFINE_FILENAMES
#undef TEST_FILES_EQUALITY

} // anonymous

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
