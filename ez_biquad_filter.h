
#pragma once

namespace ez {
    template<typename float_t>
    class biquad_filter {
        float_t a_0_,
                inv_a_0_,
                a_1_,
                a_2_,

                b_0_,
                b_1_,
                b_2_;

        float_t x_n_1_ = 0,
                x_n_2_ = 0,

                y_n_1_ = 0,
                y_n_2_ = 0;
    public:
        biquad_filter() = default;

        biquad_filter(float_t a_0, float_t a_1, float_t a_2,
                      float_t b_0, float_t b_1, float_t b_2) : a_0_(a_0), inv_a_0_(1/a_0), a_1_(a_1), a_2_(a_2),
                                                               b_0_(b_0),                  b_1_(b_1), b_2_(b_2) {}

        void set_coefficients(float_t a_0, float_t a_1, float_t a_2,
                              float_t b_0, float_t b_1, float_t b_2)  {
            a_0_ = a_0;
            inv_a_0_ = 1/a_0_;
            a_1_ = a_1;
            a_2_ = a_2;

            b_0_ = b_0;
            b_1_ = b_1;
            b_2_ = b_2;
        }

        struct coefficients_t {
            float_t a_0,
                    a_1,
                    a_2,

                    b_0,
                    b_1,
                    b_2;

        };

        void set_coefficients(const coefficients_t& coefficients)  {
            set_coefficients(coefficients.a_0, coefficients.a_1, coefficients.a_2,
                             coefficients.b_0, coefficients.b_1, coefficients.b_2);
        }

        void set_a_0(float_t value) {
            a_0_ = value;
            inv_a_0_ = 1/a_0_;
        }
        void set_a_1(float_t value) {
            a_1_ = value;
        }
        void set_a_2(float_t value) {
            a_2_ = value;
        }


        void set_b_0(float_t value) {
            b_0_ = value;
        }
        void set_b_1(float_t value) {
            b_1_ = value;
        }
        void set_b_2(float_t value) {
            b_2_ = value;
        }

        void process(float_t in, float_t& out) {
            const auto& x_n = in;
            auto y_n = inv_a_0_*(b_0_*x_n+b_1_*x_n_1_+b_2_*x_n_2_-a_1_*y_n_1_-a_2_*y_n_2_);

            x_n_2_ = x_n_1_;
            x_n_1_ = x_n;

            y_n_2_ = y_n_1_;
            y_n_1_ = y_n;

            out = y_n;
        }
    };
}