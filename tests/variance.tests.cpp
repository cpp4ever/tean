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

#include "tean/simple_moving_average.hpp" /// for tean::simple_moving_average
#include "tean/variance.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleNear, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <indicators.h> /// for TI_OKAY, ti_var, ti_var_start
#include <ta_func.h> /// for TA_SUCCESS, TA_VAR, TA_VAR_Lookback

#include <algorithm> /// for std::ranges::fill
#include <memory> /// for std::addressof, std::make_unique
#include <ranges> ///< for std::views::iota
#include <vector> /// for std::vector

namespace tean::tests
{

template<uint32_t test_period>
void test_variance_step(TeAn &fixture, decimal const testPriceStep)
{
   constexpr auto testLookbackPeriod{variance<test_period>::lookback_period,};
   constexpr auto testIterationsNumber{test_period * 10ui32,};
   auto const testPrices{std::make_unique<double[]>(testLookbackPeriod + testIterationsNumber),};
   auto const testValues{std::make_unique<testing::Matcher<double>[]>(testIterationsNumber),};
   {
      variance<test_period> testIndicator{};
      auto const testPricePrecision{inverted_power_of_ten[testPriceStep.scale / 3ui32] * inverted_power_of_ten[1ui32],};
      double const testPriceStepValue{testPriceStep,};
      {
         tean::simple_moving_average testAdditionalIndicator{test_period};
         for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod))
         {
            auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 1ui32]),};
            [[maybe_unused]] auto const testAdditionalValue{testAdditionalIndicator.calc(testIteration, testPrice),};
            auto testPickAdditionalValue{0e0,};
            auto const testPickValue{testIndicator.pick(testIteration, testPrice, testPickAdditionalValue),};
            ASSERT_FALSE(std::isfinite(testPickAdditionalValue));
            ASSERT_FALSE(std::isfinite(testPickValue));
            auto testCalcAdditionalValue{0e0,};
            auto const testCalcValue{testIndicator.calc(testIteration, testPrice, testCalcAdditionalValue),};
            ASSERT_FALSE(std::isfinite(testCalcAdditionalValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
            testPrices[testIteration] = testPrice;
         }
         for (auto const testIteration : std::views::iota(0ui32, testIterationsNumber))
         {
            auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 1ui32]),};
            auto const testAdditionalValue{testAdditionalIndicator.calc(testLookbackPeriod + testIteration, testPrice),};
            auto testPickAdditionalValue{0e0,};
            auto const testPickValue{testIndicator.pick(testLookbackPeriod + testIteration, testPrice, testPickAdditionalValue),};
            ASSERT_TRUE(std::isfinite(testPickAdditionalValue));
            ASSERT_THAT(testAdditionalValue, testing::DoubleNear(testPickAdditionalValue, testPricePrecision));
            ASSERT_TRUE(std::isfinite(testPickValue));
            auto testCalcAdditionalValue{0e0,};
            auto const testCalcValue{testIndicator.calc(testLookbackPeriod + testIteration, testPrice, testCalcAdditionalValue),};
            ASSERT_TRUE(std::isfinite(testCalcAdditionalValue));
            ASSERT_THAT(testAdditionalValue, testing::DoubleNear(testCalcAdditionalValue, testPricePrecision));
            ASSERT_TRUE(std::isfinite(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickAdditionalValue, testCalcAdditionalValue);
            ASSERT_THAT(testPickValue, testing::DoubleNear(testCalcValue, testPricePrecision));
            testPrices[testLookbackPeriod + testIteration] = testPrice;
            testValues[testIteration] = testing::DoubleNear(testCalcValue, testPricePrecision);
         }
      }
      auto const testMatcher{testing::ElementsAreArray(testValues.get(), testIterationsNumber),};
      std::vector<double> expectedValues;
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_VAR_Lookback(static_cast<int>(test_period), 1e0), static_cast<int>(testLookbackPeriod));
         auto expectedFirstIndex{0i32,};
         auto expectedNumberOfElements{0i32,};
         ASSERT_EQ(TA_VAR(
            0i32,
            static_cast<int>(testLookbackPeriod + testIterationsNumber) - 1i32,
            testPrices.get(),
            static_cast<int>(test_period),
            1e0,
            std::addressof(expectedFirstIndex),
            std::addressof(expectedNumberOfElements),
            expectedValues.data()
         ), TA_SUCCESS);
         ASSERT_EQ(expectedFirstIndex, static_cast<int>(testLookbackPeriod));
         ASSERT_EQ(expectedNumberOfElements, static_cast<int>(testIterationsNumber));
         ASSERT_THAT(expectedValues, testMatcher);
         testIndicator.reset();
         for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod))
         {
            auto const testPrice{testPrices[testIteration],};
            auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
            ASSERT_FALSE(std::isfinite(testPickValue));
            auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         {
            auto const testPrice{testPrices[testLookbackPeriod],};
            auto const testPickValue{testIndicator.pick(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testPickValue));
            auto const testCalcValue{testIndicator.calc(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testCalcValue));
            ASSERT_THAT(testPickValue, testing::DoubleNear(testCalcValue, testPricePrecision));
            ASSERT_THAT(expectedValues[0ui32], testing::DoubleNear(testCalcValue, testPricePrecision));
         }
      }
      std::ranges::fill(expectedValues, std::numeric_limits<double>::signaling_NaN());
      {
         double *testInputs[]{testPrices.get(),};
         double const testOptions[]{test_period,};
         double *testOutputs[]{expectedValues.data(),};
         ASSERT_EQ(ti_var_start(testOptions), static_cast<int>(testLookbackPeriod));
         ASSERT_EQ(TI_OKAY, ti_var(static_cast<int>(testLookbackPeriod + testIterationsNumber), testInputs, testOptions, testOutputs));
         ASSERT_THAT(expectedValues, testMatcher);
      }
   }
   {
      variance<> testIndicator{test_period,};
      ASSERT_EQ(test_period, testIndicator.period());
      ASSERT_EQ(testLookbackPeriod, testIndicator.lookback_period());
      for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod + testIterationsNumber))
      {
         auto const testPickValue{testIndicator.pick(testIteration, testPrices[testIteration]),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrices[testIteration]),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testPickValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_THAT(testPickValue, testValues[testIteration - testLookbackPeriod]);
            ASSERT_THAT(testCalcValue, testValues[testIteration - testLookbackPeriod]);
         }
      }
      testIndicator.reset();
      for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod + 1ui32))
      {
         auto const testPickValue{testIndicator.pick(testIteration, testPrices[testIteration]),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrices[testIteration]),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testPickValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_THAT(testPickValue, testValues[testIteration - testLookbackPeriod]);
            ASSERT_THAT(testCalcValue, testValues[testIteration - testLookbackPeriod]);
         }
      }
   }
}

template<uint32_t test_period>
void test_variance(TeAn &fixture, decimal const testPriceStep)
{
   test_variance_step<test_period>(fixture, testPriceStep);
   if constexpr (2ui32 < test_period)
   {
      test_variance<test_period - 1>(fixture, testPriceStep);
   }
}

TEST_F(TeAn, Variance)
{
   constexpr auto testMaxPeriod{100ui32,};
   ASSERT_NO_FATAL_FAILURE(test_variance<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[0ui32]), .scale = 12ui8,}));
   ASSERT_NO_FATAL_FAILURE(test_variance<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[6ui32]), .scale = 00ui8,}));
}

}
