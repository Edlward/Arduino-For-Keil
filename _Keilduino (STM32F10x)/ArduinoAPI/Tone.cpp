/*
 * MIT License
 * Copyright (c) 2019 _VIFEXTech
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Tone.h"

static TIM_TypeDef *ToneTimer_Last = 0, *ToneTimer = ToneTimer_Default;
static bool IsToneEnable = false;
static uint8_t tone_Pin;
static uint32_t tone_StopTimePoint;

/**
  * @brief  tone中断入口，被定时中断调用
  * @param  无
  * @retval 无
  */
static void toneTimer_Handler()
{
    togglePin(tone_Pin);
    if(millis() > tone_StopTimePoint && !IsToneEnable)
    {
        noTone(tone_Pin);
    }
}

/**
  * @brief  改变tone所使用的定时器
  * @param  TIMx: 定时器地址
  * @retval 无
  */
void toneSetTimer(TIM_TypeDef* TIMx)
{
    ToneTimer = TIMx;
}

/**
  * @brief  在Pin上生成指定频率 (50%占空比的方波)
  * @param  pin: 产生音调的 Pin
  * @param  freq: 频率(Hz)
  * @param  Time_ms: 音调的持续时间 (以毫秒为单位)
  * @retval 无
  */
void tone(uint8_t Pin, uint32_t freq, uint32_t Time_ms)
{
    if(!IS_PIN(Pin))
        return;
    
    if(freq == 0 || freq > 500000)
    {
        noTone(Pin);
        return;
    }
    tone(Pin, freq);
    tone_StopTimePoint = millis() + Time_ms;
    IsToneEnable = false;
}

/**
  * @brief  在Pin上生成指定频率 (50%占空比的方波)
  * @param  pin: 产生音调的引脚编号
  * @param  freq: 频率(Hz)
  * @retval 无
  */
void tone(uint8_t Pin, uint32_t freq)
{
    if(!IS_PIN(Pin))
        return;
    
    if(freq == 0 || freq > 500000)
    {
        noTone(Pin);
        return;
    }
    tone_Pin = Pin;
    IsToneEnable = true;

    if(ToneTimer != ToneTimer_Last)
    {
        Timer_Init(ToneTimer, (500000.0 / freq), toneTimer_Handler, 0, 0);
        TIM_Cmd(ToneTimer, ENABLE);
        ToneTimer_Last = ToneTimer;
    }
    else
    {
        TimerSet_InterruptTimeUpdate(ToneTimer, 500000.0 / freq);
        TIM_Cmd(ToneTimer, ENABLE);
    }
}

/**
  * @brief  关闭声音
  * @param  Pin: 产生音调的引脚编号
  * @retval 无
  */
void noTone(uint8_t Pin)
{
    if(!IS_PIN(Pin))
        return;
    
    TIM_Cmd(ToneTimer, DISABLE);
    digitalWrite_LOW(Pin);
    IsToneEnable = false;
}

/**
  * @brief  在Pin上生成指定频率 (50%占空比的方波，阻塞式不占用定时器)
  * @param  pin: 产生音调的引脚编号
  * @param  freq: 频率(Hz)
  * @param  Time_ms: 音调的持续时间 (以毫秒为单位)
  * @retval 无
  */
void toneBlock(uint8_t Pin, uint32_t freq, uint32_t Time_ms)
{
    if(!IS_PIN(Pin) || freq == 0 || Time_ms == 0)
        return;
    
    uint32_t TimePoint = millis() + Time_ms;
    uint32_t dlyus = 500000 / freq; 
    do
    {
        digitalWrite_HIGH(Pin);
        delayMicroseconds(dlyus);
        digitalWrite_LOW(Pin);
        delayMicroseconds(dlyus);
    } while(millis() < TimePoint);
    digitalWrite_LOW(Pin);
}

/**
  * @brief  在Pin上生成指定频率和音量 (可调占空比的方波，阻塞式不占用定时器)
  * @param  pin: 产生音调的引脚编号
  * @param  freq: 频率(Hz)
  * @param  Time_ms: 音调的持续时间 (以毫秒为单位)
  * @param  vol: 音量(0.0~1.0对应占空比0%~100%)
  * @retval 无
  */
void toneBlock_Volumn(uint8_t Pin, uint32_t freq, uint32_t Time_ms, float vol)
{
    if(!IS_PIN(Pin) || freq == 0 || Time_ms == 0 || vol < 0.0)
        return;
    
    uint32_t TimePoint = millis() + Time_ms;
    uint32_t dlyus = 500000 / freq;
    uint32_t dlHigh = dlyus * vol;
    uint32_t dlLow = 2 * dlyus - dlHigh;
    if(freq == 0)return;
    do
    {
        digitalWrite_HIGH(Pin);
        delayMicroseconds(dlHigh);
        digitalWrite_LOW(Pin);
        delayMicroseconds(dlLow);
    } while(millis() < TimePoint);
    digitalWrite_LOW(Pin);
}

/**
  * @brief  在Pin上生成指定频率和音量 (可调占空比的方波，阻塞式不占用定时器)
  * @param  pin: 产生音调的引脚编号
  * @param  freq: 频率(Hz)
  * @param  Time_us: 音调的持续时间 (以微秒为单位)
  * @param  vol: 音量(0.0~1.0对应占空比0%~100%)
  * @retval 无
  */
void toneBlock_Volumn_us(uint8_t Pin, uint32_t freq, uint32_t Time_us, float vol)
{
    if(!IS_PIN(Pin) || freq == 0 || Time_us == 0 || vol < 0.0)
        return;
    
    uint32_t TimePoint = micros() + Time_us;
    uint32_t dlyus = 500000 / freq;
    uint32_t dlHigh = dlyus * vol;
    uint32_t dlLow = 2 * dlyus - dlHigh;
    if(freq == 0)return;
    do
    {
        digitalWrite_HIGH(Pin);
        delayMicroseconds(dlHigh);
        digitalWrite_LOW(Pin);
        delayMicroseconds(dlLow);
    } while(micros() < TimePoint);
    digitalWrite_LOW(Pin);
}
