
#pragma once

#include <cmath>
#include <numbers>

#include "biquad_filter.h"

namespace ez {
    namespace detail {

    }

    template<typename float_t, auto recalculate_biquad_coefficients_func>
    class shelf_filter {
        biquad_filter<float_t> filter_;

        float_t center_frequency_,
                db_gain_,
                slope_,

                A_,
                w_0_,
                cos_w_0_,
                sin_w_0_,
                alpha_,
                sqrt_A_,

                sample_rate_; // store as a float to minimize conversions

        inline static float_t pi = std::numbers::pi;

        void recalculate_w_0_() {
            w_0_ = 2*pi*center_frequency_/sample_rate_;

            cos_w_0_ = std::cos(w_0_);
            sin_w_0_ = std::sin(w_0_);

            recalculate_alpha_();
        }

        void recalculate_alpha_() {
            alpha_ = .5*sin_w_0_*std::sqrt((A_+1/A_)*(1/slope_-1)+2);
        }

        void recalculate_biquad_coefficients_() {
            auto coefficients = recalculate_biquad_coefficients_func(A_,
                                                                     sqrt_A_,
                                                                     alpha_,
                                                                     cos_w_0_);

            filter_.set_coefficients(coefficients);
        }


    public:
        shelf_filter() = default;
        shelf_filter(float_t center_frequency, float_t db_gain, float_t slope, unsigned sample_rate) : center_frequency_(center_frequency),
                                                                                                       db_gain_(db_gain),
                                                                                                       slope_(slope),
                                                                                                       sample_rate_(sample_rate) {
            A_ = std::pow(10, db_gain_/40);
            sqrt_A_ = std::sqrt(A_);

            recalculate_alpha_();
            recalculate_biquad_coefficients_();
        }

        void set_center_frequency(float_t value) {
            center_frequency_ = value;

            recalculate_w_0_();
            recalculate_biquad_coefficients_();
        }

        void set_db_gain(float_t value) {
            db_gain_ = value;
            A_ = std::pow(10, db_gain_/40);
            sqrt_A_ = std::sqrt(A_);

            recalculate_alpha_();
            recalculate_biquad_coefficients_();
        }

        void set_slope(float_t value) {
            slope_ = value;

            recalculate_alpha_();
            recalculate_biquad_coefficients_();
        }

        void set_sample_rate(unsigned value) {
            sample_rate_ = value;

            recalculate_w_0_();
            recalculate_biquad_coefficients_();
        }

        void process(float_t in, float_t& out) {
            filter_.process(in, out);
        }
    };

    template<typename float_t>
    using low_shelf_filter = shelf_filter<float_t, [](float_t A, float_t sqrt_A, float_t alpha, float_t cos_w_0) {
        typename biquad_filter<float_t>::coefficients_t ret;

        ret.b_0 = A*((A+1)-(A-1)*cos_w_0+2*sqrt_A*alpha);
        ret.b_1 = 2*A*((A-1)-(A+1)*cos_w_0);
        ret.b_2 = A*((A+1)-(A-1)*cos_w_0-2*sqrt_A*alpha);

        ret.a_0 = (A+1)+(A-1)*cos_w_0+2*sqrt_A*alpha;
        ret.a_1 = -2*((A-1)+(A+1)*cos_w_0);
        ret.a_2 = (A+1)+(A-1)*cos_w_0-2*sqrt_A*alpha;

        return ret;
    }>;

    template<typename float_t>
    using high_shelf_filter = shelf_filter<float_t, [](float_t A, float_t sqrt_A, float_t alpha, float_t cos_w_0) {
        typename biquad_filter<float_t>::coefficients_t ret;

        ret.b_0 = A*((A+1)+(A-1)*cos_w_0+2*sqrt_A*alpha);
        ret.b_1 = -2*A*((A-1)+(A+1)*cos_w_0);
        ret.b_2 = A*((A+1)+(A-1)*cos_w_0-2*sqrt_A*alpha);

        ret.a_0 = (A+1)-(A-1)*cos_w_0+2*sqrt_A*alpha;
        ret.a_1 = 2*((A-1)-(A+1)*cos_w_0);
        ret.a_2 = (A+1)-(A-1)*cos_w_0-2*sqrt_A*alpha;

        return ret;
    }>;
}
