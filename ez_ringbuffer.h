
#pragma once

#include <vector>
#include <cstring>

#include "ez_math.h"

namespace ez {
    template<typename float_t>
    static constexpr float_t pi = 3.141592653589793;

    template<std::size_t fir_tap_count, typename float_t> // TODO: move hamming and sinc to another file probably
    float_t hamming(float_t samples_ago) {
        auto ret = /*float_t(25./46.)*/float_t(.5)*(float_t(1)+std::cos(float_t(2)*samples_ago*pi<float_t>/float_t(fir_tap_count)));
        return ret;
    }

    template<typename float_t>
    float_t sinc(float_t x) {
        if(x == 0) {
            return 1;
        }
        else {
            return std::sin(pi<float_t>*x)/(pi<float_t>*x);
        }
    }

    namespace detail {
        template<typename float_t>
        struct empty_ {
        protected:
            const float_t* make_copy_of_src_buffer_if_dst_and_src_alias_(const float_t* src, std::size_t count) {
                return src;
            }

            using base_class_type = empty_;
        };

        template<typename float_t>
        struct src_buffer_copy_wrapper_ {
        protected:
            const float_t* make_copy_of_src_buffer_if_dst_and_src_alias_(const float_t* src, std::size_t count) {
                if(count > src_buffer_copy_.size()) {
                    src_buffer_copy_.resize(count);
                }
                
                std::memcpy(src_buffer_copy_.data(), src, sizeof(float_t)*count);
                return src_buffer_copy_.data();
            }

            using base_class_type = src_buffer_copy_wrapper_;
            
        private:
                std::vector<float_t> src_buffer_copy_;
        };
    }

    template<typename float_t = float, bool dst_and_src_alias = true>
    class ringbuffer : public std::conditional_t<dst_and_src_alias, detail::src_buffer_copy_wrapper_<float_t>, detail::empty_<float_t>> {
    public:
        void resize(std::size_t new_size) {
            buf_.resize(std::max(new_size, std::size_t(1)));
            read_index_ = read_index_ % buf_.size();
        }

        static void default_read_write(float_t* dst, const float_t* src, float_t* buffer, std::size_t count) {
            std::memcpy(dst   ,  buffer, sizeof(float_t)*count);
            std::memcpy(buffer,  src   , sizeof(float_t)*count);
        }

        template<typename func_t = decltype(default_read_write)>
        void read_write(float_t* dst, const float_t* src, std::size_t count, func_t&& read_write_func = default_read_write) {
            size_t dst_src_index = 0;

            auto src_ = base_class_type::make_copy_of_src_buffer_if_dst_and_src_alias_(src, count);

            while(dst_src_index < count) {
                auto number_of_elements_to_process_this_iteration = std::min(buf_.size()-read_index_, count-dst_src_index);

                read_write_func(dst+dst_src_index, src_+dst_src_index, buf_.data()+read_index_, number_of_elements_to_process_this_iteration);

                read_index_ += number_of_elements_to_process_this_iteration;

                if(read_index_ == buf_.size()) {
                    read_index_ = 0;
                }
                dst_src_index += number_of_elements_to_process_this_iteration;
            }
        }

        void write_sample(float_t sample) {
            buf_[read_index_] = sample;
            read_index_ = (read_index_ + 1)%buf_.size();
        }

        float_t read_sample(std::size_t samples_ago) {
            auto index = (buf_.size()+read_index_-samples_ago)%buf_.size();

            return buf_[index];
        }

        /// assumes sample_ago is non-negative
        float_t read_sample_fractional_linear(float_t samples_ago) {
            std::size_t samples_ago_integer_component = samples_ago; // round down
            float_t samples_ago_fractional_component  = samples_ago - samples_ago_integer_component;

            auto ret = lerp(read_sample(samples_ago_integer_component), read_sample(samples_ago_integer_component+1), samples_ago_fractional_component);

            return ret;
        }

        template<std::size_t fir_tap_count>
        float_t read_sample_fractional_sinc(float samples_ago) {
            unsigned integer_component = std::floor(samples_ago);
            float_t fractional_component = samples_ago-integer_component;

            float sample_value = 0.f;

            for(unsigned i = 0; i < fir_tap_count; ++i) {
                auto signed_i = static_cast<int>(i)-static_cast<int>(fir_tap_count/2);
                float_t offset_samples_ago = signed_i-fractional_component;
                auto hamming_weight = hamming<fir_tap_count>(offset_samples_ago);
                auto sinc_weight = sinc(offset_samples_ago);
                float_t sample_weight = hamming_weight*sinc_weight;

                sample_value += sample_weight*read_sample(integer_component+i);
            }

            return sample_value;
        }

    private:
        using base_class_type = std::conditional_t<dst_and_src_alias, detail::src_buffer_copy_wrapper_<float_t>, detail::empty_<float_t>>;

        std::vector<float_t> buf_;
        std::size_t read_index_ = 0;
    };
}
