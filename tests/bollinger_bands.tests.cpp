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

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::inverted_power_of_ten, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/bollinger_bands.hpp" /// for the test target
#include "tean/exponential_moving_average.hpp" /// for tean::exponential_moving_average
#include "tean/simple_moving_average.hpp" /// for tean::simple_moving_average

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleNear, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <ta_func.h> /// for TA_BBANDS, TA_BBANDS_Lookback, TA_FUNC_UNST_EMA, TA_SetUnstablePeriod, TA_SUCCESS

#include <cmath> /// for std::isnan
#include <cstdint> /// for int64_t, uint32_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::addressof, std::make_unique, std::unique_ptr
#include <type_traits> /// for std::remove_cvref_t
#include <vector> /// for std::vector

namespace tean::tests
{

namespace
{

template<typename moving_average>
class bollinger_bands_test_traits;

template<>
class [[nodiscard]] bollinger_bands_test_traits<exponential_moving_average> final
{
public: static constexpr TA_MAType ta_ma_type = TA_MAType::TA_MAType_EMA;

public:
   bollinger_bands_test_traits() = delete;
   bollinger_bands_test_traits(bollinger_bands_test_traits &&) = delete;
   bollinger_bands_test_traits(bollinger_bands_test_traits const &) = delete;

   [[nodiscard]] explicit bollinger_bands_test_traits(uint32_t const unstablePeriod) noexcept :
      m_unstablePeriod(unstablePeriod)
   {}

   bollinger_bands_test_traits &operator = (bollinger_bands_test_traits &&) = delete;
   bollinger_bands_test_traits &operator = (bollinger_bands_test_traits const &) = delete;

   [[nodiscard]] std::unique_ptr<bollinger_bands<exponential_moving_average>> create_indicator(uint32_t const period, double const upperBandMultiplier, double const lowerBandMultiplier)
   {
      TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, static_cast<int>(m_unstablePeriod));
      return std::make_unique<bollinger_bands<exponential_moving_average>>(period, upperBandMultiplier, lowerBandMultiplier, m_unstablePeriod);
   }

private:
   uint32_t const m_unstablePeriod;
};

template<>
class [[nodiscard]] bollinger_bands_test_traits<simple_moving_average>
{
public: static constexpr TA_MAType ta_ma_type = TA_MAType::TA_MAType_SMA;

public:
   [[nodiscard]] bollinger_bands_test_traits() noexcept = default;
   bollinger_bands_test_traits(bollinger_bands_test_traits &&) = delete;
   bollinger_bands_test_traits(bollinger_bands_test_traits const &) = delete;

   bollinger_bands_test_traits &operator = (bollinger_bands_test_traits &&) = delete;
   bollinger_bands_test_traits &operator = (bollinger_bands_test_traits const &) = delete;

   [[nodiscard]] std::unique_ptr<bollinger_bands<simple_moving_average>> create_indicator(uint32_t const period, double const upperBandMultiplier, double const lowerBandMultiplier)
   {
      return std::make_unique<bollinger_bands<simple_moving_average>>(period, upperBandMultiplier, lowerBandMultiplier);
   }
};

}

TEST_F(TeAn, BollingerBands)
{
   constexpr uint32_t testMinPeriod = 2;
   constexpr uint32_t testMaxPeriod = 100;
   auto const testStep = [&] (decimal const &testPriceStep, double const upperBandMultiplier, double const lowerBandMultiplier, auto &&testTraits)
   {
      auto const testPricePrecision = inverted_power_of_ten[testPriceStep.scale / 2] * inverted_power_of_ten[3];
      auto const testPriceStepValue = static_cast<double>(testPriceStep);
      for (auto testPeriod = testMinPeriod; testPeriod <= testMaxPeriod; ++testPeriod)
      {
         auto const testIterationsNumber = testPeriod * 10;
         auto testIndicator = testTraits.create_indicator(testPeriod, upperBandMultiplier, lowerBandMultiplier);
         ASSERT_EQ(testPeriod, testIndicator->period());
         auto testPrices = std::make_unique<double[]>(testIndicator->lookback_period() + testIterationsNumber);
         for (uint32_t testIteration = 0; testIteration < testIndicator->lookback_period(); ++testIteration)
         {
            auto const testPrice = testPriceStepValue * random_number<int64_t>(100, 1000);
            auto const testPickValue = testIndicator->pick(testIteration, testPrice);
            ASSERT_TRUE(std::isnan(testPickValue.upper));
            ASSERT_TRUE(std::isnan(testPickValue.middle));
            ASSERT_TRUE(std::isnan(testPickValue.lower));
            auto const testCalcValue = testIndicator->calc(testIteration, testPrice);
            ASSERT_TRUE(std::isnan(testCalcValue.upper));
            ASSERT_TRUE(std::isnan(testCalcValue.middle));
            ASSERT_TRUE(std::isnan(testCalcValue.lower));
            testPrices[testIteration] = testPrice;
         }
         auto testUpperValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
         auto testMiddleValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
         auto testLowerValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
         for (uint32_t testIteration = 0; testIteration < testIterationsNumber; ++testIteration)
         {
            auto const testPrice = testPriceStepValue * random_number<int64_t>(100, 1000);
            auto const testPickValue = testIndicator->pick(testIndicator->lookback_period() + testIteration, testPrice);
            ASSERT_FALSE(std::isnan(testPickValue.upper));
            ASSERT_FALSE(std::isnan(testPickValue.middle));
            ASSERT_FALSE(std::isnan(testPickValue.lower));
            auto const testCalcValue = testIndicator->calc(testIndicator->lookback_period() + testIteration, testPrice);
            ASSERT_FALSE(std::isnan(testCalcValue.upper));
            ASSERT_FALSE(std::isnan(testCalcValue.middle));
            ASSERT_FALSE(std::isnan(testCalcValue.lower));
            ASSERT_THAT(testPickValue.upper, testing::DoubleNear(testCalcValue.upper, testPricePrecision));
            ASSERT_THAT(testPickValue.middle, testing::DoubleNear(testCalcValue.middle, testPricePrecision));
            ASSERT_THAT(testPickValue.lower, testing::DoubleNear(testCalcValue.lower, testPricePrecision));
            testPrices[testIndicator->lookback_period() + testIteration] = testPrice;
            testUpperValues[testIteration] = testing::DoubleNear(testCalcValue.upper, testPricePrecision);
            testMiddleValues[testIteration] = testing::DoubleNear(testPickValue.middle, testPricePrecision);
            testLowerValues[testIteration] = testing::DoubleNear(testPickValue.lower, testPricePrecision);
         }
         auto const testUpperMatcher = testing::ElementsAreArray(testUpperValues.get(), testIterationsNumber);
         auto const testMiddleMatcher = testing::ElementsAreArray(testMiddleValues.get(), testIterationsNumber);
         auto const testLowerMatcher = testing::ElementsAreArray(testLowerValues.get(), testIterationsNumber);
         std::vector<double> expectedUpperValues;
         std::vector<double> expectedMiddleValues;
         std::vector<double> expectedLowerValues;
         expectedUpperValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
         expectedMiddleValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
         expectedLowerValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
         {
            ASSERT_EQ(
               TA_BBANDS_Lookback(
                  static_cast<int>(testPeriod),
                  upperBandMultiplier,
                  lowerBandMultiplier,
                  std::remove_cvref_t<decltype(testTraits)>::ta_ma_type
               ),
               static_cast<int>(testIndicator->lookback_period())
            );
            int expectedFirstIndex = 0;
            int expectedNumberOfElements = 0;
            ASSERT_EQ(TA_BBANDS(
               0,
               static_cast<int>(testIndicator->lookback_period() + testIterationsNumber) - 1,
               testPrices.get(),
               static_cast<int>(testPeriod),
               upperBandMultiplier,
               lowerBandMultiplier,
               std::remove_cvref_t<decltype(testTraits)>::ta_ma_type,
               std::addressof(expectedFirstIndex),
               std::addressof(expectedNumberOfElements),
               expectedUpperValues.data(),
               expectedMiddleValues.data(),
               expectedLowerValues.data()
            ), TA_SUCCESS);
            ASSERT_EQ(expectedFirstIndex, static_cast<int>(testIndicator->lookback_period()));
            ASSERT_EQ(expectedNumberOfElements, static_cast<int>(testIterationsNumber));
            ASSERT_THAT(expectedMiddleValues, testMiddleMatcher);
            ASSERT_THAT(expectedLowerValues, testLowerMatcher);
            ASSERT_THAT(expectedUpperValues, testUpperMatcher);
            testIndicator->reset();
            for (uint32_t testIteration = 0; testIteration < testIndicator->lookback_period(); ++testIteration)
            {
               auto const testPrice = testPrices[testIteration];
               auto const testPickValue = testIndicator->pick(testIteration, testPrice);
               ASSERT_TRUE(std::isnan(testPickValue.upper));
               ASSERT_TRUE(std::isnan(testPickValue.middle));
               ASSERT_TRUE(std::isnan(testPickValue.lower));
               auto const testCalcValue = testIndicator->calc(testIteration, testPrice);
               ASSERT_TRUE(std::isnan(testCalcValue.upper));
               ASSERT_TRUE(std::isnan(testCalcValue.middle));
               ASSERT_TRUE(std::isnan(testCalcValue.lower));
            }
            {
               auto const testPrice = testPrices[testIndicator->lookback_period()];
               auto const testPickValue = testIndicator->pick(testIndicator->lookback_period(), testPrice);
               ASSERT_FALSE(std::isnan(testPickValue.upper));
               ASSERT_FALSE(std::isnan(testPickValue.middle));
               ASSERT_FALSE(std::isnan(testPickValue.lower));
               auto const testCalcValue = testIndicator->calc(testIndicator->lookback_period(), testPrice);
               ASSERT_FALSE(std::isnan(testCalcValue.upper));
               ASSERT_FALSE(std::isnan(testCalcValue.middle));
               ASSERT_FALSE(std::isnan(testCalcValue.lower));
               ASSERT_THAT(testPickValue.upper, testing::DoubleNear(testCalcValue.upper, testPricePrecision));
               ASSERT_THAT(testPickValue.middle, testing::DoubleNear(testCalcValue.middle, testPricePrecision));
               ASSERT_THAT(testPickValue.lower, testing::DoubleNear(testCalcValue.lower, testPricePrecision));
               ASSERT_THAT(expectedUpperValues[0], testing::DoubleNear(testCalcValue.upper, testPricePrecision));
               ASSERT_THAT(expectedMiddleValues[0], testing::DoubleNear(testCalcValue.middle, testPricePrecision));
               ASSERT_THAT(expectedLowerValues[0], testing::DoubleNear(testCalcValue.lower, testPricePrecision));
            }
         }
      }
   };
   auto const testBandMultiplier = 1.0;
   ASSERT_NO_FATAL_FAILURE(
      testStep(
         decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12},
         testBandMultiplier,
         testBandMultiplier,
         bollinger_bands_test_traits<exponential_moving_average>{random_number<uint32_t>(0, 10)}
      )
   );
   ASSERT_NO_FATAL_FAILURE(
      testStep(
         decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00},
         testBandMultiplier,
         testBandMultiplier,
         bollinger_bands_test_traits<exponential_moving_average>{random_number<uint32_t>(0, 10)}
      )
   );
   ASSERT_NO_FATAL_FAILURE(
      testStep(
         decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12},
         testBandMultiplier,
         testBandMultiplier,
         bollinger_bands_test_traits<simple_moving_average>{}
      )
   );
   ASSERT_NO_FATAL_FAILURE(
      testStep(
         decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00},
         testBandMultiplier,
         testBandMultiplier,
         bollinger_bands_test_traits<simple_moving_average>{}
      )
   );
}

}
