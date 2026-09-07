/*
 1. detect if an inductor is connected: sample ADC node several times with exponential delays (only on positives). If it is 0 then there is nothing.
 2. determine the range: excites the largest resistor branch first, try to time the first tau moment. Then calculate the rough inductance, repeat several times.
 3. out of range detection: upper bound: if the time constant is larger than 10 ms for several measures; lower bound: if the time constant is smaller than 1 us for several measures;
 4. range callibration: if the precise measurement indicates a different range, try the other one. It is okay to jump between ranges, an average value will be calculated from both range results.
*/

void main(void)
{
    while (true)
    {
        if (is_inductor_detected())
        {
            measure_once_with_filter();
            filter_result();
        }
    }
}
