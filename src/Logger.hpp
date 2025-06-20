#pragma once
#include <But/Log/Logger.hpp>
#include <But/Log/Destination/Console.hpp>
#include <But/Log/Destination/AutoFlush.hpp>
#include <But/Log/Field/LogLevel.hpp>
#include <But/Log/Field/PreciseDateTime.hpp>
#include <But/Log/Field/ThreadId.hpp>
#include <But/Log/Field/Pid.hpp>
#include <typeinfo>


struct Logger final
{
  using EntryArray = But::Log::Backend::EntryArray;
  using EntryProxy = But::Log::Backend::EntryProxy;

  using DestPtr = But::Log::Logger<>::Destination;

  explicit Logger(DestPtr d): impl_{ std::move(d) } { }

  template<typename ...Args>
  void debug(std::string_view message, Args&& ...args) const
  {
    log( But::Log::Field::LogLevel::debug, message, std::forward<Args>(args)... );
  }

  template<typename ...Args>
  void info(std::string_view message, Args&& ...args) const
  {
    log( But::Log::Field::LogLevel::info, message, std::forward<Args>(args)... );
  }

  template<typename ...Args>
  void warning(std::string_view message, Args&& ...args) const
  {
    log( But::Log::Field::LogLevel::warning, message, std::forward<Args>(args)... );
  }

  template<typename ...Args>
  void error(std::string_view message, Args&& ...args) const
  {
    log( But::Log::Field::LogLevel::error, message, std::forward<Args>(args)... );
  }

  template<typename ...Args>
  auto withFields(Args&& ...args) const
  {
    return Logger{ impl_.withFields( std::forward<Args>(args)... ) };
  }

  void reload() { impl_.reload(); }
  void flush()  { impl_.flush();  }

private:
  explicit Logger(But::Log::Logger<>&& l): impl_{ std::move(l) } { }

  template<typename ...Args>
  void log(But::Log::Field::LogLevel ll, std::string_view message, Args&& ...args) const
  {
    // here some common fields are adde: UTC ISO timestamp, PID, thread ID
    impl_.log( message,
        ll,
        But::Log::Field::PreciseDateTime{},
        But::Log::Field::Pid{},
        But::Log::Field::ThreadId{},
        std::forward<Args>(args)... );
  }

  But::Log::Logger<> impl_;
};


template<typename Dst, typename ...Args>
auto make_logger(Args&& ...args)
{
  auto dst = But::makeSharedNN<Dst>( std::forward<Args>(args)... );
  auto flushed = But::makeSharedNN<But::Log::Destination::AutoFlush>( std::move(dst) );
  return Logger{ std::move(flushed) };
}

inline auto make_console_logger(bool syncWithStdio = true)
{
  return make_logger<But::Log::Destination::Console>(syncWithStdio);
}


struct Exception
{
  std::exception const& value_;
};

inline constexpr auto fieldName(Exception const*) { return "Exception"; }
inline auto objectValue(Logger::EntryProxy& p, Exception const& ex)
{
  p.value("type", typeid(ex.value_).name());
  p.value("what", ex.value_.what());
}
