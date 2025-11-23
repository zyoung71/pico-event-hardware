#pragma once

template<typename T1, typename T2>
class ResultOptional
{
public:
    const T1& main;
    const T2& supporting;

    inline ResultOptional(const T1& main, const T2& supporting)
        : main(main), supporting(supporting) {}

    operator const T1&()
    {
        return main;
    }
};