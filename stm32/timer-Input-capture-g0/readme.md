# **Input frequency capture using the genreal purpose timers**

In this example we have setup the timer 3 of stm32g0-71rb in the input capture mode. the pin for the capturing the singnal has been turned to the PA6, as the alternative gpio function.
Dma has been enabled for capturing the counter values and send to the processor for processing.

the formula used for calculation of the uev(update event) is:
$$
UEV = (TIMxCLK)/(prescaler+1)(period+1)
$$

Now suppose we choose the prescaler is 1599 and the Rcc Clock is 16MHz , then the value becomes ,
$UEV =  10000/(period+1)$

## Maximum frequency measurement
So the maximum frequency can be measured will be when period is 0. This implies that, period is just increment by one. so when we get the frequency to be 10000Hz or 10Khz. If a frequency has been fed more than this then the one cycle has been completed before the timer start counting, hence we got wrong value of frequency. Although we can fed the input signal to the prescalar to trim down the frequency, but this is the matter for another time.

## Minimum frequency measurement
We can calculate the minimum frequency measurement when we have the preriod values exceed its range, the counter over flow and we have wrong value. Generally period is 16bit counter and its maximum value is 65535. wen we put we got the uev value to be .15hz.

Although we have the the frequency range, it doesnot guranteee the resolution in the frequency range.


