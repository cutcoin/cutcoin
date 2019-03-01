#include "logging.h"

namespace plant {

const char *tr(const char *str)
{
  return i18n_translate(str, "cryptonote::simple_wallet");
}

tools::scoped_message_writer success_msg_writer(bool color)
{
  return tools::scoped_message_writer(color ? epee::console_color_green : epee::console_color_default, false, std::string(), el::Level::Info);
}

tools::scoped_message_writer message_writer(epee::console_colors color, bool bright)
{
  return tools::scoped_message_writer(color, bright);
}

tools::scoped_message_writer fail_msg_writer()
{
  return tools::scoped_message_writer(epee::console_color_red, true, tr("Error: "), el::Level::Error);
}


bool verbosity::d_verbose = false;

bool &verbosity::verbose()
{
  return d_verbose;
}

} // namespace plant