/*
   Part of the TeAn Project (https://github.com/cpp4ever/tean), under the MIT License
   SPDX-License-Identifier: MIT

   Copyright (c) 2024 Mikhail Smirnov

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/average_true_range.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleEq, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <ta_func.h> /// for TA_ATR, TA_ATR_Lookback, TA_FUNC_UNST_ATR, TA_SetUnstablePeriod, TA_SUCCESS

#include <cmath> /// for std::isnan
#include <cstdint> /// for int64_t, uint32_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::addressof, std::make_unique
#include <vector> /// for std::vector

namespace tean::tests
{

TEST_F(TeAn, AverageTrueRange)
{
   constexpr uint32_t testMinPeriod = 2;
   constexpr uint32_t testMaxPeriod = 100;
   auto const testStep = [&] (decimal const &testPriceStep)
   {
      auto const testPriceStepValue = static_cast<double>(testPriceStep);
      for (auto testPeriod = testMinPeriod; testPeriod <= testMaxPeriod; ++testPeriod)
      {
         auto const testIterationsNumber = testPeriod * 10;
         auto const testUnstablePeriod = random_number<uint32_t>(0, testPeriod);
         average_true_range testIndicator{testPeriod, testUnstablePeriod};
         ASSERT_EQ(testPeriod, testIndicator.period());
         auto testHighPrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
         auto testLowPrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
         auto testClosePrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
         for (uint32_t testIteration = 0; testIteration < testIndicator.lookback_period(); ++testIteration)
         {
            auto const testClosePrice = testPriceStepValue * random_number<int64_t>(100, 1000);
            auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
            auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
            auto testPickAdditionalValue = 0.0;
            auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testClosePrice, testPickAdditionalValue);
            ASSERT_TRUE(std::isnan(testPickValue));
            auto testCalcAdditionalValue = 0.0;
            auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testClosePrice, testCalcAdditionalValue);
            ASSERT_TRUE(std::isnan(testCalcValue));
            if (0 == testIteration)
            {
               ASSERT_TRUE(std::isnan(testPickAdditionalValue));
               ASSERT_TRUE(std::isnan(testCalcAdditionalValue));
            }
            else
            {
               ASSERT_FALSE(std::isnan(testPickAdditionalValue));
               ASSERT_FALSE(std::isnan(testCalcAdditionalValue));
               ASSERT_DOUBLE_EQ(testPickAdditionalValue, testCalcAdditionalValue);
            }
            testHighPrices[testIteration] = testHighPrice;
            testLowPrices[testIteration] = testLowPrice;
            testClosePrices[testIteration] = testClosePrice;
         }
         auto testValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
         for (uint32_t testIteration = 0; testIteration < testIterationsNumber; ++testIteration)
         {
            auto const testClosePrice = testPriceStepValue * random_number<int64_t>(100, 1000);
            auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
            auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
            auto testPickAdditionalValue = 0.0;
            auto const testPickValue = testIndicator.pick(testIndicator.lookback_period() + testIteration, testHighPrice, testLowPrice, testClosePrice, testPickAdditionalValue);
            ASSERT_FALSE(std::isnan(testPickValue));
            ASSERT_FALSE(std::isnan(testPickAdditionalValue));
            auto testCalcAdditionalValue = 0.0;
            auto const testCalcValue = testIndicator.calc(testIndicator.lookback_period() + testIteration, testHighPrice, testLowPrice, testClosePrice, testCalcAdditionalValue);
            ASSERT_FALSE(std::isnan(testCalcValue));
            ASSERT_FALSE(std::isnan(testCalcAdditionalValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            ASSERT_DOUBLE_EQ(testPickAdditionalValue, testCalcAdditionalValue);
            testHighPrices[testIndicator.lookback_period() + testIteration] = testHighPrice;
            testLowPrices[testIndicator.lookback_period() + testIteration] = testLowPrice;
            testClosePrices[testIndicator.lookback_period() + testIteration] = testClosePrice;
            testValues[testIteration] = testing::DoubleEq(testCalcValue);
         }
         auto const testMatcher = testing::ElementsAreArray(testValues.get(), testIterationsNumber);
         std::vector<double> expectedValues;
         expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::quiet_NaN());
         {
            ASSERT_EQ(TA_SetUnstablePeriod(TA_FUNC_UNST_ATR, static_cast<int>(testUnstablePeriod)), TA_SUCCESS);
            ASSERT_EQ(TA_ATR_Lookback(static_cast<int>(testPeriod)), static_cast<int>(testIndicator.lookback_period()));
            int expectedFirstIndex = 0;
            int expectedNumberOfElements = 0;
            ASSERT_EQ(TA_ATR(
               0,
               static_cast<int>(testIndicator.lookback_period() + testIterationsNumber) - 1,
               testHighPrices.get(),
               testLowPrices.get(),
               testClosePrices.get(),
               static_cast<int>(testPeriod),
               std::addressof(expectedFirstIndex),
               std::addressof(expectedNumberOfElements),
               expectedValues.data()
            ), TA_SUCCESS);
            ASSERT_EQ(expectedFirstIndex, static_cast<int>(testIndicator.lookback_period()));
            ASSERT_EQ(expectedNumberOfElements, static_cast<int>(testIterationsNumber));
            ASSERT_THAT(expectedValues, testMatcher);
            testIndicator.reset();
            for (uint32_t testIteration = 0; testIteration < testIndicator.lookback_period(); ++testIteration)
            {
               auto const testHighPrice = testHighPrices[testIteration];
               auto const testLowPrice = testLowPrices[testIteration];
               auto const testClosePrice = testClosePrices[testIteration];
               auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testClosePrice);
               ASSERT_TRUE(std::isnan(testPickValue));
               auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testClosePrice);
               ASSERT_TRUE(std::isnan(testCalcValue));
            }
            {
               auto const testHighPrice = testHighPrices[testIndicator.lookback_period()];
               auto const testLowPrice = testLowPrices[testIndicator.lookback_period()];
               auto const testClosePrice = testClosePrices[testIndicator.lookback_period()];
               auto const testPickValue = testIndicator.pick(testIndicator.lookback_period(), testHighPrice, testLowPrice, testClosePrice);
               ASSERT_FALSE(std::isnan(testPickValue));
               auto const testCalcValue = testIndicator.calc(testIndicator.lookback_period(), testHighPrice, testLowPrice, testClosePrice);
               ASSERT_FALSE(std::isnan(testCalcValue));
               ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
               ASSERT_DOUBLE_EQ(expectedValues[0], testCalcValue);
            }
         }
      }
   };
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12}));
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00}));
}

}
