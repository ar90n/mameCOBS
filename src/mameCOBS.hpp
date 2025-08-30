// mameCOBS_v2.hpp - Chunk-of-chunks COBS implementation
#pragma once

#include <concepts>
#include <cstddef>
#include <expected>
#include <optional>
#include <ranges>
#include <span>

namespace mamecobs
{
  // Error types for decode operations
  enum class decode_error
  {
    oversized,    // Frame exceeds MaxFrameSize
    invalid_cobs, // Invalid COBS data structure
    incomplete    // Incomplete frame at end of stream
  };
  template <class T>
  concept ByteLike = std::same_as<std::remove_cvref_t<T>, std::byte> ||
                     (std::integral<std::remove_cvref_t<T>> && sizeof(std::remove_cvref_t<T>) == 1);

  template <class T>
  concept ByteRange = std::ranges::input_range<T> && ByteLike<std::ranges::range_value_t<T>>;

  template <class T>
  concept ByteRangeRange = std::ranges::input_range<T> && ByteRange<std::ranges::range_value_t<T>>;

  inline constexpr std::byte frame_delim{ 0x00 };

  namespace
  {
    template <class T>
    [[nodiscard]] constexpr std::byte to_byte(T v) noexcept
    {
      if constexpr (std::same_as<std::remove_cvref_t<T>, std::byte>)
      {
        return v;
      }
      else
      {
        return static_cast<std::byte>(static_cast<unsigned char>(v));
      }
    }

    namespace views
    {
      // COBS Encoder: Range<Range<byte>> -> Range<byte>
      // Encodes multiple frames into a single COBS stream
      template <std::ranges::input_range R>
        requires ByteRangeRange<R>
      class encode : public std::ranges::view_interface<encode<R>>
      {
        using Base = std::views::all_t<R>;
        Base base_;
        bool append_delim_;

      public:
        encode() = default;
        encode(R r, bool append_delim = true)
            : base_(std::views::all(std::move(r)))
            , append_delim_(append_delim)
        {
        }

        encode(const encode &) = delete;
        encode &operator=(const encode &) = delete;
        encode(encode &&) = default;
        encode &operator=(encode &&) = default;

        class iterator
        {
          using BaseIter = std::ranges::iterator_t<Base>;
          using BaseSent = std::ranges::sentinel_t<Base>;

          BaseIter frames_it_;
          BaseSent frames_end_;
          bool append_delim_;

          std::ranges::iterator_t<std::ranges::range_value_t<Base>> current_frame_it_;
          std::ranges::sentinel_t<std::ranges::range_value_t<Base>> current_frame_end_;

          std::array<std::byte, 255> unit_buffer_;
          std::size_t unit_size_ = 0;
          std::size_t unit_pos_ = 0;
          enum class encode_state
          {
            start_of_frame,
            start_of_chunk,
            on_byte,
            end_of_chunk,
            end_of_last_chunk,
            end_of_frame,
            finished,
          };
          encode_state state_ = encode_state::start_of_frame;

          bool can_start_next_frame()
          {
            return frames_it_ != frames_end_;
          }

          void setup_next_frame()
          {
            auto &&frame = *frames_it_;
            current_frame_it_ = std::ranges::begin(frame);
            current_frame_end_ = std::ranges::end(frame);
            ++frames_it_;
          }

          encode_state process_start_of_frame()
          {
            if (!can_start_next_frame())
            {
              return encode_state::finished;
            }

            setup_next_frame();
            return encode_state::start_of_chunk;
          }

          encode_state process_start_of_chunk()
          {
            unit_size_ = 1;
            return encode_state::on_byte;
          }

          encode_state process_on_byte()
          {
            if (current_frame_it_ == current_frame_end_)
            {
              return encode_state::end_of_last_chunk;
            }

            if (255 <= unit_size_)
            {
              return encode_state::end_of_chunk;
            }

            std::byte b = to_byte(*current_frame_it_);
            ++current_frame_it_;
            if (b == frame_delim)
            {
              return encode_state::end_of_chunk;
            }

            unit_buffer_[unit_size_] = b;
            ++unit_size_;

            return encode_state::on_byte;
          }

          encode_state process_end_of_chunk()
          {
            unit_buffer_[0] = static_cast<std::byte>(unit_size_);
            return encode_state::start_of_chunk;
          }

          encode_state process_end_of_last_chunk()
          {
            unit_buffer_[0] = static_cast<std::byte>(unit_size_);
            return encode_state::end_of_frame;
          }

          encode_state process_end_of_frame()
          {
            if (append_delim_ || can_start_next_frame())
            {
              unit_buffer_[0] = frame_delim;
              unit_size_ = 1;
              return encode_state::start_of_frame;
            }

            unit_size_ = 0;
            return encode_state::finished;
          }

          bool build_next_unit()
          {
            unit_pos_ = 0; // unit_size_ は状態関数で管理

            while (true)
            {
              switch (state_)
              {
              case encode_state::start_of_frame:
                state_ = process_start_of_frame();
                break;
              case encode_state::start_of_chunk:
                state_ = process_start_of_chunk();
                break;
              case encode_state::on_byte:
                state_ = process_on_byte();
                break;
              case encode_state::end_of_chunk:
                state_ = process_end_of_chunk();
                return true;
              case encode_state::end_of_last_chunk:
                state_ = process_end_of_last_chunk();
                return true;
              case encode_state::end_of_frame:
                state_ = process_end_of_frame();
                return true;
              case encode_state::finished:
                unit_size_ = 0;
                return false;
              }
            }
          }

        public:
          using value_type = std::byte;
          using difference_type = std::ptrdiff_t;
          using iterator_category = std::input_iterator_tag;

          iterator() = default;
          iterator(BaseIter it, BaseSent end, bool append_delim)
              : frames_it_(it)
              , frames_end_(end)
              , append_delim_(append_delim)
          {
            build_next_unit();
          }

          std::byte operator*() const
          {
            return unit_buffer_[unit_pos_];
          }

          iterator &operator++()
          {
            ++unit_pos_;

            if (unit_pos_ >= unit_size_)
            {
              build_next_unit();
            }

            return *this;
          }

          void operator++(int)
          {
            ++*this;
          }

          friend bool operator==(const iterator &it, std::default_sentinel_t) noexcept
          {
            return it.state_ == encode_state::finished && it.unit_pos_ >= it.unit_size_;
          }
        };

        iterator begin()
        {
          return iterator{ std::ranges::begin(base_), std::ranges::end(base_), append_delim_ };
        }

        std::default_sentinel_t end()
        {
          return {};
        }
      };

      // COBS Decoder: Range<byte> -> Range<expected<span<byte>, error>>
      // Decodes a COBS stream into multiple frames with error handling
      template <std::size_t MaxFrameSize, std::ranges::input_range R>
        requires ByteLike<std::ranges::range_value_t<R>>
      class decode : public std::ranges::view_interface<decode<MaxFrameSize, R>>
      {
        using Base = std::views::all_t<R>;
        Base base_;

      public:
        static constexpr std::size_t max_frame_size = MaxFrameSize;

        decode() = default;
        explicit decode(R r)
            : base_(std::views::all(std::move(r)))
        {
        }

        decode(const decode &) = delete;
        decode &operator=(const decode &) = delete;
        decode(decode &&) = default;
        decode &operator=(decode &&) = default;

        class iterator
        {
          using BaseIter = std::ranges::iterator_t<Base>;
          using BaseSent = std::ranges::sentinel_t<Base>;

          BaseIter it_;
          BaseSent end_;

          std::array<std::byte, MaxFrameSize> frame_buffer_;
          std::size_t frame_size_ = 0;

          std::optional<decode_error> current_error_;
          bool frame_ready_ = false;
          bool finished_ = false;

          enum class decode_state
          {
            wait_for_code,
            read_data_bytes,
            handle_zero,
            frame_complete,
            error_state,
            finished
          };
          decode_state state_ = decode_state::wait_for_code;

          std::size_t code_ = 0;
          std::size_t bytes_read_ = 0;

          void skip_to_delimiter()
          {
            while (it_ != end_)
            {
              if (to_byte(*it_) == frame_delim)
              {
                ++it_;
                break;
              }
              ++it_;
            }
            current_error_.reset();
            frame_ready_ = false;
            state_ = decode_state::wait_for_code;
          }
          decode_state process_wait_for_code()
          {
            if (it_ == end_)
            {
              return decode_state::finished;
            }

            std::byte code_byte = to_byte(*it_);
            ++it_;

            if (code_byte == frame_delim)
            {
              return decode_state::frame_complete;
            }

            code_ = static_cast<std::size_t>(code_byte);
            if (code_ == 0)
            {
              current_error_ = decode_error::invalid_cobs;
              return decode_state::error_state;
            }

            bytes_read_ = 0;
            return decode_state::read_data_bytes;
          }

          decode_state process_read_data_bytes()
          {
            if (code_ == 1)
            {
              return decode_state::handle_zero;
            }

            while (bytes_read_ < code_ - 1 && it_ != end_)
            {
              if (MaxFrameSize <= frame_size_)
              {
                current_error_ = decode_error::oversized;
                return decode_state::error_state;
              }

              std::byte b = to_byte(*it_);
              if (b == frame_delim)
              {
                current_error_ = decode_error::invalid_cobs;
                return decode_state::error_state;
              }

              frame_buffer_[frame_size_] = b;
              ++frame_size_;
              ++it_;
              ++bytes_read_;
            }

            if (bytes_read_ == code_ - 1)
            {
              return decode_state::handle_zero;
            }

            current_error_ = decode_error::incomplete;
            return decode_state::error_state;
          }

          decode_state process_handle_zero()
          {
            if (255 <= code_)
            {
              return decode_state::wait_for_code;
            }

            if (it_ == end_)
            {
              current_error_ = decode_error::incomplete;
              return decode_state::error_state;
            }

            std::byte next = to_byte(*it_);
            if (next == frame_delim)
            {
              ++it_;
              return decode_state::frame_complete;
            }

            if (frame_size_ >= MaxFrameSize)
            {
              current_error_ = decode_error::oversized;
              return decode_state::error_state;
            }
            frame_buffer_[frame_size_++] = std::byte{ 0 };

            return decode_state::wait_for_code;
          }

          decode_state process_frame_complete()
          {
            frame_ready_ = true;
            return decode_state::wait_for_code;
          }

          decode_state process_error_state()
          {
            return decode_state::error_state;
          }

          bool decode_next_frame()
          {
            frame_size_ = 0;
            frame_ready_ = false;
            current_error_.reset();
            state_ = decode_state::wait_for_code;

            while (true)
            {
              switch (state_)
              {
              case decode_state::wait_for_code:
                state_ = process_wait_for_code();
                break;

              case decode_state::read_data_bytes:
                state_ = process_read_data_bytes();
                break;

              case decode_state::handle_zero:
                state_ = process_handle_zero();
                break;

              case decode_state::frame_complete:
                state_ = process_frame_complete();
                return true; // Frame successfully decoded

              case decode_state::error_state:
                return false; // Error occurred

              case decode_state::finished:
                finished_ = true;
                return false; // No more data
              }
            }
          }

        public:
          using frame_type = std::span<const std::byte>;
          using value_type = std::expected<frame_type, decode_error>;
          using difference_type = std::ptrdiff_t;
          using iterator_category = std::input_iterator_tag;

          iterator() = default;
          iterator(BaseIter it, BaseSent end)
              : it_(it)
              , end_(end)
          {
            finished_ = !decode_next_frame() && !current_error_;
          }

          value_type operator*() const
          {
            if (current_error_)
            {
              return std::unexpected(*current_error_);
            }
            return frame_type{ frame_buffer_.data(), frame_size_ };
          }

          iterator &operator++()
          {
            if (current_error_)
            {
              skip_to_delimiter();
            }

            if (!finished_)
            {
              finished_ = !decode_next_frame() && !current_error_;
            }

            return *this;
          }

          void operator++(int)
          {
            ++*this;
          }

          friend bool operator==(const iterator &it, std::default_sentinel_t) noexcept
          {
            return it.finished_;
          }
        };

        iterator begin()
        {
          return iterator{ std::ranges::begin(base_), std::ranges::end(base_) };
        }

        std::default_sentinel_t end()
        {
          return {};
        }
      };
    } // namespace views

    namespace adapters
    {
      struct encode
      {
        bool append_delim_;

        explicit encode(bool append_delim)
            : append_delim_(append_delim)
        {
        }

        template <std::ranges::input_range R>
          requires ByteRangeRange<R>
        auto operator()(R &&r) const
        {
          using R_type = std::remove_cvref_t<R>;
          return views::encode<R_type>{ std::forward<R>(r), append_delim_ };
        }

        template <std::ranges::input_range R>
          requires ByteRange<R> && (!ByteRangeRange<R>)
        auto operator()(R &&r) const
        {
          auto single_frame = std::views::all(std::forward<R>(r));
          auto frames = std::views::single(single_frame);
          using FramesType = decltype(frames);
          return views::encode<FramesType>{ frames, append_delim_ };
        }

        template <ByteLike T>
        auto operator()(T &&b) const
        {
          auto single_byte = std::views::single(to_byte(b));
          auto frames = std::views::single(single_byte);
          using FramesType = decltype(frames);
          return views::encode<FramesType>{ frames, append_delim_ };
        }
      };

      template <std::size_t MaxFrameSize = 4096>
      struct decode
      {
        template <std::ranges::input_range R>
          requires ByteRange<R>
        auto operator()(R &&r) const
        {
          using R_type = std::remove_cvref_t<R>;
          return views::decode<MaxFrameSize, R_type>{ std::forward<R>(r) };
        }

        template <ByteLike T>
        auto operator()(T &&b) const
        {
          auto chunk = std::views::single(to_byte(b));
          using ChunkType = decltype(chunk);
          return views::decode<MaxFrameSize, ChunkType>{ chunk };
        }
      };
    } // namespace adapters
  } // anonymous namespace

  inline auto encode(bool append_delim = true)
  {
    return adapters::encode{ append_delim };
  }

  template <std::size_t MaxFrameSize = 4096>
  inline auto decode()
  {
    return adapters::decode<MaxFrameSize>{};
  }
  template <std::ranges::input_range R>
  auto operator|(R &&r, const adapters::encode &adapter)
  {
    return adapter(std::forward<R>(r));
  }

  template <std::ranges::input_range R, std::size_t MaxFrameSize>
  auto operator|(R &&r, const adapters::decode<MaxFrameSize> &adapter)
  {
    return adapter(std::forward<R>(r));
  }

  template <ByteLike T>
  auto operator|(T &&b, const adapters::encode &adapter)
  {
    return adapter(std::forward<T>(b));
  }

  template <ByteLike T, std::size_t MaxFrameSize>
  auto operator|(T &&b, const adapters::decode<MaxFrameSize> &adapter)
  {
    return adapter(std::forward<T>(b));
  }

} // namespace mamecobs