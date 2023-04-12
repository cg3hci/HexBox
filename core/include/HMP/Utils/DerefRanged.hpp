#pragma once 

#include <HMP/Utils/MapRanged.hpp>

namespace HMP::Utils
{

    namespace Internal
    {

        template<typename TIterable>
        using NonConstIteratorDereference = decltype(*std::declval<const NonConstIteratorReference<TIterable>&>());

        template<typename TIterable>
        using ConstIteratorDereference = decltype(*std::declval<const ConstIteratorReference<TIterable>&>());

        template<typename TIterable>
        constexpr NonConstIteratorDereference<TIterable> nonConstIteratorDereference(const NonConstIteratorReference<TIterable>& _reference)
        {
            return *_reference;
        }

        template<typename TIterable>
        constexpr ConstIteratorDereference<TIterable> constIteratorDereference(const ConstIteratorReference<TIterable>& _reference)
        {
            return *_reference;
        }

    }

    template<typename TIterable>
    class ConstDerefRanged: public ConstMapRanged<
        TIterable,
        Internal::ConstIteratorDereference<TIterable>,
        Internal::constIteratorDereference<TIterable>
    >
    {

    protected:

        using ConstMapRanged<TIterable, Internal::ConstIteratorDereference<TIterable>, Internal::constIteratorDereference<TIterable>>::ConstMapRanged;

    };

    template<typename TIterable>
    class NonConstDerefRanged: public NonConstMapRanged<
        TIterable,
        Internal::NonConstIteratorDereference<TIterable>,
        Internal::nonConstIteratorDereference<TIterable>
    >
    {

    protected:

        using NonConstMapRanged<TIterable, Internal::NonConstIteratorDereference<TIterable>, Internal::nonConstIteratorDereference<TIterable>>::NonConstMapRanged;

    };

    template<typename TIterable>
    class ConstAndNonConstDerefRanged: public ConstAndNonConstMapRanged<
        TIterable,
        Internal::ConstIteratorDereference<TIterable>,
        Internal::constIteratorDereference<TIterable>,
        Internal::NonConstIteratorDereference<TIterable>,
        Internal::nonConstIteratorDereference<TIterable>
    >
    {

    protected:

        using ConstAndNonConstMapRanged<
            TIterable,
            Internal::ConstIteratorDereference<TIterable>,
            Internal::constIteratorDereference<TIterable>,
            Internal::NonConstIteratorDereference<TIterable>,
            Internal::nonConstIteratorDereference<TIterable>
        >::ConstAndNonConstMapRanged;

    };

}