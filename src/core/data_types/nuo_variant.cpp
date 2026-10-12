#include "core/data_types/nuo_variant.hpp"

namespace nuostl
{

const char* NuoBadVariantAccess::what() const noexcept
{
  return "NuoVariant access failed";
}

} /* namespace nuostl */
