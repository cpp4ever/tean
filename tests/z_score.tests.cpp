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

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::inverted_power_of_ten, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/standard_deviation.hpp" /// for tean::standard_deviation
#include "tean/z_score.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleNear
#include <gtest/gtest.h> /// for ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE

#include <cmath> /// for std::isfinite
#include <cstdint> /// for int64_t, uint32_t
#include <ranges> ///< for std::views::iota

namespace tean::tests
{

template<uint32_t test_period>
void test_z_score_step(TeAn &fixture, decimal const testPriceStep)
{
   constexpr auto testLookbackPeriod{z_score<test_period>::lookback_period(),};
   constexpr auto testIterationsNumber{test_period * 10u,};
   auto const testPricePrecision{inverted_power_of_ten[testPriceStep.scale / 3u] * inverted_power_of_ten[3u],};
   double const testPriceStepValue{testPriceStep,};
   {
      z_score<test_period> testIndicator{};
      standard_deviation<test_period> testAdditionalIndicator{};
      for (auto const testIteration : std::views::iota(0u, testLookbackPeriod))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2u]),};
         auto testAdditionalMean{0e0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testIteration, testPrice, testAdditionalMean),};
         ASSERT_FALSE(std::isfinite(testAdditionalMean));
         ASSERT_FALSE(std::isfinite(testAdditionalValue));
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         ASSERT_FALSE(std::isfinite(testCalcValue));
      }
      for (auto const testIteration : std::views::iota(0u, testIterationsNumber))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2u]),};
         auto testAdditionalMean{0e0,};
         auto const testAdditionalValue{testAdditionalIndicator.calc(testLookbackPeriod + testIteration, testPrice, testAdditionalMean),};
         ASSERT_TRUE(std::isfinite(testAdditionalMean));
         ASSERT_TRUE(std::isfinite(testAdditionalValue));
         auto const testCalcValue{testIndicator.calc(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue));
         auto const expectedValue{(0e0 >= testAdditionalValue) ? 0e0 : ((testPrice - testAdditionalMean) / testAdditionalValue),};
         ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
      }
      testIndicator.reset();
      testAdditionalIndicator.reset();
      for (auto const testIteration : std::views::iota(0u, testLookbackPeriod + 1u))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2u]),};
         auto testAdditionalMean{0e0,};
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
            auto const expectedValue{(0e0 >= testAdditionalValue) ? 0e0 : ((testPrice - testAdditionalMean) / testAdditionalValue),};
            ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
         }
      }
   }
   {
      z_score<> testIndicator{test_period,};
      standard_deviation<> testAdditionalIndicator{test_period,};
      ASSERT_EQ(test_period, testIndicator.period());
      ASSERT_EQ(testLookbackPeriod, testIndicator.lookback_period());
      for (auto const testIteration : std::views::iota(0u, testLookbackPeriod + testIterationsNumber))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2u]),};
         auto testAdditionalMean{0e0,};
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
            auto const expectedValue{(0e0 >= testAdditionalValue) ? 0e0 : ((testPrice - testAdditionalMean) / testAdditionalValue),};
            ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
         }
      }
      testIndicator.reset();
      testAdditionalIndicator.reset();
      for (auto const testIteration : std::views::iota(0u, testLookbackPeriod + 1u))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2u]),};
         auto testAdditionalMean{0e0,};
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
            auto const expectedValue{(0e0 >= testAdditionalValue) ? 0e0 : ((testPrice - testAdditionalMean) / testAdditionalValue),};
            ASSERT_THAT(testCalcValue, testing::DoubleNear(expectedValue, testPricePrecision));
         }
      }
   }
}

template<uint32_t test_period>
void test_z_score(TeAn &fixture, decimal const testPriceStep)
{
   test_z_score_step<test_period>(fixture, testPriceStep);
   if constexpr (2u < test_period)
   {
      test_z_score<test_period - 1u>(fixture, testPriceStep);
   }
}

TEST_F(TeAn, ZScore)
{
   constexpr auto testMaxPeriod{100u,};
   ASSERT_NO_FATAL_FAILURE(test_z_score<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[0u]), .scale = 12,}));
   ASSERT_NO_FATAL_FAILURE(test_z_score<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[6u]), .scale =  0,}));
}

}
