/*
   Part of the TeAn Project (https://github.com/cpp4ever/tean), under the MIT License
   SPDX-License-Identifier: MIT

   Copyright (c) 2024-2026 Mikhail Smirnov

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

#include "decimal.tests.hpp"
#include "tean.tests.hpp"

#include "tean/standard_deviation.hpp"
#include "tean/z_score.hpp"

#include <gmock/gmock.h>

namespace tean::tests
{

template<uint32_t test_period>
void test_z_score_step(TeAn &fixture, decimal const testPriceStep)
{
   constexpr auto testLookbackPeriod{z_score<test_period>::lookback_period,};
   constexpr auto testIterationsNumber{test_period * 10,};
   auto const testPricePrecision{inverted_power_of_ten[testPriceStep.scale / 3] * inverted_power_of_ten[3],};
   double const testPriceStepValue{testPriceStep,};
   {
      z_score<test_period> testIndicator{};
      standard_deviation<test_period> testAdditionalIndicator{};
      for (uint32_t testIteration{0,}; testIteration < testLookbackPeriod; ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2]),};
         double testAdditionalMean{0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testIteration, testPrice, testAdditionalMean),};
         ASSERT_FALSE(std::isfinite(testAdditionalMean));
         ASSERT_FALSE(std::isfinite(testAdditionalValue));
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         ASSERT_FALSE(std::isfinite(testCalcValue));
      }
      for (uint32_t testIteration{0,}; testIteration < testIterationsNumber; ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2]),};
         double testAdditionalMean{0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testLookbackPeriod + testIteration, testPrice, testAdditionalMean),};
         ASSERT_TRUE(std::isfinite(testAdditionalMean));
         ASSERT_TRUE(std::isfinite(testAdditionalValue));
         auto const testCalcValue{testIndicator.calc(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue));
         auto const expectedValue{(0 >= testAdditionalValue) ? 0.0 : (testPrice - testAdditionalMean) / testAdditionalValue,};
         ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
      }
      testIndicator.reset();
      testAdditionalIndicator.reset();
      for (uint32_t testIteration{0,}; testIteration <= testLookbackPeriod; ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2]),};
         double testAdditionalMean{0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testIteration, testPrice, testAdditionalMean),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testAdditionalMean));
            ASSERT_FALSE(std::isfinite(testAdditionalValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_TRUE(std::isfinite(testAdditionalMean));
            ASSERT_TRUE(std::isfinite(testAdditionalValue));
            ASSERT_TRUE(std::isfinite(testCalcValue));
            auto const expectedValue{(0 >= testAdditionalValue) ? 0.0 : (testPrice - testAdditionalMean) / testAdditionalValue,};
            ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
         }
      }
   }
   {
      z_score<> testIndicator{test_period,};
      standard_deviation<> testAdditionalIndicator{test_period,};
      ASSERT_EQ(test_period, testIndicator.period());
      ASSERT_EQ(testLookbackPeriod, testIndicator.lookback_period());
      for (uint32_t testIteration{0,}; testIteration < (testLookbackPeriod + testIterationsNumber); ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2]),};
         double testAdditionalMean{0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testIteration, testPrice, testAdditionalMean),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testAdditionalMean));
            ASSERT_FALSE(std::isfinite(testAdditionalValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_TRUE(std::isfinite(testAdditionalMean));
            ASSERT_TRUE(std::isfinite(testAdditionalValue));
            ASSERT_TRUE(std::isfinite(testCalcValue));
            auto const expectedValue{(0 >= testAdditionalValue) ? 0.0 : (testPrice - testAdditionalMean) / testAdditionalValue,};
            ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
         }
      }
      testIndicator.reset();
      testAdditionalIndicator.reset();
      for (uint32_t testIteration{0,}; testIteration <= testLookbackPeriod; ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2]),};
         double testAdditionalMean{0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testIteration, testPrice, testAdditionalMean),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testAdditionalMean));
            ASSERT_FALSE(std::isfinite(testAdditionalValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_TRUE(std::isfinite(testAdditionalMean));
            ASSERT_TRUE(std::isfinite(testAdditionalValue));
            ASSERT_TRUE(std::isfinite(testCalcValue));
            auto const expectedValue{(0 >= testAdditionalValue) ? 0.0 : (testPrice - testAdditionalMean) / testAdditionalValue,};
            ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
         }
      }
   }
}

template<uint32_t test_period>
void test_z_score(TeAn &fixture, decimal const testPriceStep)
{
   test_z_score_step<test_period>(fixture, testPriceStep);
   if constexpr (2 < test_period)
   {
      test_z_score<test_period - 1>(fixture, testPriceStep);
   }
}

TEST_F(TeAn, ZScore)
{
   constexpr uint32_t testMaxPeriod{100,};
   ASSERT_NO_FATAL_FAILURE(test_z_score<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12}));
   ASSERT_NO_FATAL_FAILURE(test_z_score<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00}));
}

}
