#pragma once 

#include <cpputils/range/Ranged.hpp>
#include <cpputils/range/RangeSizer.hpp>
#include <cpputils/range/of.hpp>
#include <type_traits>

namespace HMP::Utils
{

    namespace Internal
    {

        template<typename TIterable>
        using NonConstIteratorReference = decltype(*std::begin(std::declval<TIterable&>()));

        template<typename TIterable>
        using ConstIteratorReference = decltype(*std::cbegin(std::declval<const TIterable&>()));

        template<
            typename TIterable,
            typename TResult,
            TResult(*TMapper)(const Internal::NonConstIteratorReference<TIterable>&)
        >
        using NonConstMapRange = decltype(cpputils::range::of(std::declval<TIterable&>()).map(TMapper));

        template<
            typename TIterable,
            typename TResult,
            TResult(*TMapper)(const Internal::ConstIteratorReference<const TIterable>&)
        >
        using ConstMapRange = decltype(cpputils::range::ofc(std::declval<const TIterable&>()).map(TMapper));

        template<
            typename TIterable,
            typename TResult,
            TResult(*TMapper)(const Internal::NonConstIteratorReference<TIterable>&)
        >
        using NonConstMapRangeIterator = typename NonConstMapRange<TIterable, TResult, TMapper>::Iterator;

        template<
            typename TIterable,
            typename TResult, TResult(*TMapper)(const Internal::ConstIteratorReference<TIterable>&)
        >
        using ConstMapRangeIterator = typename ConstMapRange<TIterable, TResult, TMapper>::Iterator;

    }

    template<
        typename TIterable,
        typename TResult,
        TResult(*TMapper)(const Internal::ConstIteratorReference<TIterable>&)
    >
    class ConstMapRanged:
        public cpputils::range::ConstRanged<
        Internal::ConstMapRangeIterator<TIterable, TResult, TMapper>,
        cpputils::range::internal::compTimeSize<TIterable>
        >
    {

    private:

        const TIterable& m_iterable;

        virtual Internal::ConstMapRange<TIterable, TResult, TMapper> range() const override final
        {
            return cpputils::range::ofc(m_iterable).map(TMapper);
        }

    protected:

        ConstMapRanged(const TIterable& _iterable): m_iterable{ _iterable } {}

    };

    template<
        typename TIterable,
        typename TResult,
        TResult(*TMapper)(const Internal::NonConstIteratorReference<TIterable>&)
    >
    class NonConstMapRanged:
        public cpputils::range::NonConstRanged<
        Internal::NonConstMapRangeIterator<TIterable, TResult, TMapper>,
        cpputils::range::internal::compTimeSize<TIterable>
        >
    {

    private:

        TIterable& m_iterable;

        virtual Internal::NonConstMapRange<TIterable, TResult, TMapper> range() override final
        {
            return cpputils::range::of(m_iterable).map(TMapper);
        }

    protected:

        NonConstMapRanged(TIterable& _iterable): m_iterable{ _iterable } {}

    };

    template<
        typename TIterable,
        typename TConstResult,
        TConstResult(*TConstMapper)(const Internal::ConstIteratorReference<TIterable>&),
        typename TNonConstResult,
        TNonConstResult(*TNonConstMapper)(const Internal::NonConstIteratorReference<TIterable>&)
    >
    class ConstAndNonConstMapRanged: public cpputils::range::ConstAndNonConstRanged<
        Internal::ConstMapRangeIterator<TIterable, TConstResult, TConstMapper>,
        Internal::NonConstMapRangeIterator<TIterable, TNonConstResult, TNonConstMapper>,
        cpputils::range::internal::compTimeSize<TIterable>
    >
    {

    private:

        TIterable& m_iterable;

        virtual Internal::ConstMapRange<TIterable, TConstResult, TConstMapper> range() const override final
        {
            return cpputils::range::ofc(m_iterable).map(TConstMapper);
        }

        virtual Internal::NonConstMapRange<TIterable, TNonConstResult, TNonConstMapper> range() override final
        {
            return cpputils::range::of(m_iterable).map(TNonConstMapper);
        }

    protected:

        ConstAndNonConstMapRanged(TIterable& _iterable): m_iterable{ _iterable } {}

    };

}