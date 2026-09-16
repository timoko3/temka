#ifndef ENGINE_CORE_UNIQUE_PTR_HH
#define ENGINE_CORE_UNIQUE_PTR_HH

#include <memory>

// Алиас UniquePtr (architecture.md §5.1): пока — обычный std::unique_ptr.
// В будущем может быть заменён на makeUnique<T>(Arena&) без изменения API
// потребителей.

namespace tml
{

template <typename T >
using UniquePtr = std::unique_ptr< T >;

} // namespace tml

#endif // ENGINE_CORE_UNIQUE_PTR_HH
