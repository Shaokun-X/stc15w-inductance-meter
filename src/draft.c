/*
 1. detect if an inductor is connected: sample ADC node several times with exponential delays (only on positives). If it is 0 then there is nothing.
 2. determine the range: excites the largest resistor branch first, try to time the first tau moment. Then calculate the rough inductance, repeat several times.
 3. out of range detection: upper bound: if the time constant is larger than 10 ms for several measures; lower bound: if the time constant is smaller than 1 us for several measures;
 4. range callibration: if the precise measurement indicates a different range, try the other one. It is okay to jump between ranges, an average value will be calculated from both range results.
*/


// try to use macros, because call stacks can result in significant delays

#define MEASURE_REPEAT 64

static bool has_inductor = false;
static unsigned int measure_buffer[];

enum Range {
    BELOW_1U,
    1U_TO_10U,
    10U_TO_100U,
    100U_TO_1M,
    ABOVE_10,
};

void main(void)
{
    while (true)
    {
        if (is_inductor_detected())
        {
            Range range = probe_range();
            for (int i = 0; i < MEASURE_REPEAT; i++)
            {
                result = measure(range);
                measure_buffer[i] = result;
                range = get_range(result);
            }
        }
    }
}

bool is_inductor_detected()
{
    if (has_inductor)
    {
        return has_inductor;
    }
    // TODO run adc
    return has_inductor;
}

Range probe_range()
{
    // TODO 
    return Range.BELOW_1U;
}

void turn_on_branch()
{
    // TODO set one mosfet on and the reset off
}

void turn_off_all_branches()
{

}


Range get_range(unsigned int result)
{

}

unsigned int measure(Range range)
{
    
}

unsigned int measure_with_comparator()
{

}

unsigned int measure_with_adc()
{

}