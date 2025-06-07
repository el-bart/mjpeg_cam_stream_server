#include "Client_handler.hpp"

Client_handler::Client_handler(But::System::Descriptor fd):
  fd_{ std::move(fd) }
{ }

void Client_handler::enqueueFrame(std::shared_ptr<Jpeg> frame)
{
  // TODO...
}

void Client_handler::nonBlockingIo()
{
  // TODO...
}
