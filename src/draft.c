/*
 1. detect if an inductor is connected: sample ADC node several times with exponential delays (only on positives). If it is 0 then there is nothing.
 2. determine the range: excites the largest resistor branch first, try to time the first tau moment. Then calculate the rough inductance, repeat several times.
 3. out of range detection: upper bound: if the time constant is larger than 10 ms for several measures; lower bound: if the time constant is smaller than 1 us for several measures;
 4. range callibration: if the precise measurement indicates a different range, try the other one. It is okay to jump between ranges, an average value will be calculated from both range results.



| Inductance range | Test resistor | Approx. (\tau=L/R) | Primary method                               | Excitation | Approx. max current* |
| ---------------- | ------------: | -----------------: | -------------------------------------------- | ---------: | -------------------: |
| **10–100 µH**    |       **2 Ω** |            5–50 µs | 2× LM393 + timer; ADC overlap near upper end |  **0.5 V** |               250 mA |
| **100 µH–1 mH**  |      **10 Ω** |          10–100 µs | ADC regression + comparator cross-check      |  **0.5 V** |                50 mA |
| **1–10 mH**      |     **100 Ω** |          10–100 µs | ADC regression                               |    **5 V** |                50 mA |
| **10 mH–1 H**    |      **1 kΩ** |         10 µs–1 ms | ADC regression                               |    **5 V** |                 5 mA |

*/


// try to use macros, because call stacks can result in significant delays

#define MEASURE_REPEAT 64

static bool has_inductor = false;
static unsigned int measure_buffer[];

enum Range {
    RANGE_BELOW_1U, // outbound
    RANGE_1U_TO_10U,
    RANGE_10U_TO_100U,
    RANGE_100U_TO_1M,
    RANGE_ABOVE_1M // outbound
};

enum Mode {
    MODE_AUTO,
    MODE_1U_TO_10U,
    MODE_10U_TO_100U,
    MODE_100U_TO_1M,
}

volatile Mode mode = MODE_AUTO;
volatile Range range = RANGE_ABOVE_10;

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

void button_isr()
{

}